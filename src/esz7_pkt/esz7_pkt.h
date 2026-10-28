/**
  \file   esz7_pkt.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  The packet content
  */
#ifndef ESZ7_PACKET_H
#define ESZ7_PACKET_H

#include <stdbool.h>

#include "esz7_pkt_t.h"

#define MAX_PCKT_SIZE 1024

#ifdef __cplusplus
namespace ESZ7PKT {
extern "C" {
#endif

esz7_pkt_t *esz7_pkt(const esz7_pkt_t *packet);
esz7_pkt_t *esz7_pkt_create(const char *data, int datalen);
void *esz7_pkt_ptr(const esz7_pkt_t *packet);
const esz7_pkt_hdr_t *esz7_pkt_header(const esz7_pkt_t *packet);

bool  esz7_pkt_check(const char *bytes, int byteslen, const char **errormsg); // errormsg - maybe null
bool  esz7_pkt_update(esz7_pkt_t **packet, void *bytes);
void  esz7_pkt_set_data(esz7_pkt_t **packet, const char *data, int datalen);
void  esz7_pkt_remove_data(esz7_pkt_t **packet);
void  esz7_pkt_free(esz7_pkt_t **packet);

int   esz7_pkt_size(const esz7_pkt_t *packet);
int   esz7_pkt_hdr_size(const esz7_pkt_t *packet);
int   esz7_pkt_data_len(const esz7_pkt_t *packet);
const char *esz7_pkt_data(const esz7_pkt_t *packet, int *datalen);

esz7_pkt_t *esz7_pkt_make_cmd(int cmd);
esz7_pkt_t *esz7_pkt_make_read_cmd(int cmd, int regno);
esz7_pkt_t *esz7_pkt_make_write_cmd(int cmd, int regno, uint32_t value);

const char *esz7_pkt_str_error(int err);
const char *esz7_pkt_cmd_string(pckt_cmd_t cmd);

void  esz7_pkt_dbg_print(const esz7_pkt_t *packet);
void  data_hexdump(const char *data, size_t datalen);

#ifdef __cplusplus
} // extern C
} // namespace ESZ7PKT
#endif


#endif // ESZ7_PACKET_H
