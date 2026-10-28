/**
  \file   server.cpp
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  Simple server DGRAM UDP
*/

#include "network.h"

#include "adc.h"
#include "cfg.h"
#include "dbg.h"
#include "dpt.h"
#include "mw.h"
#include "ok.h"
#include "channels_points.h"
#include "utils.h"
#include "wsa.h"

//// #include <iostream>
//// #include <string>
//// #include <cstring>
//// #include <sys/types.h>
//// #include <sys/socket.h>
//// #include <netdb.h>
//// #include <unistd.h>
//// #include <arpa/inet.h>

#include <fmt/core.h>
#include <thread>

#include <QApplication>

using namespace std::chrono_literals;

//---------------------------------------------------------
static void print_symbols2(std::string &buf, const float *data, const size_t len)
{
    for (size_t n = 0; n < len; n++) {
        buf.append(fmt::format("{:9.06f} ", data[n]));
    }
}

//---------------------------------------------------------
void NETWORK::dump(const std::vector<float> &points)
{
    auto datalen = (int)points.size();
    std::string buf = fmt::format("{} bytes:\n", datalen);
    buf.append("0    ");
    auto *ptr = points.data();
    for (int i = 0; i < datalen; i += 32) {
        if ((i % 32) == 0 && i > 0) {
            buf.append(fmt::format("\n{:<4} ", i));
        }
        int remain = datalen - i;
        int len = (remain < 32 ? remain : 32);
        print_symbols2(buf, ptr + i, len);
    }
    buf.append("\n");
    DBG(DBG::debug_level, buf);
}

//---------------------------------------------------------
bool NETWORK::running()
{
    return m_run_status == RUN_STATUS::RUN;
}

//---------------------------------------------------------
bool NETWORK::run()
{
    DBGF(4, "\nstatus: {}", status_string(m_run_status));
    if (m_run_status != RUN_STATUS::DOWN) {
        m_strerror = "Server already started";
        return FAIL;
    }

    m_atlas_host = CFG::cfg->atlas_host.toStdString();
    m_atlas_port = CFG::cfg->atlas_port;
    m_ads_host = CFG::cfg->ads_host.toStdString();
    m_ads_data_port = CFG::cfg->ads_data_port;
    m_sureshot_host = CFG::cfg->sureshot_host.toStdString();
    m_sureshot_data_port = CFG::cfg->sureshot_data_port;

    bool ok = FAIL;
    m_sock = {};
    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
        m_sock.host = m_atlas_host;
        m_sock.port = m_atlas_port;
        ok = open_socket_as_udp();
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS) {
        m_sock.host = m_ads_host;
        m_sock.port = m_ads_data_port;
        ok = ads_data_connect();
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
        m_sock.host = m_sureshot_host;
        m_sock.port = m_sureshot_data_port;
        ok = ads_data_connect();
    } else {
        m_strerror = fmt::format("500: INTERNAL ERROR:\n"
                                 "Selected configuration is {}\n"
                                 "Only Atlas and SureShot configuration were applied",
            (int)CFG::cfg->data_format);
        ok = FAIL;
    }
    if (!ok) {
        return FAIL;
    }

    std::thread thr(&NETWORK::server_thread_func, this);
    thr.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return OK;
}

//---------------------------------------------------------
int NETWORK::stop()
{
    if (m_run_status == RUN_STATUS::DOWN) {
        DBGF(4, "status: already {}", NETWORK::status_string(m_run_status));
        return 0;
    }
    m_run_status = RUN_STATUS::GOESDOWN;
    DBGF(5, "status: {}", NETWORK::status_string(m_run_status));

    auto time_beg = std::chrono::system_clock::now();
    while (std::chrono::system_clock::now() <= (time_beg + 300ms)) {
        QApplication::processEvents();
        if (m_run_status == RUN_STATUS::DOWN) {
            break;
        }
        std::this_thread::sleep_for(10ms);
    }

    DBGF(4, "status: {}", NETWORK::status_string(m_run_status));
    socket_close();
    int err = WSA::get_error();
    if (err != 0) {
        m_strerror = WSA::str_error(err);
        DBGF(0, "ERROR: {}", m_strerror);
    }

    if (m_run_status == RUN_STATUS::DOWN) {
        return 0;
    }

    int cnt = 10;
    while (m_run_status != RUN_STATUS::DOWN && cnt > 0) {
        time_beg = std::chrono::system_clock::now();
        while (std::chrono::system_clock::now() <= (time_beg + 200ms)) {
            QApplication::processEvents();
            if (m_run_status == RUN_STATUS::DOWN) {
                DBGF(3, "status: {}", NETWORK::status_string(m_run_status));
                return 0;
            }
            std::this_thread::sleep_for(10ms);
        }
        --cnt;
    }
    DBGF(3, "status [3]: {}", NETWORK::status_string(m_run_status));
    return -1;
}

//---------------------------------------------------------
bool NETWORK::get_bytes()
{
    static int one_print_flag = 1;
    static SOCKADDR_STORAGE src_adr;
    socklen_t src_len = sizeof(src_adr);

    m_strerror.clear();
    m_got_bytes_size = 0;
    memset(m_got_bytes, 0, sizeof(m_got_bytes));
    DBGF(4, "\nwaiting for recvfrom(fd:{})", m_sock.sockfd);

    int nbytes = (int)recvfrom(m_sock.sockfd, m_got_bytes, PKT_DEF_MAX_SIZE, 0, (SOCKADDR*)&src_adr, &src_len);

    DBGF(4, "recvfrom() returns {}", nbytes);
    clock_gettime(CLOCK_REALTIME, &m_timespec);
    if (nbytes == SOCKET_ERROR || nbytes == 0) {
        int err = WSA::get_error();
        if (err != 0) {
            m_strerror = std::string(WSA::str_error(err));
            DBGNL(0);
            DBGNL(0);
            DBGF(0, "==== ISSUE ==== {}, {}\n", err, strerror());
        }
        // #ifdef __linux__
        //         if (err == EINTR) err = 0;
        // #endif
        return FAIL;
    }

    if (m_run_status != RUN_STATUS::RUN) {
        return FAIL;
    }

    // ручная фильтрация адреса отправителя
    if (m_sock.family == AF_INET) {
        const auto *target_sin = (const SOCKADDR_IN *)&m_sock.sockaddr;
        // Если адрес хоста в структуре инициализации не нулевой — значит, включен фильтр IP
        if (target_sin->sin_addr.s_addr != INADDR_ANY) {
            if (src_adr.ss_family == AF_INET) {
                const auto *src_sin = (const SOCKADDR_IN *)&src_adr;
                // Сравниваем IP-адрес прилетевшего пакета с целевым хостом
                if (src_sin->sin_addr.s_addr != target_sin->sin_addr.s_addr) {
                    DBG(4, "Packet ignored: sender IP mismatch");
                    return FAIL; // Пакет не от нашего хоста, сбрасываем итерацию
                }
            } else {
                return FAIL; // Пришел IPv6 в режиме фильтра IPv4
            }
        }
    } else if (m_sock.family == AF_INET6) {
        const auto *target_sin6 = (const SOCKADDR_IN6 *)&m_sock.sockaddr;
        if (std::memcmp(&target_sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any)) != 0) {
            if (src_adr.ss_family == AF_INET6) {
                const auto *src_sin6 = (const SOCKADDR_IN6 *)&src_adr;
                if (std::memcmp(&src_sin6->sin6_addr, &target_sin6->sin6_addr, sizeof(in6addr_any)) != 0) {
                    DBG(4, "Packet ignored: sender IPv6 mismatch");
                    return FAIL;
                }
            } else {
                return FAIL;
            }
        }
    }


    if (one_print_flag > 0) {
        --one_print_flag;
        char sender_ip[INET6_ADDRSTRLEN];
        int sender_port = 0;
        if (src_adr.ss_family == AF_INET) {
            const auto *sin = (const SOCKADDR_IN *)&src_adr;
            inet_ntop(AF_INET, &sin->sin_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin_port);
            DBGF(1, "SENDER {}:{} {}", sender_ip, sender_port, sock_family(sin->sin_family));
        } else if (src_adr.ss_family == AF_INET6) {
            const auto *sin = (const SOCKADDR_IN6 *)&src_adr;
            inet_ntop(AF_INET6, &sin->sin6_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin6_port);
            DBGF(1, "SENDER {}:{} {}", sender_ip, sender_port, sock_family(sin->sin6_family));
        }
    }

    m_got_bytes_size = nbytes;
    m_got_packet_counter++;

    if (m_timespec_prev.tv_sec == 0) {
        m_timespec_prev = m_timespec;
    }

    DBGF(2, "\n#{}, {} bytes", m_got_packet_counter, nbytes);
    if (DBG::debug_level >= 7) {
        auto dump = UTILS::hexdump((unsigned char *)m_got_bytes, m_got_bytes_size);
        DBG(DBG::debug_level, dump);
    }
    return OK;
}

//---------------------------------------------------------
void NETWORK::server_thread_func()
{
    bool first = true;
    m_run_status = RUN_STATUS::RUN;
    DBGF(1, "status: {}", status_string(m_run_status));
    free_packet_atlas();
    m_got_bytes_size = PKT_DEF_MAX_SIZE;
    m_got_packet_counter = 0;
    m_pckt_timestamp = 0;

    while (m_run_status == RUN_STATUS::RUN) {

        DBG(4, "\n--------------------------------------------------");

        if (!get_bytes()) {
            break;
        }

        std::string sign(m_got_bytes, 3);
        if (sign == "APS") {
            DBGF(3, "{}", sign);
            parse_aps_packet();
        } else {
            DBG(3, "ATLAS");
            parse_atlas_packet();

            if (DBG::debug_level >= 4) {
                dbg_print_atlas_packet();
            }

            parse_atlas_data_bytes();

            DBGF(5, "S2Points::points->ch1().timestamp: {}", BBIPoints::points->ch1().timestamp());
            DBGF(5, "S2Points::points->ch2().timestamp: {}", BBIPoints::points->ch2().timestamp());
            DBGF(5, "S2Points::points->ch3().timestamp: {}", BBIPoints::points->ch3().timestamp());
            DBGF(5, "S2Points::points->ch4().timestamp: {}", BBIPoints::points->ch4().timestamp());
        }

        if (first) {
            first = false;
            continue;
        }

        if (MW::mw->currentTabWidget() == ADC::adc->gui_page()) {
           ADC::adc->set_points(*BBIPoints::points);

        } else if (DPT::dpt && MW::mw->currentTabWidget() == DPT::dpt->gui_page()) {
           DPT::dpt->set_points(*BBIPoints::points);
        }

        emit signal_points_updated();
    }

    m_run_status = RUN_STATUS::DOWN;
    free_packet_atlas();
    free_packet_aps();
    DBGF(1, "server finished - status: {}\n", status_string(m_run_status));
}
