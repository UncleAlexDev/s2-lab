/**
  \file   esz7_client.c
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  The client for control the ESZ7 ADC board.
*/

#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <ws2tcpip.h>

#include "dbg.h"
#include "wsa.h"

#include "esz7_client.h"
#include "esz7_pkt.h"

#ifdef __cplusplus
namespace ESZ7CLIENT {
    extern "C" {
#endif


static esz7_client_t *m_esz7_client[1];
static int m_clients_total;

static char error_msg[1024] = {0};

static bool cli_connect(esz7_client_t *cli);
static bool cli_disconnect(esz7_client_t *cli);
static bool send_request(const esz7_client_t *cli);
static void *esz7_client_thread_func(void *arg);
static void client_doing_his_job(esz7_client_t *cli);

esz7_client_t *esz7_client_create(const char *host, int port)
{
    int clients_capacity = sizeof(m_esz7_client) / sizeof(m_esz7_client[0]);
    if (m_clients_total >= clients_capacity) {
        fprintf(stderr, "--- no more space for new client instance (total %d)\n", clients_capacity);
        assert(m_clients_total >= clients_capacity);
        return nullptr;
    }

    auto cli = (esz7_client_t *)calloc(1, sizeof(esz7_client_t));
    thr_t *thr = thr_create("client", esz7_client_thread_func, cli);
    if ((intptr_t)thr < 0) {
        int e = -(int)(intptr_t)thr;
        snprintf(error_msg,  sizeof(error_msg)-1,
                 "Can't create thread for ESZ7 board client\n%s",
                 strerror(e));
        return nullptr;
    }

    cli->thr = thr;
    cli->sock.fd = INVALID_SOCKET;
    strncpy(cli->hostname, host, sizeof(cli->hostname)-1);
    cli->port = port;
    snprintf(cli->sPort, sizeof(cli->sPort)-1, "%d", cli->port);

    m_esz7_client[m_clients_total] = cli;
    m_clients_total++;

    return cli;
}

bool esz7_client_run(esz7_client_t *cli)
{
    int err = thr_run(cli->thr);
    if (err > 0) {
        snprintf(error_msg,  sizeof(error_msg)-1,
                 "Can't start thread for ESZ7 board client\n%s",
                 strerror(err));
        return FAIL;
    }
    return OK;
}

void esz7_client_socket_close(esz7_client_t *cli)
{
    cli_disconnect(cli);
}

void esz7_clients_stop()
{
    while(m_clients_total) {
        int i = m_clients_total-1;
        esz7_client_stop(&m_esz7_client[i]);
        m_clients_total--;
    }
}

void esz7_client_stop(esz7_client_t **cli)
{
    if (!cli || !(*cli)) {
        return;
    }
    esz7_client_socket_close((*cli));
    ESZ7PKT::esz7_pkt_free(&(*cli)->request);
    ESZ7PKT::esz7_pkt_free(&(*cli)->response);
    free(*cli);
    *cli = nullptr;
}

void esz7_client_set_debug_level(int level)
{
    DBG::debug_level = level;
}

const char *esz7_client_strerror()
{
    return error_msg;
}

//int esz7_client_is_work(esz7_client_t *cli)
//{
//    return (cli->thr ? true : false);
//}
//
//void esz7_client_stop(esz7_client_t *cli)
//{
//    DBGF(4, "stop client's thread[%s] id: %d\n", cli->thr->name, cli->thr->id);
//    thr_destroy(cli->thr);
//    cli->thr->func = nullptr;
//    cli->thr = nullptr;
//}
//

void esz7_client_set_response_callback(esz7_client_t *cli, void *obj, void (*callback)(void *obj, void *arg, int size))
{
    cli->response_obj = obj;
    cli->response_callback = callback;
}

void esz7_client_set_error_callback(esz7_client_t *cli, void *obj, void (*callback)(void *obj, void *arg, int size))
{
    cli->errors_obj = obj;
    cli->error_callback = callback;
}

void emit_response(const esz7_client_t *cli)
{
    DBG(4);
    if (!cli->response_callback) {
        DBG(1, "callback is zero - unspecified - skip\n");
        return;
    }
    int size = ESZ7PKT::esz7_pkt_size(cli->response);
    cli->response_callback(cli->response_obj, cli->response, size);
}

void emit_error(const esz7_client_t *cli)
{
    DBG(4);
    if (!cli->error_callback) {
        DBG(1, "callback is zero - unspecified - skip\n");
        return;
    }
    int sz = ESZ7PKT::esz7_pkt_size(cli->response);
    cli->error_callback(cli->errors_obj, (void *)cli->response, sz);
}

void esz7_client_send_packet(esz7_client_t *cli, const esz7_pkt_t *packet)
{
    DBGF(3, "packet size: {} bytes, data: {} bytes",
         ESZ7PKT::esz7_pkt_size(packet),
         ESZ7PKT::esz7_pkt_data_len(packet));
    ESZ7PKT::esz7_pkt_dbg_print(packet);

    ESZ7PKT::esz7_pkt_free(&cli->request);
    cli->request = ESZ7PKT::esz7_pkt(packet); // make deep copy!

    DBG(3, "wakeup thr id: %d\n", cli->thr->id);
    thr_wakeup(cli->thr->id);
}

static bool cli_connect(esz7_client_t *cli)
{
    if (cli->sock.fd != INVALID_SOCKET) {
        DBGF(4, "-- already connected to '%s:%d'\n", cli->hostname, cli->port);
        return OK;
    }

    DBGF(3, "connect to '{}:{}'", cli->hostname, cli->port);
    if (cli->hostname[0] == 0) {
        snprintf(error_msg, sizeof(error_msg)-1, "The target host name does not specified");
        return FAIL;
    }

    if (cli->port == 0) {
        snprintf(error_msg, sizeof(error_msg)-1, "The connection port does not specified");
        return FAIL;
    }

    struct addrinfo hints{};
    struct addrinfo *result, *rp;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    int ret = getaddrinfo(cli->hostname, cli->sPort, &hints, &result);
    if (ret != 0) {
        int err = WSA::get_error();
#ifdef _WIN32
        const char *msg = WSA::str_error(err);
        strncpy(error_msg, WSA::str_error(err), sizeof(error_msg)-1);
        DBG(1, " ERROR: getaddrinfo(): %s\n", msg);
#endif
#ifdef __linux
        strncpy(error_msg, gai_strerror(err), sizeof(error_msg)-1);
        DBG(1, " ERROR: getaddrinfo(): %s\n", gai_strerror(err));
#endif
        cli_disconnect(cli);
        return FAIL;
    }

    int err = 0;
    const char *errmsg = nullptr;
    SOCKET fd;
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == INVALID_SOCKET) {
            continue;
        }

#ifdef __linux__
        // TODO
        //int syn_retries = 2; // Send a total of 3 SYN packets => Timeout ~7s
        //if (setsockopt(fd, IPPROTO_TCP, TCP_SYNCNT, &syn_retries, sizeof(syn_retries))) == 0) {
        //       DBG(1, "setsockopt(SO_REUSEADDR) to %d\n", syn_retries);
        //}
#endif
        ret = connect(fd, rp->ai_addr, (socklen_t)rp->ai_addrlen);
        if (ret != SOCKET_ERROR) {
            break;
        }
        err = WSA::get_error();
#ifdef _WIN32
        errmsg = WSA::str_error(err);
        DBG(1, " ERROR: connect(): %s\n", errmsg);
#endif
#ifdef __linux
        errmsg = gai_strerror(err);
        DBG(1, " ERROR: connect(): %s\n", errmsg);
#endif
        closesocket(fd);
    }
    freeaddrinfo(result);

    if (rp == nullptr) {
        DBGF(1, "failed to connect to {}:{}", cli->hostname, cli->port);
        snprintf(error_msg, sizeof(error_msg)-1, "Failed to connect to %s:%d\n\n%s",
                 cli->hostname, cli->port, errmsg);
        return FAIL;
    }
    cli->sock.fd = fd;
    DBGF(3, "connected to {}:{} (fd: {})", cli->hostname, cli->port, cli->sock.fd);

#ifdef __linux__
    const int enable = 1;
#endif
#ifdef _WIN32
    DWORD enable = 1;
#endif
    if (setsockopt(cli->sock.fd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(enable)) < 0) {
        DBG(2, "setsockopt(SO_REUSEADDR) failed");
    } else {
        DBGF(2, "setsockopt(SO_REUSEADDR) to {}", enable);
    }

    int timeout_sec = 7;
#ifdef __linux__
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
#endif
#ifdef _WIN32
    DWORD timeout = timeout_sec * 1000; // ms
#endif
    ret = setsockopt(cli->sock.fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (ret == SOCKET_ERROR) {
        err = WSA::get_error();
        DBGF(0, "ERROR: ({}) - setsockopt return {}\n{}\n", err, ret, WSA::str_error(err));
        snprintf(error_msg, sizeof(error_msg)-1, "Can't set receive timeout (SO_RCVTIMEO)\n%s", WSA::str_error(err));
        cli_disconnect(cli);
        return FAIL;
    }
    DBGF(2, "setsockopt(SO_RCVTIMEO) to %d\n", timeout_sec);
    DBGF(4, "response timeout set in %d sec\n", timeout_sec);
    return OK;
}

static bool cli_disconnect(esz7_client_t *cli)
{
    if (cli->sock.fd == INVALID_SOCKET) {
        return FAIL;
    }
    DBGF(3, "disconnecting from %s:%d (fd: {})\n", cli->hostname, cli->port, cli->sock.fd);
    closesocket(cli->sock.fd);
    cli->sock.fd = INVALID_SOCKET;
    DBGF(4, "socket fd: %d\n\n", cli->sock.fd);
    return OK;
}

static bool send_request(const esz7_client_t *cli)
{
    esz7_pkt_t *packet = cli->request;
    assert(packet != nullptr);
    int sz = ESZ7PKT::esz7_pkt_size(packet);
    DBGF(3, "send {} bytes...", sz);

    int ret = send(cli->sock.fd, (char *)packet, sz, 0);
    if (ret != ESZ7PKT::esz7_pkt_size(packet)) {
        int err = WSA::get_error();
        snprintf(error_msg, sizeof(error_msg)-1, "The request sending failed\n%s", WSA::str_error(err));
        DBGF(4, "esz7_client_thr_func::%s(): ERROR: (%d)\n%s\n", err, error_msg);
        return FAIL;
    }
    return OK;
}

static bool receive_response(esz7_client_t *cli)
{
    DBG(3);
    char buf[MAX_PCKT_SIZE] = {0};
    // TODO принимать esz7_prt_t и докачивать данные  - в цикле.
    int nread = (int)recv(cli->sock.fd, buf, MAX_PCKT_SIZE, 0);
    DBGF(4, "esz7_client_thr_func::%s(): responsed with %d bytes\n", nread);
    if (nread == SOCKET_ERROR) {
        int err = WSA::get_error();
#ifdef __linux__
        if (errno == EAGAIN) {
            DBG(1, " EAGAIN (timeout)\n");
        }
        if (errno == EWOULDBLOCK) {
            DBG(1, " EWOULDBLOCK (timeout)\n");
        }
#endif
#ifdef _WIN32
        if (err == WSAETIMEDOUT) {
            DBG(1, " WSATIMEDOUT\n");
        }
#endif
        DBGF(4, "esz7_client_thr_func::%s() ERROR: failed to receive response\n%s\n",
            WSA::str_error(err));
        snprintf(error_msg, sizeof(error_msg)-1, "The response receiving failed\n\n%s", WSA::str_error(err));
        return FAIL;
    }

    if (nread == 0) {
        DBG(4, "lost connection");
        snprintf(error_msg, sizeof(error_msg)-1, "The client has dropped the connection");
        return FAIL;
    }

    ESZ7PKT::esz7_pkt_free(&cli->response);
    cli->response = ESZ7PKT::esz7_pkt((const esz7_pkt_t *)buf);
    return OK;
}

void *esz7_client_thread_func(void *arg)
{
    int ret = 0;
    auto *thr = (thr_t *)arg;
    auto *cli = (esz7_client_t *)thr->obj;

    for(;;) {
        pthread_mutex_lock(&thr->mutex);
        while (thr->wakeup == 0 && ret == 0) {
            DBG(4, "pthread_cond_wait() - waiting for next event\n");
            ret = pthread_cond_wait(&thr->cond, &thr->mutex);
        }
        thr->wakeup = 0;

        if (thr->cmd_to_stop == 1) {
            break;
        }

        client_doing_his_job(cli);

        pthread_mutex_unlock(&thr->mutex);
    }

    DBGF(4, "thr[{}] #{} finished (wakeup: {}, cmd_to_stop: {})",
         thr->name, thr->id, thr->wakeup, thr->cmd_to_stop);
    thr->wakeup = 0;
    thr->cmd_to_stop = 0;
    cli_disconnect(cli);
    pthread_mutex_unlock(&thr->mutex);
    return nullptr;
}

static void client_doing_his_job(esz7_client_t *cli)
{
    for(;;) {
        DBGF(3, "connect to '{}:{}'\n", cli->hostname, cli->port);
        if (!cli_connect(cli)) {
            DBGF(3, "can't connect to {}:{}\n",  __func__, cli->hostname, cli->port);
            ESZ7PKT::esz7_pkt_free(&cli->response);
            cli->response = ESZ7PKT::esz7_pkt(cli->request);
            ESZ7PKT::esz7_pkt_set_data(&cli->response, error_msg, (int)strlen(error_msg));
            emit_error(cli);
            break;
        }

        if (!send_request(cli)) {
            DBG(4, "can't send request");
            ESZ7PKT::esz7_pkt_free(&cli->response);
            cli->response = ESZ7PKT::esz7_pkt(cli->request);
            ESZ7PKT::esz7_pkt_set_data(&cli->response, error_msg,(int) strlen(error_msg));
            emit_error(cli);
            break;
        }
        DBGF(4, "sent {}", ESZ7PKT::esz7_pkt_cmd_string(cli->request->cmd));

        if (!receive_response(cli)) {
            DBG(3, "can't get response");
            ESZ7PKT::esz7_pkt_free(&cli->response);
            cli->response = ESZ7PKT::esz7_pkt(cli->request);
            ESZ7PKT::esz7_pkt_set_data(&cli->response, error_msg, (int)strlen(error_msg));
            cli->response->cmd = PKT_CMD_MESG;
            emit_error(cli);
            break;
        }
        DBG(4, "got response");
        emit_response(cli);

        break;
    } // for(;;)

    cli_disconnect(cli);
}

#ifdef __cplusplus
} // extern C
} // namespace ESZ7CLIENT
#endif
