/*!
  \file   utils.cpp
  \author Aleksey Lekhtin
  \date   27.11.2024
  \brief  Miscellaneous
*/

#include "utils.h"

#include "dbg.h"
//// #include "mw.h"
//// #include "wmsg.h"

#include <ctime>
#include <fmt/chrono.h> // Важно: подключаем заголовок для chrono
#include <fmt/core.h>

#include <QFileInfo>
#include <QStorageInfo>

//---------------------------------------------------------
std::string UTILS::transl(const char *str)
{
#ifdef QOBJECT_H
    return QObject::tr(str).toStdString();
#else
    return std::string(str);
#endif
}

//---------------------------------------------------------
void UTILS::windows_Filetime_to_secs(uint64_t windows_ticks, uint32_t *unix_sec, uint32_t *unix_ns)
{
    if (!unix_sec)
        return;
    *unix_sec = (time_t)(windows_ticks / WINDOWS_TICK);
    *unix_sec -= SEC_TO_UNIX_EPOCH;
    if (unix_ns) {
        uint64_t num = *unix_sec + SEC_TO_UNIX_EPOCH;
        num *= WINDOWS_TICK;
        *unix_ns = (long)(windows_ticks - num);
        *unix_ns *= 100;
    }
}

//---------------------------------------------------------
/**
 * @brief returns date string
 * @param secs - total seconds from start Unix time
 * @return always date string (no null anyway)
 */
const char *UTILS::secs_to_date_str(uint32_t secs)
{
    time_t tt = secs;
    struct tm *tm = gmtime(&tt); // localtime(&tt)
    if (!tm) {
        return "--.--.----";
    }
    if (tm->tm_year < (2024 - 1900) || tm->tm_year > (3000 - 1900)) {
        return "--.--.----";
    }
    static char buf[32];
    buf[0] = 0;
    strftime(buf, sizeof(buf), "%d.%m.%Y-%H:%M:%S", tm);
    return buf;
}

//---------------------------------------------------------
std::string UTILS::ms_to_hh_mm_ss_zzz(long long ms)
{
    using namespace std::chrono;
    std::chrono::milliseconds ms_duration(ms);

    // Преобразуем в hh:mm:ss
    auto hh = std::chrono::duration_cast<hours>(ms_duration);
    ms_duration -= hh;
    auto mm = std::chrono::duration_cast<minutes>(ms_duration);
    ms_duration -= mm;
    auto ss = std::chrono::duration_cast<seconds>(ms_duration);
    ms_duration -= ss;

    return fmt::format("{:02}:{:02}:{:02}.{:03}", hh.count(), mm.count(), ss.count(), ms_duration.count());
}

//---------------------------------------------------------
/**
 * The difference beetwen times specification
 *
 * @param[out] tsdiff - the difference time
 * @param[in] ts1 - the start time
 * @param[in] ts2 - the end time
 * @param[in] minus - the sign of difference negative
 */
void UTILS::diff_timespec(struct timespec *tsdiff, const struct timespec *ts1, const struct timespec *ts2, int *minus)
{
    if (minus) {
        *minus = 0;
    }
    tsdiff->tv_sec = ts2->tv_sec - ts1->tv_sec;
    tsdiff->tv_nsec = ts2->tv_nsec - ts1->tv_nsec;
    if (tsdiff->tv_sec == 0) {
        if (tsdiff->tv_nsec < 0) {
            tsdiff->tv_nsec *= -1;
            if (minus) {
                *minus = 1;
            }
        }
        return;
    }

    if (tsdiff->tv_sec > 0) {
        if (tsdiff->tv_nsec < 0) {
            tsdiff->tv_sec--;
            tsdiff->tv_nsec += 1000000000; // nsec/sec
        }
        return;
    }

    if (minus) {
        *minus = 1;
    }
    if (tsdiff->tv_nsec == 0) {
        tsdiff->tv_sec *= -1;
        return;
    }

    if (tsdiff->tv_nsec > 0) {
        tsdiff->tv_sec++;
        tsdiff->tv_nsec = ts2->tv_nsec + ts1->tv_nsec;
        tsdiff->tv_sec *= -1;
    } else if (tsdiff->tv_nsec < 0) {
        tsdiff->tv_sec *= -1;
        tsdiff->tv_nsec *= -1;
    }
}

//---------------------------------------------------------
struct tm *local_time(const time_t *secs, struct tm *tmval)
{
#ifdef __linux__
    if (!localtime_r(secs, tmval)) {
        return NULL;
    }
#endif
#ifdef _WIN32
    if (localtime_s(tmval, secs) != 0) {
        return nullptr;
    }
#endif
    return tmval;
}

//---------------------------------------------------------
struct tm *gmt_time(const time_t *secs, struct tm *tm)
{
#ifdef __linux__
    if (!gmtime_r(secs, tm)) {
        return NULL;
    }
#endif
#ifdef _WIN32
    if (gmtime_s(tm, secs) != 0) {
        return nullptr;
    }
#endif
    return tm;
}

///// //---------------------------------------------------------
///// bool UTILS::DoubleEquals(const double firstValue, const double secondValue)
///// {
/////     if (std::abs(firstValue - secondValue) < epsilon) {
/////         return true;
/////     }
/////     return false;
///// }

///// //---------------------------------------------------------
///// bool UTILS::DoubleGreater(const double firstValue, const double secondValue)
///// {
/////     if (firstValue - secondValue > DoubleEpsilon) {
/////         return true;
/////     }
/////     return false;
///// }

//---------------------------------------------------------
bool UTILS::DoubleGreaterOrEqual(const double firstValue, const double secondValue)
{
    if (firstValue - secondValue >= DoubleEpsilon) {
        return true;
    }
    return false;
}

///// //---------------------------------------------------------
///// bool UTILS::DoubleLess(const double firstValue, const double secondValue)
///// {
/////     if (secondValue - firstValue > DoubleEpsilon) {
/////         return true;
/////     }
/////     return false;
///// }

//---------------------------------------------------------
bool UTILS::DoubleLessOrEqual(const double firstValue, const double secondValue)
{
    if (secondValue - firstValue >= DoubleEpsilon) {
        return true;
    }
    return false;
}

//// //---------------------------------------------------------
//// bool UTILS::FloatEquals(const float firstValue, const float secondValue)
//// {
////     return FloatEquals(firstValue, secondValue, 0.0001f);
//// }
////
//// //---------------------------------------------------------
//// bool UTILS::FloatEquals(const float firstValue, const float secondValue, const float epsilon)
//// {
////     if (std::abs(firstValue - secondValue) < epsilon) {
////         return true;
////     }
////     return false;
//// }

//---------------------------------------------------------
static void print_symbols2(std::string &msg, const unsigned char *data, const size_t len)
{
    for (size_t n = 0; n < len; n++) {
        if (isprint(data[n])) {
            msg.append(fmt::format("{:c}", data[n]));
        } else if (data[n] != 0) {
            msg.append("_");
        } else {
            msg.append(".");
        }
    }
}

//---------------------------------------------------------
/// Just print obj in hex form
/// @param[in] data - byte sequence
/// @param[in] datalen - length of sequence
///
std::string UTILS::hexdump(const unsigned char *data, size_t datalen)
{
    if (!data) {
        DBGF(0, "obj: {:08x}. skip", (uintptr_t)data);
        return {};
    }

    if (datalen == 0) {
        DBG(0, "no data. skip");
        return {};
    }

    std::string msg = fmt::format("{}: {} bytes:\n", __func__, datalen);
    msg.append("0   : ");
    for (size_t i = 0; i < datalen; i++) {
        if (i > 0) {
            if ((i % 32) == 0) {
                msg.append(" ");
                print_symbols2(msg, data + i - 32, 32);
                msg.append(fmt::format("\n{:<4}: ", i));
            } else if ((i % 16) == 0) {
                msg.append(" ");
            }
        }
        msg.append(fmt::format("{:02X} ", data[i]));
    }
    const size_t n = datalen % 32;
    const size_t remains = 32 - n;
    const size_t off = datalen - n;
    for (size_t i = 0; i < remains; ++i) {
        msg.append("   ");
    }
    msg.append("  ");
    print_symbols2(msg, data + off, n);
    msg.append("\n");
    return msg;
}

//---------------------------------------------------------
QString UTILS::storage_space_available(const QString &dirname, qint64 required_bytes)
{
    QString dn = dirname;
    if (dn.isEmpty()) {
        dn = QDir::toNativeSeparators(QDir::currentPath());
    }
    QFileInfo finfo(dn);
    QStorageInfo storage(finfo.absoluteDir());
    storage.refresh();
    auto storage_name = QDir::toNativeSeparators(storage.displayName());

    if (!storage.isValid() || !storage.isReady()) {
        auto msg = QString("%1\n%2").arg(QObject::tr("Can't read storage info about"), storage_name);
        //// WMSG::WMsg({}, msg, QMessageBox::Warning);
        return msg;
    }

    // qint64 bytesTotal = storage.bytesTotal();
    qint64 available = storage.bytesAvailable();
    double gbAvailable = static_cast<double>(available) / (1024 * 1024 * 1024);
    auto total = QString("%1 %2").arg(gbAvailable, 0, 'f', 2).arg(QObject::tr("Gb"));

    if (gbAvailable < 0) {
        double availableMb = static_cast<double>(available) / (1024 * 1024);
        auto total = QString("%1 %2").arg(availableMb, 0, 'f', 2).arg(QObject::tr("Mb"));
    }

    if (available < required_bytes) {
        auto msg = QString("%1\n%2\n\n%3\n\n%4 %5\n")
                       .arg(QObject::tr("Not enough storage space in"))
                       .arg(QDir::toNativeSeparators(finfo.filePath()))
                       .arg(QObject::tr("At least 1 GB of storage space is required for two days of work."))
                       .arg(QObject::tr("Total available"))
                       .arg(total);
        //// WMSG::WMsg(MW::mw, msg, QMessageBox::Warning);
        return msg;
    }
    return {};
}

//---------------------------------------------------------
uint64_t UTILS::get_date_ms()
{
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::floor<std::chrono::milliseconds>(now);
    std::chrono::duration duration = milliseconds.time_since_epoch();
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    if (DBG::debug_level >= 4) {
        auto str = UTILS::get_date_str(ms);
        DBGF(4, "{}", str);
    }

    return ms;
}

//---------------------------------------------------------
std::string UTILS::get_date_str(uint64_t ms)
{
    std::chrono::milliseconds duration(ms);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto ms_part = (duration - seconds).count();

    std::time_t timet = seconds.count();
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &timet);
#else
    localtime_r(&timet, &time_info);
#endif
    auto str = fmt::format("{:%Y-%m-%d %H:%M:%S}.{:03}", tm, ms_part);
    return str;
}

