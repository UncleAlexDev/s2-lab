/**
  \file   packet.c
  \author Aleksey Lekhtin
  \date   April 1, 2024
  \brief  The packet content
  */

#include <cassert>
#include <malloc.h>
#include <cstdio>
#include <cstring>

#ifdef __linux__
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#endif

#include "dbg.h"
#include "ok.h"
#include "esz7_pkt.h"

static_assert(sizeof(esz7_pkt_hdr_t) == PKT_HDR_SIZE, "Wrong packet header size.");

#ifdef __cplusplus
namespace ESZ7PKT {
    extern "C" {
#endif

/**
 * @brief Create copy of packet
 * @param packet - incoming packet
 * @return created copy of packet
 */
esz7_pkt_t *esz7_pkt(const esz7_pkt_t *packet)
{
    int packetsize = esz7_pkt_size(packet);
    auto *pkt = (esz7_pkt_t *)calloc(1, packetsize);
    assert(pkt != nullptr);
    memcpy(pkt, packet, packetsize);
    return pkt;
}

esz7_pkt_t *esz7_pkt_create(const char *data, int datalen)
{
    if (datalen > MAX_PCKT_SIZE) {
        DBGF(0, "data is too huge ({} > {})", datalen, MAX_PCKT_SIZE);
        return nullptr;
    }

    int packetsize = (int)sizeof(esz7_pkt_t) + datalen;
    auto *pckt = (esz7_pkt_t *)calloc(1, packetsize);
    pckt->header = (esz7_pkt_hdr_t) {
            .sign        = {'E','S','Z','7'}, // PKT_SIGN
            .ver         = PKT_VER,
            .hdr_size    = (int)htonl(sizeof(esz7_pkt_hdr_t)),
            .pckt_size   = (int)htonl(packetsize),
            .data_len    = (int)htonl(datalen),
            .errcode     = PKT_WRONG,
            .reserv      = {},
            .mark        = '@',
            .zero        = 0,
};

    for(int i = 0; i < (int)sizeof(pckt->header.reserv); i++) {
        pckt->header.reserv[i] = (i%2 ? 0x77 : 0x55);
    }
    pckt->header.reserv[0] = 0xAA;

    if (datalen > 0) {
        char *data_ptr = (char *)pckt + sizeof(esz7_pkt_t);
        memcpy(data_ptr, data, datalen);
    }

    DBGF(4, "hdr_t: {}, pkt_t: {}, data: {}, total size: {}\n",
         (int)sizeof(esz7_pkt_hdr_t),
         (int)(sizeof(esz7_pkt_t) - sizeof(esz7_pkt_hdr_t)),
         (int)ntohl(pckt->header.data_len),
         (int)ntohl(pckt->header.pckt_size));
    return pckt;
}

/**
 * @brief Задать новые данные пакета
 * @param packet - пакет
 * @param data - данные (могут быть заданы nullptr)
 * @param datalen - размер данных (0 - пакета будут удалены данные)
 */
void esz7_pkt_set_data(esz7_pkt_t **packet, const char *data, int datalen)
{
    assert(packet != nullptr);
    assert(*packet != nullptr);
    int new_pktsz = (int)sizeof(esz7_pkt_t) + datalen;
    auto *p = (esz7_pkt_t *)realloc(*packet, new_pktsz);
    assert(p != nullptr);
    if (datalen > 0) {
        char *data_ptr = (char *)p + sizeof(esz7_pkt_t);
        memcpy(data_ptr, data, datalen);
    }

    p->header.data_len = (int)htonl(datalen);
    p->header.pckt_size = (int)htonl(new_pktsz);
    *packet = p;
}

void esz7_pkt_remove_data(esz7_pkt_t **packet)
{
    esz7_pkt_set_data(packet, nullptr, 0);
}

bool esz7_pkt_update(esz7_pkt_t **packet, void *bytes)
{
    DBG(3);
    const char *errmsg = nullptr;
    if (!esz7_pkt_check((char *)(*packet), (*packet)->header.pckt_size, &errmsg)) {
        DBGF(1, "check current packet storage:\n{}", errmsg);
        return FAIL;
    }
    DBG(3, "check current packet storage - OK");

    auto *new_packet = (esz7_pkt_t *)bytes;
    if (!esz7_pkt_check((char *)new_packet, new_packet->header.pckt_size, &errmsg)) {
        DBG(0, "check new packet storage:\n{}", errmsg);
        return FAIL;
    }
    DBG(3, "check new packet storage - OK");

    // пересоздавать текущий пакет, если предыдущие данные были большего размера
    if (esz7_pkt_size(*packet) > (MAX_PCKT_SIZE/2)) {
        esz7_pkt_free(packet);
        *packet = esz7_pkt_create(nullptr, 0);
    }

    char *new_data      = (char *)new_packet + sizeof(esz7_pkt_hdr_t);
    int new_datalen     = (int)ntohl(((esz7_pkt_hdr_t *)new_packet)->data_len);
    int current_datalen = (int)ntohl(((esz7_pkt_hdr_t *)(*packet))->data_len);

    if (new_datalen == current_datalen) {
        char *data = (char *)(*packet) + sizeof(esz7_pkt_hdr_t);
        memcpy(data, new_data, current_datalen);
    } else if (new_datalen < current_datalen) {
        // reuse memory
        char *data = (char *)(*packet) + sizeof(esz7_pkt_hdr_t);
        memcpy(data, new_data, new_datalen);
        (*packet)->header.data_len = new_datalen;
        (*packet)->header.pckt_size = (int)sizeof(esz7_pkt_hdr_t) + new_datalen;
    } else {
        // resize memory
        int sz = (int)sizeof(esz7_pkt_hdr_t) + new_datalen;
        auto *p = (esz7_pkt_t *)realloc(*packet, sz);
        if (!p) {
            int e = errno;
            fprintf(stderr, "(%d) %s]\n", e, strerror(e));
            return FAIL;
        }
        memcpy(p, new_packet, ntohl(new_packet->header.pckt_size));
        *packet = p;
    }

    if (DBG::debug_level >= 2) {
        auto *header = (esz7_pkt_hdr_t *)(*packet);
        DBGF(2, "packet updated - size: {}, datalen: {}\n",
             (int)ntohl(header->pckt_size),
             (int)ntohl(header->data_len));
    }
    return OK;
}

esz7_pkt_t *esz7_pkt_make_cmd(int cmd)
{
    esz7_pkt_t *packet = esz7_pkt_create(nullptr, 0);
    packet->cmd = (pckt_cmd_t)cmd;
    DBGF(4, "cmd: {} ({})", (int)packet->cmd, esz7_pkt_cmd_string(packet->cmd));
    return packet;
}

esz7_pkt_t *esz7_pkt_make_read_cmd(int cmd, int regno)
{
    esz7_pkt_t *packet = esz7_pkt_create(nullptr, 0);
    packet->cmd = (pckt_cmd_t)cmd;
    packet->regno = (uint8_t)regno;
    DBGF(4, "cmd: {} ({}), regno: {}",
         (int)packet->cmd, esz7_pkt_cmd_string(packet->cmd), packet->regno);
    return packet;
}

esz7_pkt_t *esz7_pkt_make_write_cmd(int cmd, int regno, uint32_t value)
{
    esz7_pkt_t *packet = esz7_pkt_create(nullptr, 0);
    packet->cmd = (pckt_cmd_t)cmd;
    packet->regno = (uint8_t)regno;
    packet->value = value;
    DBGF(4, "cmd: {} ({}), regno: {}, value: 0x{:08X}",
         (int)packet->cmd, esz7_pkt_cmd_string(packet->cmd), packet->regno, packet->value);
    return packet;
}

/**
 * Just print data in hex form
 * @param[in] data - byte sequence
 * @param[in] datalen - length of sequence
 */
void data_hexdump(const char *data, size_t datalen)
{
    if (!data) {
        DBG(0, "no data (null). skip");
        return;
    }

    DBGF(4, "{} bytes:", datalen);
    if (DBG::debug_level >3 ) {
        printf("0   : ");
        for (size_t i = 0; i < datalen; i++) {
            if (i > 0 && (i % 16) == 0) {
                printf("\n%-4llu ", i);
            }
            printf("%02X ", (unsigned char)data[i]);
        }
        puts("\n");
    }
}

bool esz7_pkt_check(const char *bytes, int byteslen, const char **errormsg)
{
    static char buf[256] = {0};
    if (byteslen < (int)sizeof(esz7_pkt_hdr_t)) {
        if (errormsg) {
            snprintf(buf, sizeof(buf),
                     "BAD DATA - a less than the required header (size:%d < header:%d)",
                     byteslen, (int)sizeof(esz7_pkt_hdr_t));
            *errormsg = buf;
            DBG(3, buf);
        }
        return FAIL;
    }

    auto *packet = (esz7_pkt_t *)bytes;

    int hdr_size = esz7_pkt_hdr_size(packet);
    if (hdr_size != (int)sizeof(esz7_pkt_hdr_t)) {
        if (DBG::debug_level >= 5) {
            data_hexdump(bytes, byteslen);
        }
        if (errormsg) {
            snprintf(buf, sizeof(buf),
                     "BAD PACKET - unexpected header size (got size:%d != header_t:%d)",
                     hdr_size, (int)sizeof(esz7_pkt_hdr_t));
            *errormsg = buf;
            DBG(3, buf);
        }
        return FAIL;
    }

    int pkt_size = esz7_pkt_size(packet);
    if (pkt_size > byteslen) {
        if (DBG::debug_level >= 5) {
            data_hexdump(bytes, byteslen);
        }
        if (errormsg) {
            snprintf(buf, sizeof(buf),
                     "BAD PACKET - size in header is greater than number of bytes (size:%d > bytes:%d)",
                     pkt_size, byteslen);
            *errormsg = buf;
            DBG(3, buf);
        }
        return FAIL;
    }

    if (pkt_size < (int)sizeof(esz7_pkt_hdr_t)) {
        if (DBG::debug_level >= 5) {
            data_hexdump(bytes, byteslen);
        }
        if (errormsg) {
            snprintf(buf, sizeof(buf),
                     "BAD PACKET - size in header a less than require (size:%d < header:%d)",
                     pkt_size, (int)sizeof(esz7_pkt_hdr_t));
            *errormsg = buf;
            DBG(3, buf);
        }
        return FAIL;
    }

    int datalen;
    const char *data = esz7_pkt_data(packet, &datalen);

    if (((int)sizeof(esz7_pkt_t) + datalen) != pkt_size) {
        if (DBG::debug_level >= 5) {
            data_hexdump(bytes, byteslen);
        }
        if(errormsg) {
            snprintf(buf, sizeof(buf), "BAD PACKET (header:%d + bytes:%d != size:%d)",
                     PKT_HDR_SIZE, datalen, pkt_size);
            *errormsg = buf;
            DBG(3, buf);
        }
        return FAIL;
    }

    if (data) {
        if (data != (char *)packet + (int)sizeof(esz7_pkt_t)) {
            if (DBG::debug_level >= 5) {
                data_hexdump(bytes, byteslen);
            }
            if (errormsg) {
                snprintf(buf, sizeof(buf), "BAD PACKET (data pointer points to incorrect space)");
                *errormsg = buf;
                DBG(3, buf);
            }
            return FAIL;
        }
    }

    DBGF(4, "OK (header: {}, bytes: {}, total size: {})",
         (int)sizeof(esz7_pkt_hdr_t), datalen, pkt_size);
    return OK;
}

void *esz7_pkt_ptr(const esz7_pkt_t *packet)
{
    assert(packet != nullptr);
    return (char *)packet;
}

const esz7_pkt_hdr_t *esz7_pkt_header(const esz7_pkt_t *packet)
{
    assert(packet != nullptr);
    return (esz7_pkt_hdr_t *)packet;
}

int esz7_pkt_size(const esz7_pkt_t *packet)
{
    assert(packet != nullptr);
    return (int)ntohl(packet->header.pckt_size);
}

int esz7_pkt_hdr_size(const esz7_pkt_t *packet)
{
    return (int)ntohl(packet->header.hdr_size);
}

int esz7_pkt_data_len(const esz7_pkt_t *packet)
{
    return (int)ntohl(packet->header.data_len);
}

const char *esz7_pkt_data(const esz7_pkt_t *packet, int *datalen)
{
    assert(packet != nullptr);
    int len = (int)ntohl(packet->header.data_len);
    if (datalen) *datalen = len;
    if (len == 0) {
        return nullptr;
    }
    char *ptr = (char *)packet + sizeof(esz7_pkt_t);
    return ptr;
}

void esz7_pkt_free(esz7_pkt_t **packet)
{
    if (packet && *packet) {
        free((void *)(*packet));
        *packet = nullptr;
    }
}

void esz7_pkt_dbg_print(const esz7_pkt_t *packet)
{
    if (DBG::debug_level <= 4) return;

    int pktsize = esz7_pkt_size(packet);
    const char *pkt = static_cast<const char *>(esz7_pkt_ptr(packet));
    if (!pkt) {
        DBG(DBG::debug_level, "%s(): just no pkt");
        return;
    }

    if (DBG::debug_level >= 5) {
        data_hexdump((char *)packet, pktsize);
    }

    const esz7_pkt_hdr_t *hdr = esz7_pkt_header(packet);
    DBGF(DBG::debug_level, "sign:     0x{:X}{:X}{:X}{:X} {}{}{}{}\n",
         hdr->sign[0], hdr->sign[1], hdr->sign[2], hdr->sign[3],
            hdr->sign[0], hdr->sign[1], hdr->sign[2], hdr->sign[3]);
    DBGF(DBG::debug_level, "ver:      0x{:02X}", hdr->ver);
    DBGF(DBG::debug_level, "hdr size: {}", (int)ntohl(hdr->hdr_size));
    DBGF(DBG::debug_level, "data:     {}", (int)ntohl(hdr->data_len));
    DBGF(DBG::debug_level, "size:     {}", (int)ntohl(hdr->pckt_size));
    DBGF(DBG::debug_level, "mark:     {}", hdr->mark);
    DBGF(DBG::debug_level, "cmd:      {} ({})", (int)packet->cmd, esz7_pkt_cmd_string(packet->cmd));
    DBGF(DBG::debug_level, "regno:    {}", packet->regno);
    DBGF(DBG::debug_level, "value:    0x{:08X}", packet->value);

    int datalen;
    const char *data = esz7_pkt_data(packet, &datalen);
    if (datalen > 0) {
        DBGF(DBG::debug_level, "data {} bytes: ", datalen);
        for(int i =0; i < datalen; i++) {
            char ch = data[i];
            printf("%c", (isascii(ch) ? ch : '.'));
        }
        puts("");
    }
}

const char *packet_str_error(int err)
{
    if (err == PKT_OK) return "success";
    if (err == PKT_CHECK_ERROR) return "wrong format";
    if (err == PKT_SEND_ERROR) return "send error";
    if (err == PKT_RECV_ERROR) return "receive error";
    return "unknown error";
}

const char *esz7_pkt_cmd_string(pckt_cmd_t cmd)
{
    if (cmd == PKT_CMD_NONE) return "NONE";
    if (cmd == PKT_CMD_MESG) return "MSG";
    if (cmd == PKT_CMD_PING) return "PING";
    if (cmd == PKT_CMD_PONG) return "PONG";
    if (cmd == PKT_CMD_READ_REG) return "READ-REG";
    if (cmd == PKT_CMD_WRITE_REG) return "WRITE-REG";
    return "UNK";
}

#ifdef __cplusplus
} // extern C
} // namespace ESZ7PKT
#endif
