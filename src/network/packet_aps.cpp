/**
  \file   packet_aps.c
  \author Aleksey Lekhtin
  \date   Mar 17, 2025
  \brief  The SIU packet content
  */

#include "network.h"

#include "cfg.h"
#include "dbg.h"
#include "channels_points.h"
#include "utils.h"

#include <stdio.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif


//---------------------------------------------------------
void NETWORK::parse_aps_packet()
{
    m_pckt_aps = {};
    double scale = CFG::cfg->atlas_scale_factor;

    const char *p = m_got_bytes;
    m_pckt_aps.sign[0] = *(p + 0);
    m_pckt_aps.sign[1] = *(p + 1);
    m_pckt_aps.sign[2] = *(p + 2);
    m_pckt_aps.msg60 = *(p + 3);
    m_pckt_aps.sensor = *(p + 4);
    p += 5;

    m_pckt_aps.timestamp_sec = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    m_pckt_aps.timestamp_us = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    m_pckt_aps.filenum = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    m_pckt_aps.sample_num = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    auto *points = BBIPoints::points;
    points->toggle_points_buffer();

    int32_t *src = m_pckt_aps.pnts;
    points->ch1().buffer_raw_W().assign(src, src + packet_aps_t::npnts);

    points->ch1().buffer_W().resize(packet_aps_t::npnts);
    std::transform(src, src + packet_aps_t::npnts, points->ch1().buffer_W().begin(),
        [scale](uint32_t val)
        {
            return val * scale;
        });

    points->ch1().set_timestamp(m_pckt_aps.timestamp_us);
    points->ch1().set_timerange(m_pckt_aps.timestamp_us);
    ////int dt = int(points->ch1().timestamp / packet_aps_t::npnts);

    if (DBG::debug_level >= 2) {
        DBG(2, "parsed:\n  0 : ");
        DBG(2, "float:\n  0 : ");
        for (size_t i = 0; i < packet_aps_t::npnts; i++) {
            auto value = points->ch1().buffer_raw_W().data()[i];
            if (i > 0 && (i % 10) == 0) {
                printf("\n%3llu : ", i);
            }
            printf("%10.2f ", value);
        }
        puts("\n");

        DBGF(2, ">>> sign:{:3}, msg60:{}(0x{:X}), sensor:{}, file:{}, samples:{}, created {}.{:3} sec",
            std::string_view(reinterpret_cast<char *>(m_pckt_aps.sign), 3), m_pckt_aps.msg60, m_pckt_aps.msg60,
            m_pckt_aps.sensor, (int)m_pckt_aps.filenum, (int)m_pckt_aps.sample_num, (int)m_pckt_aps.timestamp_sec,
            (int)m_pckt_aps.timestamp_us);

        static struct timespec tsdiff;
        UTILS::diff_timespec(&tsdiff, &m_timespec_prev, &m_timespec, nullptr);
        DBGF(2, "timestamp: {}.{:03} - {}.{:03}  = {}ms", m_timespec_prev.tv_sec, m_timespec_prev.tv_nsec / 1000000,
            m_timespec.tv_sec, m_timespec.tv_nsec / 1000000, tsdiff.tv_sec * 1000 + tsdiff.tv_nsec / 1000000);
        m_timespec_prev = m_timespec;
    }
}

//---------------------------------------------------------
void NETWORK::free_packet_aps()
{
}
