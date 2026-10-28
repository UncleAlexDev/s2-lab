/**
  \file   socket.cpp
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  Socket procedure
*/

#include "network.h"

#include "dbg.h"
#include "mw.h"
#include "ok.h"
#include "wsa.h"

#include <cstdio>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif
#ifdef _linux
#include <netinet/in.h>
#endif

//---------------------------------------------------------
void NETWORK::socket_close()
{
    if (m_sock.sockfd != INVALID_SOCKET) {
        closesocket(m_sock.sockfd);
        m_sock.sockfd = INVALID_SOCKET;
    }
}

//---------------------------------------------------------
const char *NETWORK::sock_type(int type)
{
    if (type == SOCK_STREAM)
        return "SOCK_STREAM";
    if (type == SOCK_DGRAM)
        return "SOCK_DGRAM";
    if (type == SOCK_RAW)
        return "SOCK_RAW";
    if (type == SOCK_RDM)
        return "SOCK_RDM";
    if (type == SOCK_SEQPACKET)
        return "SOCK_SEQPACKET";

    static char buf[64];
    snprintf(buf, sizeof(buf), "[%d]", type);
    return buf;
}

//---------------------------------------------------------
const char *NETWORK::sock_family(int family)
{
    if (family == AF_INET)
        return "AF_INET";
    if (family == AF_INET6)
        return "AF_INET6";
    if (family == AF_UNSPEC)
        return "AF_UNSPEC";
    if (family == SOCK_SEQPACKET)
        return "SOCK_SEQPACKET";

    static char buf[64];
    snprintf(buf, sizeof(buf), "[%d]", family);
    return buf;}

//---------------------------------------------------------
std::string NETWORK::status_string(NETWORK::RUN_STATUS status)
{
    if (status == RUN_STATUS::DOWN)
        return {"DOWN"};
    if (status == RUN_STATUS::RUN)
        return {"RUN"};
    if (status == RUN_STATUS::GOESDOWN)
        return {"GOESDOWN"};
    return {"UNKN"};
}

//---------------------------------------------------------
///
/// Функция инициализирует UDP сокет для приема данных
/// @return
/// - 0 - success
/// - -1 - error (SOCKET_ERROR)
///
int NETWORK::setup_universal_udp1(const std::string &host, int port)
{
    ADDRINFO hints{};
    ADDRINFO *res;
    auto socket_fd = INVALID_SOCKET;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; // Строго UDP
    DBGF(3, "family: {}, type: {}", sock_family(hints.ai_family), sock_type(hints.ai_socktype));

    bool listen_all = (host == "0.0.0.0" || host == "::" || host.empty());
    if (listen_all) {
        hints.ai_flags = AI_PASSIVE; // Важно для локальной привязки (bind)
    }
    std::string port_str = std::to_string(port);

    const char *target_host = listen_all ? nullptr : host.c_str();
    int status = getaddrinfo(target_host, port_str.c_str(), &hints, &res);
    if (status != 0 || res == nullptr) {
        const int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: getaddrinfo() \n{}", this->strerror());
        return SOCKET_ERROR;
    }

    if (DBG::debug_level > 0) {
        char sender_ip[INET6_ADDRSTRLEN];
        int sender_port = 0;
        if (res->ai_family == AF_INET) {
            const SOCKADDR_IN *sin = (const SOCKADDR_IN *)res->ai_addr;
            inet_ntop(AF_INET, &sin->sin_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin_port);
            DBGF(1, "SENDER {}:{} {}", sender_ip, sender_port, sock_family(sin->sin_family));
        } else if (res->ai_family == AF_INET6) {
            const SOCKADDR_IN6 *sin = (const SOCKADDR_IN6 *)&res->ai_addr;
            inet_ntop(AF_INET6, &sin->sin6_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin6_port);
            DBGF(1, "SENDER {}:{} {}", sender_ip, sender_port, sock_family(sin->sin6_family));
        }
    }

    socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_fd == INVALID_SOCKET) {
        const int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: socket() \n{}", this->strerror());
        freeaddrinfo(res);
        return SOCKET_ERROR;
    }
    DBGF(3, "socket: {}", socket_fd);

    // Включаем поддержку IPv4 для IPv6 сокетов
    if (res->ai_family == AF_INET6) {
        int no = 0;
#ifdef _WIN32
        char *optval = (char *)&no;
#endif
#ifdef __linux__
        void *optval = (void *)&no;
#endif
        socklen_t optlen = sizeof(no);
        if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, optval, optlen) < 0) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: setsockopt(IPPROTO_IPV6, IPV6_V6ONLY) failed\n{}", strerror());
            return SOCKET_ERROR;
        }
    }

    if (listen_all) {
        DBGF(3, "bind({}, 0x{:X}, {})", socket_fd, (uintptr_t)res->ai_addr, res->ai_addrlen);
        if (bind(socket_fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: bind: {}", strerror());
            closesocket(socket_fd);
            freeaddrinfo(res);
            return SOCKET_ERROR;
        }
        DBGF(3, "[ANY IP] listen for port {} ", port);
    } else {
        // hostname — блокируем сокет строго на этот источник
        if (::connect(socket_fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: connect: {}", strerror());
            closesocket(socket_fd);
            freeaddrinfo(res);
            return SOCKET_ERROR;
        }
        DBGF(3, "[FILTER IP] socket locked for {}:{} ", host, port);
    }

    m_sock.sockfd = socket_fd;
    m_sock.type = res->ai_socktype;
    m_sock.family = res->ai_family;
    std::memset(&m_sock.sockaddr, 0, sizeof(m_sock.sockaddr));
    std::memcpy(&m_sock.sockaddr, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return 0;
}

//---------------------------------------------------------
int NETWORK::setup_universal_udp(const std::string &host, int port)
{
    ADDRINFO hints{};
    ADDRINFO *res;
    auto socket_fd = INVALID_SOCKET;
    DBGF(3, "host: {}, port: {}", host, port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; // Строго UDP
    DBGF(3, "family: {}, type: {}", sock_family(hints.ai_family), sock_type(hints.ai_socktype));

    bool listen_all = (host == "0.0.0.0" || host == "::" || host.empty());
    DBGF(3, "listen all: {}", (listen_all ? "yes" : "no"));
    if (listen_all) {
        hints.ai_flags = AI_PASSIVE; // Важно для локальной привязки (bind)
    }
    std::string port_str = std::to_string(port);

    const char *target_host = listen_all ? nullptr : host.c_str();
    int status = getaddrinfo(target_host, port_str.c_str(), &hints, &res);
    if (status != 0 || res == nullptr) {
        const int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: getaddrinfo() \n{}", this->strerror());
        return SOCKET_ERROR;
    }

    if (DBG::debug_level > 0) {
        char sender_ip[INET6_ADDRSTRLEN];
        int sender_port = 0;
        if (res->ai_family == AF_INET) {
            const SOCKADDR_IN *sin = (const SOCKADDR_IN *)res->ai_addr;
            inet_ntop(AF_INET, &sin->sin_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin_port);
            DBGF(1, "SENDER {}:{} {} ({})", sender_ip, sender_port, sock_type(sin->sin_family), sin->sin_family);
        } else if (res->ai_family == AF_INET6) {
            const SOCKADDR_IN6 *sin = (const SOCKADDR_IN6 *)&res->ai_addr;
            inet_ntop(AF_INET6, &sin->sin6_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin6_port);
            DBGF(1, "SENDER {}:{} {} ({})", sender_ip, sender_port, sock_type(sin->sin6_family), sin->sin6_family);
        }
    }

    socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_fd == INVALID_SOCKET) {
        const int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: socket() \n{}", this->strerror());
        freeaddrinfo(res);
        return SOCKET_ERROR;
    }
    DBGF(3, "socket: {}", socket_fd);

    int reuse = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    // Включаем поддержку IPv4 для IPv6 сокетов
    if (res->ai_family == AF_INET6) {
        int no = 0;
#ifdef _WIN32
        char *optval = (char *)&no;
#endif
#ifdef __linux__
        void *optval = (void *)&no;
#endif
        DBGF(2, "setsockopt(IPPROTO_IPV6, IPV6_V6ONLY) to {}", no);
        socklen_t optlen = sizeof(no);
        if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, optval, optlen) < 0) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: setsockopt(IPPROTO_IPV6, IPV6_V6ONLY) failed\n{}", strerror());
            return SOCKET_ERROR;
        }
    }

    // Исправление: ВСЕГДА нужно привязывать сокет к локальному порту ПК.
    // Если фильтруется конкретный IP (listen_all == false), мы сначала биндим сокет
    // на локальный INADDR_ANY (0.0.0.0) с нужным портом, чтобы открыть порт для приема.
    if (!listen_all) {
        ADDRINFO local_hints{};
        ADDRINFO *local_res = nullptr;
        local_hints.ai_family = res->ai_family; // используем семейство, которое выдал getaddrinfo для хоста
        local_hints.ai_socktype = SOCK_DGRAM;
        local_hints.ai_flags = AI_PASSIVE; // для привязки

        // Получаем структуру 0.0.0.0 (или ::) с портом (16000)
        if (getaddrinfo(nullptr, port_str.c_str(), &local_hints, &local_res) == 0) {
            DBGF(3, "local bind({}, 0x{:X}, {})", socket_fd, (uintptr_t)local_res->ai_addr, local_res->ai_addrlen);
            if (::bind(socket_fd, local_res->ai_addr, (int)local_res->ai_addrlen) == SOCKET_ERROR) {
                int err = WSA::get_error();
                m_strerror = std::string(WSA::str_error(err));
                DBGF(0, "ERROR: filter local bind: {}", strerror());
                closesocket(socket_fd);
                freeaddrinfo(local_res);
                freeaddrinfo(res);
                return SOCKET_ERROR;
            }
            freeaddrinfo(local_res);
        }
    } else {
        // Стандартный путь для 0.0.0.0 — биндимся напрямую на то, что вернул getaddrinfo
        DBGF(3, "bind({}, 0x{:X}, {})", socket_fd, (uintptr_t)res->ai_addr, res->ai_addrlen);
        if (::bind(socket_fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: bind: {}", strerror());
            closesocket(socket_fd);
            freeaddrinfo(res);
            return SOCKET_ERROR;
        }
        DBGF(3, "[ANY IP] listen for port {} ", port);
    }

#if 0
    // Теперь, если это конкретный хост, мы поверх открытого порта накладываем системный фильтр connect
    if (!listen_all) {
        DBGF(3, "connect({}, 0x{:X}, {})", socket_fd, (uintptr_t)res->ai_addr, res->ai_addrlen);
        if (::connect(socket_fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            int err = WSA::get_error();
            m_strerror = std::string(WSA::str_error(err));
            DBGF(0, "ERROR: connect: {}", strerror());
            closesocket(socket_fd);
            freeaddrinfo(res);
            return SOCKET_ERROR;
        }
        DBGF(3, "[FILTER IP] socket locked for {}:{} ", host, port);
    }
#endif

    m_sock.sockfd = socket_fd;
    m_sock.type = res->ai_socktype;
    m_sock.family = res->ai_family;
    std::memset(&m_sock.sockaddr, 0, sizeof(m_sock.sockaddr));
    std::memcpy(&m_sock.sockaddr, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return 0;
}

//---------------------------------------------------------
///
/// Создание UDP сокета для приема данных
/// @return
/// 0 - success
/// -1 - error (SOCKET_ERROR)
///
bool NETWORK::open_socket_as_udp()
{
    // 1. create socket
    auto msg = QString("%1 %2:%3").arg(tr("connecting to"), QString::fromStdString(m_sock.host)).arg(m_sock.port);
    DBGF(1, "{}", msg.toStdString());
    //// DBGF(1, "socket: {} ({})", m_sock.sockfd, sock_type(m_sock.type));
    MW::mw->statusbar_show(msg, 30000);

    if (setup_universal_udp(m_sock.host, m_sock.port) == SOCKET_ERROR) {
        return FAIL;
    }

    // 2. options
    int packet_info_enabled = 1;
#ifdef _WIN32
    char *optval = (char *)&packet_info_enabled;
#endif
#ifdef __linux__
    void *optval = (void *)&packet_info_enabled;
#endif
    socklen_t optlen = sizeof(packet_info_enabled);
    int rc = setsockopt(m_sock.sockfd, IPPROTO_IP, IP_PKTINFO, optval, optlen);
    if (rc == SOCKET_ERROR) {
        int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: setsockopt(IPPROTO_IP, IP_PKTINFO) failed\n{}", strerror());
        socket_close();
        return FAIL;
    }
    DBGF(2, "setsockopt(IPPROTO_IP, IP_PKTINFO) to {}", packet_info_enabled);

    rc = getsockopt(m_sock.sockfd, IPPROTO_IP, IP_PKTINFO, optval, &optlen);
    if (rc == SOCKET_ERROR) {
        int err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: getsockopt(IPPROTO_IP, IP_PKTINFO) failed\n{}", strerror());
        socket_close();
        return FAIL;
    }
    if (packet_info_enabled != 1) {
        m_strerror = "socket option to get packet information is not enabled";
        DBGF(0, "{}", strerror());
        socket_close();
        return FAIL;
    }

    if (DBG::debug_level > 0) {
        char sender_ip[INET6_ADDRSTRLEN];
        int sender_port = 0;
        if (m_sock.family == AF_INET) {
            const auto *sin = (const SOCKADDR_IN *)&m_sock.sockaddr;
            inet_ntop(AF_INET, &sin->sin_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin_port);
            DBGF(1, "SENDER {}:{} {} ({})", sender_ip, sender_port, sock_type(sin->sin_family), sin->sin_family);
        } else if (m_sock.family == AF_INET6) {
            const auto *sin = (const SOCKADDR_IN6 *)&m_sock.sockaddr;
            inet_ntop(AF_INET6, &sin->sin6_addr, sender_ip, sizeof(sender_ip));
            sender_port = ntohs(sin->sin6_port);
            DBGF(1, "SENDER {}:{} {} ({})", sender_ip, sender_port, sock_type(sin->sin6_family), sin->sin6_family);
        }
    }

    return OK;
}

//---------------------------------------------------------
bool NETWORK::ads_data_connect()
{
    m_strerror.clear();
    char error_msg[1024 * 4];
    m_sock.sockfd = INVALID_SOCKET;

    auto msg = QString("%1 %2:%3").arg(tr("connecting to"), QString::fromStdString(m_sock.host)).arg(m_sock.port);
    DBGF(1, "{}", msg.toStdString());
    MW::mw->statusbar_show(msg, 30000);
    if (m_sock.host[0] == 0) {
        snprintf(error_msg, sizeof(error_msg) - 1, "The host is not specified");
        DBG(0, error_msg);
        m_strerror = error_msg;
        return FAIL;
    }

    if (m_sock.port == 0) {
        snprintf(error_msg, sizeof(error_msg) - 1, "The data port is not specified");
        DBG(0, error_msg);
        m_strerror = error_msg;
        return FAIL;
    }

    struct addrinfo hints = {};
    struct addrinfo *result, *rp;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", m_sock.port);
    int ret = getaddrinfo(m_sock.host.c_str(), buf, &hints, &result);
    if (ret != 0) {
        int err = WSA::get_error();
        const char *msg = WSA::str_error(err);
        strncpy(error_msg, WSA::str_error(err), sizeof(error_msg) - 1);
        DBGF(0, "ERROR: getaddrinfo(): {}", msg);
        m_strerror = error_msg;
        socket_close();
        return FAIL;
    }

    int err = 0;
    SOCKET fd;
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        DBGF(2, "addrinfo 0x{:0X} {}", (uintptr_t)rp->ai_addr, (rp->ai_canonname ? rp->ai_canonname : ""));
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == INVALID_SOCKET) {
            continue;
        }
        ret = ::connect(fd, rp->ai_addr, (socklen_t)rp->ai_addrlen);
        err = WSA::get_error();
        m_strerror = std::string(WSA::str_error(err));
        DBGF(0, "ERROR: {}", m_strerror);
        // if (ret != SOCKET_ERROR) {
        //     break;
        // }
        closesocket(fd);
    }
    freeaddrinfo(result);

    if (rp == nullptr) {
        DBGF(1, "failed to connect to {}:{}", m_sock.host.c_str(), m_sock.port);
        snprintf(error_msg,
            sizeof(error_msg) - 1,
            "Failed to connect to %s:%d\n\n%s",
            m_sock.host.c_str(),
            m_sock.port,
            (err != 0 ? m_strerror.c_str() : ""));
        m_strerror = error_msg;
        DBGF(1, "{}", m_strerror);
        return FAIL;
    }
    m_sock.sockfd = fd;
    msg = QString("%1 %1:%2").arg(tr("connected to"), QString::fromStdString(m_sock.host)).arg(m_sock.port);
    MW::mw->statusbar_show(msg, 30000);

#ifdef __linux__
    const int enable = 1;
#endif
#ifdef _WIN32
    DWORD enable = 1;
#endif
    if (setsockopt(m_sock.sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(enable)) < 0) {
        DBG(2, "setsockopt(SO_REUSEADDR) failed");
    } else {
        DBGF(2, "setsockopt(SO_REUSEADDR) to {}", (int)enable);
    }

    int timeout_sec = 7;
#ifdef __linux__
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
#endif
#ifdef _WIN32
    DWORD timeout = timeout_sec * 1000; // ms
#endif
    ret = setsockopt(m_sock.sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (ret == SOCKET_ERROR) {
        int err = WSA::get_error();
        DBGF(1, "ERROR: ({}) - setsockopt return {} {}", err, ret, WSA::str_error(err));
        snprintf(error_msg, sizeof(error_msg) - 1, "Can't set receive timeout (SO_RCVTIMEO)\n%s", WSA::str_error(err));
        m_strerror = error_msg;
        socket_close();
        return FAIL;
    }
    DBGF(2, "setsockopt(SO_RCVTIMEO) to {}", timeout_sec);
    DBGF(3, "response timeout set in {} sec", timeout_sec);
    return OK;
}
