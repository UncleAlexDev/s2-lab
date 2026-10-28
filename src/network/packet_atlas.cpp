/**
  \file   packet_atlas.c
  \author Aleksey Lekhtin
  \date   April 1, 2024
  \brief  The packet content
  */

#include "cfg.h"
#include "dbg.h"
#include "dpt.h"
#include "mw.h"
#include "network.h"
#include "utils.h"

#include <algorithm>
#include <cstdio>
#include <fmt/chrono.h>
#include <malloc.h>
#include <vector>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

//---------------------------------------------------------
void NETWORK::parse_atlas_packet()
{
    if (m_pckt_timestamp == 0) {
        m_pckt_timestamp = UTILS::get_date_ms();
        DBGF(3, "{}", fmt::format("{}", UTILS::get_date_str(m_pckt_timestamp)));
    }

    m_pckt_atlas.rtp.ext_data.clear();
    m_pckt_atlas.m_raw_points1 = nullptr;
    m_pckt_atlas.m_raw_points2 = nullptr;
    m_pckt_atlas.m_raw_points3 = nullptr;
    m_pckt_atlas.m_raw_points4 = nullptr;

    const char *p = m_got_bytes;
    m_pckt_atlas.rtp.fixed_part.bits = ntohs(*(uint16_t *)p);
    p += sizeof(uint16_t);

    m_pckt_atlas.rtp.fixed_part.seqnum = ntohs(*(uint16_t *)p);
    p += sizeof(uint16_t);

    m_pckt_atlas.rtp.fixed_part.timerange = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    m_pckt_atlas.rtp.fixed_part.SSRC = ntohl(*(uint32_t *)p);
    p += sizeof(uint32_t);

    int CSRC_count = (m_pckt_atlas.rtp.fixed_part.bits >> 8) & 0x0F;
    for (int i = 0; i < CSRC_count && i < 15; i++) {
        m_pckt_atlas.rtp.CSRC[i] = ntohl(*(uint32_t *)p);
        p += sizeof(uint32_t);
    }

    const int ext = (m_pckt_atlas.rtp.fixed_part.bits >> 12) & 0x01;
    if (ext) {
        m_pckt_atlas.rtp.ext_header.implementation = ntohs(*(uint16_t *)p);
        p += sizeof(uint16_t);
        m_pckt_atlas.rtp.ext_header.total_recs = ntohs(*(uint16_t *)p);
        p += sizeof(uint16_t);
        m_pckt_atlas.rtp.ext_data.resize(m_pckt_atlas.rtp.ext_header.total_recs);
        for (int i = 0; i < m_pckt_atlas.rtp.ext_header.total_recs; i++) {
            m_pckt_atlas.rtp.ext_data[i] = ntohl(*(uint32_t *)p);
            p += sizeof(uint32_t);
        }
    }

    m_pckt_atlas.rtp_size = p - m_got_bytes;
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    m_pckt_atlas.data_header.encoder1 = *(int32_t *)p;
    p += sizeof(uint32_t);
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    m_pckt_atlas.data_header.data_size = *(uint32_t *)p;
    p += sizeof(uint32_t);
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    m_pckt_atlas.data_header.encoder2 = *(uint32_t *)p;
    p += sizeof(m_pckt_atlas.data_header.encoder2);
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    m_pckt_atlas.data_header.misc.dword = *(uint32_t *)p;
    p += sizeof(m_pckt_atlas.data_header.misc.dword);
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    m_pckt_atlas.data_header.rezerv_3 = *(uint32_t *)p;
    p += sizeof(m_pckt_atlas.data_header.rezerv_3);
    DBGF(5, "pos: {}", (uintptr_t)(p - m_got_bytes));

    size_t pos = p - m_got_bytes;
    const uint32_t data_block_size = m_pckt_atlas.data_header.data_size;

    // channel 1
    if (pos + data_block_size <= m_got_bytes_size) {
        m_pckt_atlas.m_raw_points1 = (int32_t *)p;
        p += data_block_size;
        pos += data_block_size;
        DBGF(5, "pos: {} ptr: {}", (uintptr_t)(p - m_got_bytes), (void *)m_pckt_atlas.m_raw_points1);
    }

    // channel 2
    if (pos + data_block_size <= m_got_bytes_size) {
        m_pckt_atlas.m_raw_points2 = (int32_t *)p;
        p += data_block_size;
        pos += data_block_size;
        DBGF(5, "pos: {} ptr: {}", (uintptr_t)(p - m_got_bytes), (void *)m_pckt_atlas.m_raw_points2);
    }

    // channel 3
    if (pos + data_block_size <= m_got_bytes_size) {
        m_pckt_atlas.m_raw_points3 = (int32_t *)p;
        p += data_block_size;
        pos += data_block_size;
        DBGF(5, "pos: {} ptr: {}", (uintptr_t)(p - m_got_bytes), (void *)m_pckt_atlas.m_raw_points3);
    }

    // channel 4
    if (pos + data_block_size <= m_got_bytes_size) {
        m_pckt_atlas.m_raw_points4 = (int32_t *)p;
        p += data_block_size;
        pos += data_block_size;
        DBGF(5, "pos: {} ptr: {}", (uintptr_t)(p - m_got_bytes), (void *)m_pckt_atlas.m_raw_points4);
    }
}

//---------------------------------------------------------
void NETWORK::free_packet_atlas()
{
    m_pckt_atlas = {};
}

//---------------------------------------------------------
void NETWORK::parse_atlas_data_bytes()
{
    m_pckt_timestamp += m_pckt_atlas.rtp.fixed_part.timerange;
    // clang-format off
    DBGF(2, "{}", fmt::format("timerange: {} ms    {}, DPT::delay: {}",
            (uint64_t)m_pckt_atlas.rtp.fixed_part.timerange,
            UTILS::get_date_str(m_pckt_timestamp), (int)DPT::delay));
    // clang-format on

    // переключение буфера для приема данных (для Dual Pressure Transducer)
    BBIPoints::points->toggle_points_buffer();

    double scale = CFG::cfg->atlas_scale_factor;
    const size_t sz = packet_atlas_t::npnts;

    int32_t *src{};
    POINTS_ARRAY *dst{};

    for (const int chnum : {1, 2, 3, 4}) {
        if (chnum == 1) {
            src = m_pckt_atlas.m_raw_points1;
            dst = &BBIPoints::points->ch1();
        } else if (chnum == 2) {
            src = m_pckt_atlas.m_raw_points2;
            dst = &BBIPoints::points->ch2();
        } else if (chnum == 3) {
            src = m_pckt_atlas.m_raw_points3;
            dst = &BBIPoints::points->ch3();
        } else if (chnum == 4) {
            src = m_pckt_atlas.m_raw_points4;
            dst = &BBIPoints::points->ch4();
        }
        if (!src || !dst) {
            continue;
        }

        dst->set_timestamp(m_pckt_timestamp);
        dst->set_timerange(m_pckt_atlas.rtp.fixed_part.timerange);
        int dt = int(dst->timerange() / sz);
        //dst->buffer_raw_W().assign(src, src + sz);

        if (DPT::delay > 0 && chnum == 1) {
            static int dx0 = 0;
            auto dx = std::abs((DPT::delay / dt));
            DBGF(2, "delay: {} ms, dt: {}, dx: {//// } [{} <> {}]", DPT::delay, dt, dx, DPT::delay, dx0);

            //
            std::transform(src + sz - dx,
                src + sz,
                dst->buffer_R().begin(),
                [scale](int32_t val)
                {
                    return static_cast<float>(val * scale);
                });

            std::transform(src,
                src + sz - dx,
                dst->buffer_W().begin() + dx,
                [scale](int32_t val)
                {
                    return static_cast<float>(val * scale);
                });

            //
            std::transform(src + sz - dx,
                src + sz,
                dst->buffer_raw_R().begin(),
                [](int32_t val)
                {
                    return static_cast<float>(val);
                });

            std::transform(src,
                src + sz,
                dst->buffer_raw_W().begin(),
                [](int32_t val)
                {
                    return static_cast<float>(val);
                });


            dx0 = DPT::delay;

        } else {
            std::transform(src,
                src + sz,
                dst->buffer_W().begin(),
                [scale](int32_t val)
                {
                    return static_cast<float>(val * scale);
                });

            dst->buffer_raw_W().assign(src, src + sz);
            // std::transform(src,
            //     src + sz,
            //     dst->buffer_raw_W().begin(),
            //     [](int32_t val)
            //     {
            //         return static_cast<float>(val);
            //     });
        }

        DBGF(3, "channel: {} - data got", chnum);
    }

    BBIPoints::points->set_encoder1(m_pckt_atlas.data_header.encoder1);
    BBIPoints::points->set_encoder2(m_pckt_atlas.data_header.encoder2);
}

//---------------------------------------------------------
void NETWORK::dbg_print_atlas_packet()
{
    DBGF(4,
        "packet #{}, size:{}, data block len:{:<6}",
        m_got_packet_counter,
        m_got_bytes_size,
        (int)m_pckt_atlas.data_header.data_size);

    printf("RTP header (%u bytes): \n    ", m_pckt_atlas.rtp_size);
    for (uint32_t i = 0; i < m_pckt_atlas.rtp_size; i++) {
        printf("%02X ", (unsigned char)m_got_bytes[i]);
    }
    puts("");

    printf("    ver: %u, pad: %u, ext: %u, CSRC: %u, marker: %u, payload type: %u\n"
           "    seq: %u, timerange: %u (0x%x), SSRC: %u (0x%08x)\n",
        (m_pckt_atlas.rtp.fixed_part.bits >> 14) & 0x03,
        (m_pckt_atlas.rtp.fixed_part.bits >> 13) & 0x01,
        (m_pckt_atlas.rtp.fixed_part.bits >> 12) & 0x01,
        (m_pckt_atlas.rtp.fixed_part.bits >> 8) & 0x0F,
        (m_pckt_atlas.rtp.fixed_part.bits >> 7) & 0x01,
        (m_pckt_atlas.rtp.fixed_part.bits) & 0x7F,
        m_pckt_atlas.rtp.fixed_part.seqnum,
        m_pckt_atlas.rtp.fixed_part.timerange,
        m_pckt_atlas.rtp.fixed_part.timerange,
        m_pckt_atlas.rtp.fixed_part.SSRC,
        m_pckt_atlas.rtp.fixed_part.SSRC);

    int CSRC_count = (m_pckt_atlas.rtp.fixed_part.bits >> 8) & 0x0F;
    if (CSRC_count) {
        printf("    CSRC: [ ");
        for (int i = 0; i < CSRC_count && i < 15; i++) {
            printf("%u(0x%x) ", m_pckt_atlas.rtp.CSRC[i], m_pckt_atlas.rtp.CSRC[i]);
        }
        puts("]");
    }

    int ext = (m_pckt_atlas.rtp.fixed_part.bits >> 12) & 0x01;
    if (ext) {
        printf("    EXT:  impl: %d, nums: %d [ ",
            m_pckt_atlas.rtp.ext_header.implementation,
            m_pckt_atlas.rtp.ext_header.total_recs);
        if (!m_pckt_atlas.rtp.ext_data.empty()) {
            for (int i = 0; i < m_pckt_atlas.rtp.ext_header.total_recs; i++) {
                printf("%u(0x%08x) ", m_pckt_atlas.rtp.ext_data[i], m_pckt_atlas.rtp.ext_data[i]);
            }
        }
        puts("]");
    }

    uint32_t header_len = sizeof(m_pckt_atlas.data_header.encoder1) + sizeof(m_pckt_atlas.data_header.data_size) +
                          sizeof(m_pckt_atlas.data_header.encoder2) + sizeof(m_pckt_atlas.data_header.misc) +
                          sizeof(m_pckt_atlas.data_header.rezerv_3);
    uint32_t payload_part_len = m_got_bytes_size - m_pckt_atlas.rtp_size - header_len;
    printf("Data (%u bytes):\n"
           "    encoder1:   %d (0x%08x)\n"
           "    data size:  %u\n"
           "    encoder2:   %d (0x%08x)\n"
           "    misc:       0x%08X (ver[2bits]: %d)\n"
           "    rezerv3:    0x%08X\n"
           "Total: %u\n",
        payload_part_len,
        (int32_t)m_pckt_atlas.data_header.encoder1,
        (int32_t)m_pckt_atlas.data_header.encoder1,
        m_pckt_atlas.data_header.data_size,
        (int32_t)m_pckt_atlas.data_header.encoder2,
        (int32_t)m_pckt_atlas.data_header.encoder2,
        m_pckt_atlas.data_header.misc.dword,
        (m_pckt_atlas.data_header.misc.bits.proto_ver & 0x03),
        m_pckt_atlas.data_header.rezerv_3,
        m_pckt_atlas.rtp_size + header_len + payload_part_len);

    size_t ln;
    char buf[1024] = {0};
    char *mem = (m_got_bytes + m_pckt_atlas.rtp_size);
    mem += sizeof(uint32_t);     // skip decoder
    mem += sizeof(uint32_t);     // datasize (for each pressure and hookload)
    mem += sizeof(uint32_t) * 3; // reserv{1..3}

    if (DBG::debug_level >= 6) {
        size_t i;
        auto *ptr = (uint32_t *)mem;
        std::vector<int> channels{1};
        // int ch = MW::mw->adc_page()->chart1()->channel();
        // if (ch > 0 && ch < 5) {
        //     channels.emplace_back(1);
        // }
        // ch = MW::mw->adc_page()->chart2()->channel();
        // if (ch > 0 && ch < 5) {
        //     channels.emplace_back(2);
        // }
        for (int channel : channels) {
            printf("%s() [%d] channel-%d (raw bytes):\n", __func__, DBG::debug_level, channel);
            printf("%-4d : ", 0);
            for (i = 0; i < packet_atlas_t::npnts; i += 16) {
                size_t remains = packet_atlas_t::npnts - i;
                if (i > 0 && (i % 16) == 0) {
                    printf("\n%-4llu : ", i);
                }
                buf[0] = 0;
                char *p = buf;
                for (size_t n = 0; n < std::min((size_t)16, remains); ++n) {
                    ln = snprintf(p, 10, "%08X ", *ptr);
                    p += ln;
                    ptr++;
                }
                printf("%s", buf);
            }
            printf("\n%-4llu\n\n", i);
        }
    }

    //---------------------------------------
    if (DBG::debug_level >= 7) {
        double scale = 0;
        if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
            scale = CFG::cfg->atlas_scale_factor;
        } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS ||
                   CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
            scale = CFG::cfg->ads_scale_factor;
        }
        printf("[%d] scaled factor: %0.20f\n", DBG::debug_level, scale);

        std::vector<int> channels{1};
        // int ch = MW::mw->adc_page()->chart1()->channel();
        // if (ch > 0 && ch < 5) {
        //     channels.emplace_back(1);
        // }
        // ch = MW::mw->adc_page()->chart2()->channel();
        // if (ch > 0 && ch < 5) {
        //     channels.emplace_back(2);
        // }

        size_t i;
        auto *ptr = (uint32_t *)mem;
        for (int channel : channels) {
            printf("channel-%d (scaled bytes):\n", channel);
            double val;
            printf("%-4d : ", 0);
            for (i = 0; i < packet_atlas_t::npnts; i += 16) {
                size_t remains = packet_atlas_t::npnts - i;
                if (i > 0 && (i % 16) == 0) {
                    printf("\n%-4llu : ", i);
                }
                buf[0] = 0;
                char *p = buf;
                for (size_t n = 0; n < std::min((size_t)16, remains); ++n) {
                    if (*ptr != 0) {
                        val = 0;
                    }
                    val = (float)(*ptr);
                    val *= scale;
                    ln = snprintf(p, 10, "%8.6f ", val);
                    p += ln;
                    ptr++;
                }
                printf("%s", buf);
            }
            printf("\n\n");
        }
    }
    puts("");
}
