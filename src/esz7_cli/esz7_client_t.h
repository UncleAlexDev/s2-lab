/**
  \file   esz7_client_t.h
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  ESZ7 client type description
*/

#ifndef ESZ7_CLIENT_T_H_
#define ESZ7_CLIENT_T_H_

#include "esz7_pkt_t.h"
#include "thr.h"

#if _WIN32
#include <winsock2.h>
#endif

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
#define closesocket(fd) close(fd)
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

struct esz7_client_socket_t
{
    const char  *ipv4;
    const char  *host;
    int         port;
    SOCKET      fd;
    SOCKADDR    sockaddr;
    SOCKADDR_IN sockaddrin;
};

struct esz7_client_t
{
    char        hostname[256];
    int         port;
    char        sPort[8];
    thr_t       *thr;
    esz7_pkt_t  *request;
    esz7_pkt_t  *response;
    void        *response_obj;
    void        (*response_callback)(void *obj, void *arg, int size);
    void        *errors_obj;
    void        (*error_callback)(void *obj, void *arg, int size);
    esz7_client_socket_t sock;
};

#endif // ESZ7_CLIENT_T_H_
