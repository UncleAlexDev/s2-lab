/**
  \file   esz7_pkt_t.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  The packet structure
*/

#ifndef ESZ7_PACKET_STRUCT_H
#define ESZ7_PACKET_STRUCT_H

#include <cstdint>

#define PKT_SIGN "ESZ7"
#define PKT_VER 0x01

#define PKT_HDR_SIZE 60

typedef enum pckt_error_t
{
    PKT_OK = 0,
    PKT_WRONG,
    PKT_CHECK_ERROR,
    PKT_SEND_ERROR,
    PKT_RECV_ERROR,
} pckt_error_t;

typedef enum pckt_cmd_t
{
    PKT_CMD_NONE = (uint8_t)0,
    PKT_CMD_MESG,
    PKT_CMD_PING,
    PKT_CMD_PONG,
    PKT_CMD_READ_REG,
    PKT_CMD_WRITE_REG,
} pckt_cmd_t;

/**
 * Header of the packet being sent.
 */
typedef struct esz7_pkt_hdr_t
{
    uint8_t     sign[4];
    uint8_t     ver;
    int         hdr_size;
    int         pckt_size;
    int         data_len;
    uint8_t     errcode;        // error code if data - not valid
    uint8_t     reserv[40];
    uint8_t     mark;
    uint8_t     zero;
} __attribute__((packed, aligned(1))) esz7_pkt_hdr_t;

// typedef struct __attribute__((packed, aligned(1))) header_t

typedef struct esz7_pkt_t
{
    esz7_pkt_hdr_t header;
    pckt_cmd_t     cmd;
    uint8_t        regno;
    uint32_t       value;
    uint8_t        reserv1;
    uint8_t        reserv2;
    // any data bytes followed by esz7_pkt_t
} esz7_pkt_t;

#endif // ESZ7_PACKET_STRUCT_H
