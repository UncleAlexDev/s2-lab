/**
  \file   wsa.cpp
  \author Aleksey Lekhtin
  \date   Jan 27, 2025
  \brief  WSA procedures
*/

#include <cstdio>

#ifdef __linux__
#include <errno.h>
#include <string.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#endif

#include "ok.h"
#include "wsa.h"

#ifdef __cplusplus
namespace WSA {
#endif

bool wsa_startup()
{
#ifdef _WIN32
    WSADATA wsaData;
    const int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        printf("%s \n", str_error(ret));
        printf("WSAStartup failed with error: %d \n", ret);
        return FAIL;
    }
#endif
    return OK;
}

void wsa_cleanup()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

int get_error()
{
#ifdef __linux__
    return errno;
#endif
#ifdef _WIN32
    return WSAGetLastError();
#endif
}

const char *str_error(int err)
{
    static char buf[1024 * 4] = {0};
#ifdef __linux__
    //snprintf(buf, sizeof(buf), "(%d) %s", err, strerror(err));
    snprintf(buf, sizeof(buf), "(%d) %s", err, gai_strerror(err));
    return buf;
#endif

#ifdef _WIN32
    char *s = NULL;
    DWORD num =
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&s, 0, NULL);
    // snprintf(buf, sizeof(buf) - 1, "(%d)\n%s", err, s);
    snprintf(buf, sizeof(buf) - 1, "%s", s);
    if (buf[num - 1] == '\n')
        buf[num - 1] = '\0';
    LocalFree(s);

#if 0
    s = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &s, 0, NULL);
    //// strcat(buf, "\n");
    snprintf(buf + strlen(buf), sizeof(buf)-strlen(buf), "%s", s);
    if (buf[0] == '\n') buf[0] = '\0';
    LocalFree(s);
#endif
    return buf;
#endif
}

#if 0
#ifdef _WIN32

#include "cfg.h"

    DWORD dwLanguageId = LANG_NEUTRAL;
    //if (CFG::locale_ru) {
    //    dwLanguageId = LANG_RUSSIAN;
    //}

    /*
    // 1.
    char *s = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(dwLanguageId, SUBLANG_DEFAULT), (LPSTR)&s, 0, NULL);
    snprintf(buf, sizeof(buf) - 1, "(%d) %s", err, s);
    int len = strlen(buf);
    if (len >= 2) {
        if (buf[len - 2] == '\r' && buf[len - 1] == '\n' ) {
            buf[len - 2] = '\0';
        } else if (buf[len - 1] == '\n' ) {
            buf[len - 1] = '\0';
        }
    }
    LocalFree(s);
    return buf;
    */

#if 1
    char *s = NULL;
    int len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL, err, MAKELANGID(dwLanguageId, SUBLANG_DEFAULT), (LPSTR)&s, sizeof(buf)-1, NULL);
    //// printf("s:%d\n", (int)strlen(s));
    memcpy(buf, s, len);
    //// printf("b:%d\n", (int)strlen(buf));
    if (len >= 2) {
        if (buf[len - 2] == '\r' && buf[len - 1] == '\n' ) {
            buf[len - 2] = '\0';
        } else if (buf[len - 1] == '\n' ) {
            buf[len - 1] = '\0';
        }
    }
    //// printf("b:%d\n", (int)strlen(buf));
    //// printf("%s\n", buf);
    return buf;
#else
    static wchar_t wbuf[1024 * 4] = {0};
    int wbuflen = sizeof(wbuf)/sizeof(wbuf[0]);
    int len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err,  MAKELANGID(dwLanguageId, SUBLANG_DEFAULT), (LPWSTR)buf, wbuflen-1, NULL);
    printf("len:%d\n", len);
    printf("b:%d\n", (int)strlen(buf));
    return (const char*)wbuf;
#endif

#endif // _WIN32

#endif

#ifdef __cplusplus
}
#endif
