/**
  \file   esz7_client_t.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  ESZ7 client type description
*/

#ifndef esz7_client_H_
#define esz7_client_H_


#include "esz7_client_t.h"

#ifdef __cplusplus
namespace ESZ7CLIENT {
extern "C" {
#endif

esz7_client_t *esz7_client_create(const char *host, int port);
bool esz7_client_run(esz7_client_t *cli);
void esz7_client_stop(esz7_client_t **cli);
void esz7_clients_stop();
void esz7_client_socket_close(esz7_client_t *cli);

void esz7_client_set_debug_level(int level);
const char *esz7_client_strerror();

void esz7_client_set_response_callback(esz7_client_t *cli, void *obj, void (*callback)(void *obj, void *arg, int size));
void emit_response(const esz7_client_t *cli);

void esz7_client_set_error_callback(esz7_client_t *cli, void *obj, void (*callback)(void *obj, void *arg, int size));
void emit_error(const esz7_client_t *cli);

void esz7_client_send_packet(esz7_client_t *cli, const esz7_pkt_t *packet);

#ifdef __cplusplus
} // extern C
} // namespace ESZ7CLIENT
#endif


#endif // esz7_client_H_
