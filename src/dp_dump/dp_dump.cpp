/**
  \file   dpt_dump.cpp
  \brief  To show Dual Pressure Dump file
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#include "dp_dump.h"

#include "dbg.h"
#include "dpt.h"
#include "mw.h"
#include "utils.h"

#include <chrono>
#include <cstring>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iomanip>
#include <iostream>
#include <sstream>

#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
  #error "Компилятор не поддерживает filesystem!"
#endif

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStorageInfo>

using namespace std::chrono;

static std::string make_file_name(const std::string &dirname);

//---------------------------------------------------------
DPDUMP::DPDUMP()
{
    DBG(1, "\n");
    read_status();
}

//---------------------------------------------------------
DPDUMP::~DPDUMP()
{
#ifdef D_B_G
    DBG(4, "++");
#endif
    m_dump.clear();
    m_file.close();
    save_status();

#ifdef D_B_G
    DBG(4, "--");
#endif
}

//---------------------------------------------------------
void DPDUMP::read_status()
{
    DBG(1);
    //// auto status_file = CFG::cfg->app_status();
    //// if (!status_file) {
    ////     return;
    //// }
    //// status_file->beginGroup("DPTDUMP");
    //// chart1_selected_channel = status_file->value("DPT_Chart1ChSelected", 1).toInt();
    //// chart2_selected_channel = status_file->value("DPT_Chart2ChSelected", 2).toInt();
    //// status_file->endGroup();
}

//---------------------------------------------------------
void DPDUMP::save_status() const
{
    DBG(1);
    //// auto status_file = CFG::cfg->app_status();
    //// if (!status_file) {
    ////     return;
    //// }
    //// status_file->beginGroup("DPTDUMP");
    //// status_file->setValue("DPT_Chart1ChSelected", chart1_selected_channel);
    //// status_file->setValue("DPT_Chart2ChSelected", chart2_selected_channel);
    //// status_file->endGroup();
}

//---------------------------------------------------------
bool DPDUMP::create(const std::string &dirname)
{
    m_dump.clear();
    m_err = 0;
    m_strerror.clear();
    if (dirname.empty()) {
        m_err = EMFILE;
        m_strerror = QString("500: %1").arg(QObject::tr("The dump directory is not specified"));
        DBGF(0, "{}", m_strerror.toStdString());
        return false;
    }

    if (m_file.is_open()) {
        m_file.close();
    }

    m_filename = make_file_name(dirname);

    // create empty file
    m_file.open(m_filename, std::ios::trunc | std::ios::binary);
    m_err = errno;
    if (!m_file.is_open()) {
        m_strerror = QString("%1").arg(m_err).arg(std::strerror(m_err));
        DBGF(0, "err: {}, {} - {}", m_err, std::strerror(m_err), m_filename);
        return false;
    }
    DBGF(1, "file: {}", m_filename);

//    dump_t dump{
//        .file_header{
//            .sign = DPDUMP::DP_SIGN,
//            .version = DPDUMP::DP_VERSION1,
//            .date = UTILS::get_date_ms(),
//        },
//    };

    dump_t dump{
        {
            DPDUMP::DP_SIGN,
            DPDUMP::DP_VERSION1,
            UTILS::get_date_ms()
        }
    };

    auto off = (off_t)m_file.tellp();
    DBGF(2, "write file header - offset: {:<16}, {} bytes ", off, sizeof(file_hdr_t));
    if (DPDUMP::write((const char *)&dump.file_header, sizeof(file_hdr_t)) != sizeof(file_hdr_t)) {
        DBGF(0, "{}", m_strerror.toStdString());
        return false;
    }

    m_off_prev = (off_t)m_file.tellp();
    DBGF(2, "offset: {:<16}", (m_off_prev - off));
    DBGF(2, "{}", UTILS::get_date_str(dump.file_header.date));
    return true;
}

//---------------------------------------------------------
void DPDUMP::close()
{
    m_file.close();
    m_dump.clear();
}

//---------------------------------------------------------
bool DPDUMP::is_open()
{
    return m_file.is_open();
}

//---------------------------------------------------------
bool DPDUMP::good()
{
    return m_file.good();
}

//---------------------------------------------------------
static std::string make_file_name(const std::string &dirname)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef __linux__
    localtime_r(&now_time, &tm_struct);
#endif
#ifdef _WIN32
    localtime_s(&tm, &now_time);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y%m%d-%H%M%S");
    std::string timestamp = ss.str();

    std::string fullname = dirname + "/dual-pressure-" + timestamp + ".raw";
    fs::path dir_path(fullname);
    //fullname = dir_path.lexically_normal().string();
    fullname = dir_path.string();
    return fullname;
}

//---------------------------------------------------------
size_t DPDUMP::write(const char *bytes, size_t nbytes)
{
    m_err = 0;
    m_strerror.clear();
    if (!bytes || nbytes == 0 || !m_file.is_open()) {
        m_err = EINVAL;
        m_strerror = QString("Invalid arguments");
        DBGF(0, "{}", m_strerror.toStdString());
        return 0;
    }
    const std::streampos pos1 = m_file.tellp();
    m_file.write(bytes, static_cast<std::streamsize>(nbytes));
    m_err = errno;
    if (!m_file.good()) {
        m_strerror = QString("%1").arg(std::strerror(m_err));
        DBGF(0, "{}", m_strerror.toStdString());
        return 0;
    }

    const std::streampos pos2 = m_file.tellp();
    const auto sz = static_cast<size_t>(pos2 - pos1);
    if (sz != nbytes) {
        m_err = ERANGE;
        m_strerror = QString("%1").arg(QObject::tr("Written data size does not match"));
        DBGF(0, "{}", m_strerror.toStdString());
        return sz;
    }
    DBGF(4, "wrote {}", sz);
    return sz;
}

//---------------------------------------------------------
/// @brief write one data block to file
/// @return 0 - no write due to error
/// @return else number of wrote bytes
///
size_t DPDUMP::write_data_block(const dump_t &dump)
{
    struct data_rec_t
    {
        std::string desc; // name
        const char *ptr;  // данные из памяти или
        uint32_t value;   //    данные это число (если ptr null, то используется value)
        size_t size;      // размер данных (в байтах)
    };

    // подготовка указателей на данные
    auto data_size = dump.data.header.npnts * sizeof(uint32_t);
    struct data_rec_t data_list[] = {
        {"header   ", reinterpret_cast<const char *>(&dump.data.header), 0, sizeof(dump.data.header)},
        {"pressure1", reinterpret_cast<const char *>(dump.data.pressure1.data()), 0, data_size},
        {"pressure2", reinterpret_cast<const char *>(dump.data.pressure2.data()), 0, data_size},
        {"hookload ", reinterpret_cast<const char *>(dump.data.hookload.data()), 0, data_size},
    };

    // запись в файл
    auto off = (off_t)m_file.tellp();
    DBGF(2, "block #{} offset: {} [{}]", (int)dump.data.header.block_number, off, (off - m_off_prev));
    m_off_prev = off;

    size_t total = 0;
    for (const auto &el : data_list) {

        auto off = (off_t)m_file.tellp();
        auto wrote = DPDUMP::write((el.ptr ? el.ptr : reinterpret_cast<const char *>(&el.value)), el.size);
        auto off2 = (off_t)m_file.tellp();

        if (DBG::debug_level > 1) {
            std::string str;
            if (el.ptr) {
                str = fmt::format("0x{:<016X}", reinterpret_cast<uintptr_t>(el.ptr));
            } else {
                str = fmt::format("0x{:<04X} {}", el.value, el.value);
            }
            DBGF(2, "{:<16}: {:<18} ({:>4} bytes) offset: {:>6} 0x{:<8X} [{}]", el.desc, str, el.size, off, off,
                (off2 - off));
        }

        total += wrote;
        if (wrote != el.size) {
            return wrote;
        }
    }
    if (!m_file.flush()) {
        DBG(0, "error while flush file buffers");
    }
    DBGF(1, "total {}\n", total);
    return total;
}

//---------------------------------------------------------
void DPDUMP::dump_data(const channels_points &chs)
{
    static uint64_t timestamp_prev{0};

    // для записи блока данных нужно подготовить все три массива точек:
    // pressure1, pressure2, hookload. еще и encoders

    m_dump.data.header.ctrlw.check_byte = DPDUMP::DP_DUMP_CHECK_BYTE;
    m_dump.data.header.block_number += 1;

    m_dump.data.header.timestamp = chs.ch1().timestamp();

    if (timestamp_prev == 0) {
        timestamp_prev = m_dump.data.header.timestamp - 1000; // ms
    }
    DBGF(3, "{}, timerange: {}", UTILS::get_date_str(m_dump.data.header.timestamp),
        (m_dump.data.header.timestamp - timestamp_prev));
    timestamp_prev = m_dump.data.header.timestamp;

    m_dump.data.header.encoder1 = chs.encoder1();
    m_dump.data.header.encoder2 = chs.encoder2();
    m_dump.data.header.npnts = chs.ch1().points().size();
    m_dump.data.pressure1.assign(chs.ch1().points().begin(), chs.ch1().points().end());
    m_dump.data.pressure2.assign(chs.ch2().points().begin(), chs.ch2().points().end());
    m_dump.data.hookload.assign(chs.ch3().points().begin(), chs.ch3().points().end());

    if (write_data_block(m_dump) == 0) {
        // 0 - no bytes wrote
        auto msg = QObject::tr("FAILED to dump data block to file");
        MW::mw->statusbar_show(msg);
        DBG(0, msg.toStdString());
    }
}
