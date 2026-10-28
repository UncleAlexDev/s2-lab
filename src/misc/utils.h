#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#ifdef __cplusplus

#if 1
// if banned then error message translation will be disabled
#include <QObject>
#endif

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600

class UTILS
{
  public:
    static constexpr double DoubleEpsilon = std::numeric_limits<double>::epsilon();

    static uint64_t get_date_ms();
    static std::string get_date_str(uint64_t ms);

    static void windows_Filetime_to_secs(uint64_t windows_ticks, uint32_t *unix_sec, uint32_t *unix_ns);
    static const char *secs_to_date_str(uint32_t secs);
    static void diff_timespec(struct timespec *tsdiff, const struct timespec *ts1, const struct timespec *ts2, int *minus);
    static std::string ms_to_hh_mm_ss_zzz(long long ms);

    inline static bool DoubleEquals(const double firstValue, const double secondValue) { return (std::abs(firstValue - secondValue) < DoubleEpsilon); }
    inline static bool DoubleGreater(const double firstValue, const double secondValue) { return ((firstValue - secondValue) > DoubleEpsilon); };
    inline static bool DoubleLess(const double firstValue, const double secondValue) { return ((secondValue - firstValue) > DoubleEpsilon); }
    inline static bool FloatEquals(const float firstValue, const float secondValue) { return (std::abs(firstValue - secondValue) < DoubleEpsilon); }

    static bool DoubleGreaterOrEqual(const double firstValue, const double secondValue);
    static bool DoubleLessOrEqual(const double firstValue, const double secondValue);

    static std::string transl(const char *str);
    static std::string hexdump(const unsigned char *data, size_t datalen);

    static QString storage_space_available(const QString &dirname, qint64 required_bytes);
};
#endif // __cplusplus

#endif // UTIL_H
