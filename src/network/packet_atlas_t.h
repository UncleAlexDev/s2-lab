/**
  \file   packet_atlas_t.h
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  The packet structure
*/

#ifndef PACKET_STRUCT_ATLAS_H_
#define PACKET_STRUCT_ATLAS_H_

#include <cstdint>
#include <sys/types.h>
#include <vector>

/**
 * RFC3550 RTP: A Transport Protocol for Real-Time Applications
 * @ref https://datatracker.ietf.org/doc/html/rfc3550
 */
struct __attribute__((packed, aligned(1))) rtp_fixed_part_t
{
    // The first octet
    uint16_t bits;          // ver/pad/ext/cc/mark/payload type
    uint16_t seqnum;        // sequence number
    // The second octet
    uint32_t timerange;     // период, за еоторый получены измерения (мсек)
    // The third octet
    uint32_t SSRC;          // synchronization source
};

struct ext_hdr_t
{
    uint16_t implementation;
    uint16_t total_recs;
};

struct rtp_t
{
    rtp_fixed_part_t fixed_part;
    uint32_t CSRC[15];
    ext_hdr_t ext_header;
    std::vector<uint32_t> ext_data;
};

typedef union __attribute__((packed, aligned(4))) misc_t {
    struct {
        uint32_t proto_ver:2;
        uint32_t unused:30;
    } bits;
    uint32_t dword;
} misc_t;

typedef struct __attribute__((packed, aligned(4))) data_header_t
{
    int32_t encoder1;
    uint32_t data_size;     // 500 points x sizeof(uint32_t)
    int32_t encoder2;       // former rezerv_1
    misc_t misc;            // former rezerv_2
    uint32_t rezerv_3;
} data_hdr_t;

/**
 * Формат пакета Атлас-а
 */
typedef struct packet_atlas_t
{
    static constexpr size_t npnts = 500;

    rtp_t rtp{};
    data_hdr_t data_header{};

    // stuff
    uint32_t rtp_size{};
    int32_t *m_raw_points1{};              // pressure1
    int32_t *m_raw_points2{};              // pressure2
    int32_t *m_raw_points3{};              // hookload
    int32_t *m_raw_points4{};              // channel-4
} atlas_packet_t;

#endif // PACKET_STRUCT_ATLAS_H_
