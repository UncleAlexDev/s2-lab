/**
  \file   channels_points.h
  \brief  Данные - отсчеты АЦП
  \author Aleksey Lekhtin
  \date   24.01.2025
*/

#ifndef CHANNELS_POINTS_H_
#define CHANNELS_POINTS_H_

#include "bbi_points.h"
#include "dbg.h"
#include "network.h"

#include <QList>
#include <QPointF>

#include <cstdint>
#include <vector>

class Points
{
  public:
    Points() = default;

    explicit Points(const std::vector<float> &pnts, int dx = 2) :
        m_vector(pnts),
        m_dx(dx)
    {
        m_xminmax = {0, m_vector.size() * dx - dx};

        auto [minit, maxit] = std::minmax_element(m_vector.begin(), m_vector.end());
        m_yminmax = {*minit, *maxit};

        float sum = std::accumulate(m_vector.begin(), m_vector.end(), 0.0f);
        m_avrg = sum / static_cast<float>(m_vector.size());
    }

    inline void clear()
    {
        m_vector.resize(0);
        m_xminmax = {};
        m_yminmax = {};
        m_avrg = {};
        m_dx = {};
    }

    [[nodiscard]] inline std::vector<float> &vector()
    {
        return m_vector;
    }

    [[nodiscard]] inline const std::vector<float> &vector() const
    {
        return m_vector;
    }

    [[nodiscard]] inline size_t size() const
    {
        return m_vector.size();
    }

    inline void resize(size_t sz)
    {
        m_vector.resize(sz);
    }

    [[nodiscard]] inline bool empty() const
    {
        return m_vector.empty();
    }

    [[nodiscard]] inline float avrg() const
    {
        return m_avrg;
    }

    [[nodiscard]] inline std::pair<float, float> xminmax() const
    {
        return m_xminmax;
    }

    inline void set_xminmax(const std::pair<float, float> &v)
    {
        m_xminmax = v;
    }

    [[nodiscard]] inline std::pair<float, float> yminmax() const
    {
        return m_yminmax;
    }

    inline void set_yminmax(const std::pair<float, float> &v)
    {
        m_yminmax = v;
    }

    [[nodiscard]] auto begin()
    {
        return m_vector.begin();
    }

    [[nodiscard]] auto begin() const
    {
        return m_vector.begin();
    }

    [[nodiscard]] auto end()
    {
        return m_vector.end();
    }

    [[nodiscard]] auto end() const
    {
        return m_vector.end();
    }

    [[nodiscard]] float *data()
    {
        return m_vector.data();
    }

    [[nodiscard]] const float *data() const
    {
        return m_vector.data();
    }

  private:
    std::vector<float> m_vector;
    std::pair<float, float> m_xminmax{};
    std::pair<float, float> m_yminmax{};
    float m_avrg{};
    int m_dx{};
};

///
/// Точки одного канала для отображения на графике
///
class channel_points
{
  public:
    channel_points() = default;

    explicit channel_points(int ch, const POINTS_ARRAY &points) :
        m_ch(ch),
        m_dt(int(points.timerange() / points.buffer_R().size())),
        m_points(points.buffer_R(), m_dt),
        m_points_raw(points.buffer_raw_R(), m_dt),
        m_timestamp(points.timestamp()),
        m_timerange(points.timerange())
    {
        // clang-format off
        DBGF(4, "ch:{} time:{} range:{} dt:{}  x: {:0.2f} - {:0.2f} y: {:11.8f} - {:11.8f} avrg:{:11.8f}",
            m_ch, m_timestamp, m_timerange, m_dt,
            m_points.xminmax().first,
            m_points.xminmax().second,
            m_points.yminmax().first,
            m_points.yminmax().second,
            m_points.avrg());
        DBGF(4, " raw time:{} range:{} dt:{}  x: {:0.2f} - {:0.2f} y: {:11.2f} - {:11.2f} avrg:{:11.2f}",
            m_timestamp, m_timerange, m_dt,
            m_points_raw.xminmax().first,
            m_points_raw.xminmax().second,
            m_points_raw.yminmax().first,
            m_points_raw.yminmax().second,
            m_points_raw.avrg());
        // clang-format on

        if (DBG::debug_level >= 5) {
            NETWORK::dump(m_points.vector());
        }
    }

    inline Points &points()
    {
        return m_points;
    }

    [[nodiscard]] inline const Points &points() const
    {
        return m_points;
    }

    [[nodiscard]] inline Points &points_raw()
    {
        return m_points_raw;
    }

    [[nodiscard]] inline const Points &points_raw() const
    {
        return m_points_raw;
    }

    inline void clear()
    {
        m_points.clear();
        m_points_raw.clear();
    }

    [[nodiscard]] inline size_t size() const
    {
        return m_points.size();
    }

    [[nodiscard]] inline int dt() const
    {
        return m_dt;
    }

    inline void set_dt(int dt)
    {
        m_dt = dt;
    }

    [[nodiscard]] inline uint64_t timerange() const
    {
        return m_timerange;
    }

    inline void set_timerange(uint64_t tr)
    {
        m_timerange = tr;
    }

    [[nodiscard]] inline uint64_t timestamp() const
    {
        return m_timestamp;
    }

    inline void set_timestamp(uint64_t ts)
    {
        m_timestamp = ts;
    }

    [[nodiscard]] inline int off() const
    {
        return m_off;
    }

    inline void set_off(int off)
    {
        m_off = off;
    }

  private:
    int m_ch{0};
    int m_dt{0}; // ms
    int m_off{0};

    Points m_points{};
    Points m_points_raw{};

    uint64_t m_timestamp{}; // Время, когда получены точки (мсек)
    uint64_t m_timerange{}; // Период, за который получены points (мсек)
};

///
/// Точки по всем каналам
///
class channels_points
{
  public:
    channels_points() = default;

    inline void clear()
    {
        m_ch1.clear();
        m_ch2.clear();
        m_ch3.clear();
        m_ch4.clear();
        m_encoder1 = 0.0f;
        m_encoder2 = 0.0f;
    }

    inline const channel_points &ch1() const
    {
        return m_ch1;
    }

    inline const channel_points &ch2() const
    {
        return m_ch2;
    }

    inline const channel_points &ch3() const
    {
        return m_ch3;
    }

    inline const channel_points &ch4() const
    {
        return m_ch4;
    }

    inline const int32_t &encoder1() const
    {
        return m_encoder1;
    }

    inline const int32_t &encoder2() const
    {
        return m_encoder2;
    }

    // non const
    inline channel_points &ch1()
    {
        return m_ch1;
    }

    inline channel_points &ch2()
    {
        return m_ch2;
    }

    inline channel_points &ch3()
    {
        return m_ch3;
    }

    inline channel_points &ch4()
    {
        return m_ch4;
    }

    inline void set_encoder1(const int32_t v)
    {
        m_encoder1 = v;
    }

    inline void set_encoder2(const int32_t v)
    {
        m_encoder2 = v;
    }

    inline void set_points(int ch, const POINTS_ARRAY &points)
    {
        if (ch == 1) {
            m_ch1 = channel_points(ch, points);
        } else if (ch == 2) {
            m_ch2 = channel_points(ch, points);
        } else if (ch == 3) {
            m_ch3 = channel_points(ch, points);
        } else if (ch == 4) {
            m_ch4 = channel_points(ch, points);
        }
    }

  private:
    channel_points m_ch1{};
    channel_points m_ch2{};
    channel_points m_ch3{};
    channel_points m_ch4{};

    int32_t m_encoder1{};
    int32_t m_encoder2{};
};

#endif // CHANNELS_POINTS_H_
