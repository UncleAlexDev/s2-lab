/**
  \file   packet_struct.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  The packet structure
*/

#ifndef PACKET_STRUCT__H_
#define PACKET_STRUCT__H_

#include <stdint.h>

#define PKT_SIGN "ESZ7"
#define PKT_VER 0x01

#define PKT_HDR_SIZE 55

typedef enum pckt_error_t
{
    PCKT_NONE = 0,
    PCKT_CHECK_ERROR,
    PCKT_SEND_ERROR,
    PCKT_RECV_ERROR,
} pckt_error_t;

/**
 * Header of the packet being sent.
 */
typedef struct header_t
{
    uint8_t     sign[4];
    uint8_t     ver;
    int         header_size;
    int         packet_size;
    int         data_len;
    int         errcode;        // error code if data - not valid
    uint8_t     reserv[32];
    uint8_t     mark;
    uint8_t     zero;
} __attribute__((packed, aligned(1))) packet_header_t;

// typedef struct __attribute__((packed, aligned(1))) header_t

typedef struct packet_t
{
    packet_header_t header;
    uint8_t         cmd;
    uint8_t         regno;
} packet_t;

#endif // PACKET_STRUCT__H_
