/**
  \file   sig.c
  \author Aleksey Lekhtin
  \date   April 4, 2024
  \brief  catch signals
*/

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "dbg.h"
#include "ok.h"
#include "esz7_client.h"

#ifdef _WIN32
typedef __p_sig_fn_t sighandler_t;
#endif

#ifdef __cplusplus
namespace ESZ7_SIGNAL {
extern "C" {
#endif

//---------------------------------------------------------
static int sigs[] = {SIGINT, SIGABRT, SIGTERM, SIGSEGV, 0, 0};

//---------------------------------------------------------
inline static const char *signal_name(int snum)
{
    if (snum == SIGINT) {
        return "SIGINT";
    }
    if (snum == SIGABRT) {
        return "SIGABRT";
    }
    if (snum == SIGTERM) {
        return "SIGTERM";
    }
    if (snum == SIGSEGV) {
        return "SIGSEGV";
    }
#ifdef SIGBREAK
    if (snum == SIGBREAK) {
        return "SIGBREAK";
    }
#endif
    return "oops";
}

//---------------------------------------------------------
static void sigaction_handler(int snum)
{
    printf("\n--- Signal %s (%d) received\n", signal_name(snum), snum);
    if (snum == SIGSEGV)
    {
        printf("=== try shutdown... ===\n");
        DBG::debug_level = 4;
        ESZ7CLIENT::esz7_clients_stop();
        thr_destroy_all();
        auto ts = (struct timespec){
                      .tv_sec = 0,
                      .tv_nsec = 10000000,
                  };
        nanosleep(&ts, nullptr);
        printf("=== exit ===\n");
        exit(0);
    }

    //if (snum == SIGABRT)
    {
        printf("=== try exit... ===\n");
        DBG::debug_level = 4;
        thr_destroy_all_later();
        auto ts = (struct timespec){
            .tv_sec = 0,
            .tv_nsec = 10000000,
        };
        nanosleep(&ts, nullptr);
        ESZ7CLIENT::esz7_clients_stop();
        printf("=== exited ===\n");
        exit(0);
    }

    // restart signal actions
    // esz7_set_sig_action();
}

//---------------------------------------------------------
bool esz7_set_sig_action()
{
#ifdef __linux__
    struct sigaction old_sa = {0};
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigaction_handler;
#endif

#ifdef SIGBREAK
    int sz = sizeof(sigs)/sizeof(sigs[0]);
    sigs[sz-2] = SIGBREAK;
#endif
    for (int i = 0; sigs[i]; ++i)
    {
#ifdef __linux__
        if (sigaction(sigs[i], &sa, &old_sa) == -1)
            return FAIL;
#endif
#ifdef _WIN32
        sighandler_t old_sig = signal(sigs[i], sigaction_handler);
        if (old_sig == SIG_ERR) return FAIL;
#endif
    }
    return OK;
}

#ifdef __cplusplus
} // extern C
} // namespace ESZ7_SIGNAL
#endif
