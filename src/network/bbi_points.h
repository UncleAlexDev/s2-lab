/**
  \file   bbi2_points.h
  \brief  Данные - отсчеты АЦП
  \author Aleksey Lekhtin
  \date   24.01.2025
*/

#ifndef BBI_POINTS_H
#define BBI_POINTS_H

#include "dbg.h"
#include "network.h"
#include "packet_atlas_t.h"

#include <QList>
#include <QPointF>

#include <cstdint>
#include <vector>

template <typename T>
class BUFFER
{
  public:
    BUFFER(int ch = 0) :
        m_channel(ch)
    {
        m_pointsA.resize(packet_atlas_t::npnts);
        m_pointsB.resize(packet_atlas_t::npnts);
    };

    virtual ~BUFFER() = default;

    inline void clear()
    {
        m_pointsA.resize(0);
        m_pointsB.resize(0);
        m_W = &m_pointsA;
        m_R = &m_pointsB;
    }

    inline void toggle_points_buffer(bool bufferA)
    {
        if (bufferA) {
            m_W = &m_pointsA;
            m_R = &m_pointsB;
        } else {
            m_W = &m_pointsB;
            m_R = &m_pointsA;
        }
    }

    [[nodiscard]] inline const std::vector<T> &R() const
    {
        return *m_R;
    }

    inline std::vector<T> &R()
    {
        return *m_R;
    }

    [[nodiscard]] inline const std::vector<T> &W() const
    {
        return *m_W;
    }

    inline std::vector<T> &W()
    {
        return *m_W;
    }

  private:
    int m_channel{};
    std::vector<T> m_pointsA{};
    std::vector<T> m_pointsB{};
    std::vector<T> *m_R = {&m_pointsA};
    std::vector<T> *m_W = {&m_pointsB};
};

class POINTS_ARRAY
{
  public:
    explicit POINTS_ARRAY(int ch) :
        m_buffer_raw(ch),
        m_buffer(ch)
    {
    }

    virtual ~POINTS_ARRAY() = default;

    inline void clear()
    {
        m_buffer.clear();
        m_buffer_raw.clear();
        m_timerange = {};
    }

    inline void toggle_points_buffer(bool bufferA)
    {
        m_buffer.toggle_points_buffer(bufferA);
        m_buffer_raw.toggle_points_buffer(bufferA);
    }

    [[nodiscard]] inline uint64_t timerange() const
    {
        return m_timerange;
    }

    inline void set_timerange(uint64_t v)
    {
        m_timerange = v;
    }

    [[nodiscard]] inline uint64_t timestamp() const
    {
        return m_timestamp;
    }

    inline void set_timestamp(uint64_t v)
    {
        m_timestamp = v;
    }

    [[nodiscard]] inline const std::vector<float> &buffer_R() const
    {
        return m_buffer.R();
    }

    [[nodiscard]] inline const std::vector<float> &buffer_W() const
    {
        return m_buffer.W();
    }

    [[nodiscard]] inline std::vector<float> &buffer_R()
    {
        return m_buffer.R();
    }

    [[nodiscard]] inline std::vector<float> &buffer_W()
    {
        return m_buffer.W();
    }

    [[nodiscard]] inline const std::vector<float> &buffer_raw_R() const
    {
        return m_buffer_raw.R();
    }

    [[nodiscard]] inline const std::vector<float> &buffer_raw_W() const
    {
        return m_buffer_raw.W();
    }

    [[nodiscard]] inline std::vector<float> &buffer_raw_R()
    {
        return m_buffer_raw.R();
    }
    [[nodiscard]] inline std::vector<float> &buffer_raw_W()
    {
        return m_buffer_raw.W();
    }

  private:
    BUFFER<float> m_buffer_raw;
    BUFFER<float> m_buffer;
    uint64_t m_timestamp{}; // Время, когда получены точки (мсек)
    uint64_t m_timerange{}; // Период, за который получены points (мсек)
};

///
/// \brief Точки принятые по протоколу
///
class BBIPoints
{
  public:
    inline static BBIPoints *points;

  public:
    explicit BBIPoints()
    {
        points = this;
    }

    virtual ~BBIPoints() = default;

    inline POINTS_ARRAY &ch1()
    {
        return m_points1;
    }

    inline POINTS_ARRAY &ch2()
    {
        return m_points2;
    }

    inline POINTS_ARRAY &ch3()
    {
        return m_points3;
    }

    inline POINTS_ARRAY &ch4()
    {
        return m_points4;
    }

    [[nodiscard]] inline int32_t encoder1() const
    {
        return m_encoder1;
    }

    inline void set_encoder1(int32_t v)
    {
        m_encoder1 = v;
    }

    [[nodiscard]] inline int32_t encoder2() const
    {
        return m_encoder2;
    }

    inline void set_encoder2(int32_t v)
    {
        m_encoder2 = v;
    }

    inline void toggle_points_buffer()
    {
        m_toggle = !m_toggle;
        m_points1.toggle_points_buffer(m_toggle);
        m_points2.toggle_points_buffer(m_toggle);
        m_points3.toggle_points_buffer(m_toggle);
        m_points4.toggle_points_buffer(m_toggle);
    }

  private:
    bool m_toggle{false}; // значение определяет, в какой из буферов запиcывать данные на данный момент

    POINTS_ARRAY m_points1{1};
    POINTS_ARRAY m_points2{2};
    POINTS_ARRAY m_points3{3};
    POINTS_ARRAY m_points4{4};
    int32_t m_encoder1{};
    int32_t m_encoder2{};
};

#endif // BBI_POINTS_H
