/**
  \file   thr.cpp
  \author Aleksey Lekhtin
  \date   Jul 26, 2024
  \brief  Threads
*/

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "dbg.h"
#include "thr.h"

static thrs_t m_thrs{};

int thrs(thr_t **thrs[])
{
    *thrs = m_thrs.thrs;
    return m_thrs.total;
}

thr_t *thr_create(const char *name, void *func(void *arg), void *obj)
{
    assert(func != nullptr);
    auto *thr = (thr_t *)calloc(1, sizeof(thr_t));
    thr->id = m_thrs.total+1;
    thr->func = func;
    thr->obj = obj;
    thr->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pthread_condattr_init(&thr->condattr);
#ifdef __linux__
    pthread_condattr_setclock(&thr->condattr, CLOCK_MONOTONIC);
#endif
    pthread_cond_init(&thr->cond, &thr->condattr);
    pthread_attr_init(&thr->attr);

    if (!name || name[0] == 0) {
        strncpy(thr->name, "@", sizeof(thr->name) - 1);
    } else {
        strncpy(thr->name, name, sizeof(thr->name) - 1);
    }

    int err = pthread_create(&thr->thread, &thr->attr, thr->func, (void *)thr);
    if (err) {
        int e = errno;
        pthread_mutex_lock(&thr->mutex);
        DBGF(1, " thread creation error: (%d) %s\n",
             __func__, err, strerror(e));
        pthread_mutex_unlock(&thr->mutex);
        return (thr_t *)(intptr_t)(-e);
    }
    thr->inited = 1;
    thr->wakeup = 0;
    thr->cmd_to_stop = 0;

    auto *mem = realloc(m_thrs.thrs, m_thrs.total+1);
    if (!mem) {
        return (thr_t *)(intptr_t)(-errno);
    }
    m_thrs.thrs = (thr_t **)mem;
    m_thrs.thrs[m_thrs.total] = thr;
    m_thrs.total++;

    if (DBG::debug_level >= 4) {
        pthread_mutex_lock(&thr->mutex);
        thr_t **ths;
        int total = thrs(&ths);
        DBGF(4, " threads created: %d\n", total);
        for (int i = 0; i < total; i++) {
            DBGF(4, " thread[{}]: id:{}  ptr:0x{:08x}\n", i, ths[i]->id, (uintptr_t)ths[i]);
        }
        pthread_mutex_unlock(&thr->mutex);
    }
    return thr;
}

int thr_run(thr_t *thr)
{
    assert(thr != nullptr);
    int err = pthread_detach(thr->thread);
    if (err) {
        int e = errno;
        pthread_mutex_lock(&thr->mutex);
        DBGF(1, " thread detach error: (%d) %s\n", err, strerror(e));
        pthread_mutex_unlock(&thr->mutex);
        thr_destroy(thr);
        return e;
    }
    return 0;
}

void thr_destroy(thr_t *thr)
{
    if (!thr || !thr->inited) {
        return;
    }
    pthread_mutex_lock(&thr->mutex);
    thr->cmd_to_stop = 1;
    thr->wakeup = 1;
    DBGF(4, " thr[%s] status #1 - cmd_to_stop: %d, wakeup: %d \n",
         __func__, thr->name, thr->cmd_to_stop, thr->wakeup);
    DBGF(4, " thr[%s] wakeup: %d, emit pthread_cond_signal()\n", thr->name, thr->wakeup);
    pthread_cond_signal(&thr->cond);
    pthread_mutex_unlock(&thr->mutex);

    const struct timespec ts = (struct timespec) {
            .tv_sec = 0,
            .tv_nsec = 1000000,
    };
    nanosleep(&ts, nullptr);

    pthread_mutex_lock(&thr->mutex);
    DBGF(4, " thr[%s] status #2 - cmd_to_stop: %d, wakeup: %d \n",
         __func__, thr->name, thr->cmd_to_stop, thr->wakeup);
    thr->obj = nullptr;
    thr->func = nullptr;

    pthread_cond_destroy(&thr->cond);
    pthread_condattr_destroy(&thr->condattr);
    pthread_mutex_unlock(&thr->mutex);

    for (int i = 0; i < m_thrs.total; i++) {
        thr_t *t = m_thrs.thrs[i];
        if (t == thr) {
            free(t);
            m_thrs.thrs[i] = nullptr;
        }
    }
}

void thr_destroy_all_later()
{
    ///// m_thrs.registered_to_die = 1;
    for (int i = 0; i < m_thrs.total; i++) {
        thr_t *thr = m_thrs.thrs[i];
        if (thr) {
            thr->cmd_to_stop = 1;
            thr->wakeup = 1;
        }
    }
}

void thr_destroy_all()
{
    for (int i = 0; i < m_thrs.total; i++) {
        thr_t *thr = m_thrs.thrs[i];
        if (thr && thr->func) {
            thr_destroy(thr);
        }
    }

    const struct timespec ts = (struct timespec) {
            .tv_sec = 0,
            .tv_nsec = 1000000,
    };
    nanosleep(&ts, nullptr);

    for (int i = 0; i < m_thrs.total; i++) {
        thr_t *thr = m_thrs.thrs[i];
        if (thr) {
            DBGF(4, " free thr[%s], id: %d\n", thr->name, thr->id);
            free(thr);
        }
    }
    free(m_thrs.thrs);
    m_thrs.thrs = nullptr;
    m_thrs.total = 0;
    DBGF(4, " done. thrs: 0x{:X}, total: {}\n", (uintptr_t)m_thrs.thrs, m_thrs.total);
}

void thr_wakeup(int id)
{
    int total = m_thrs.total;
    if (DBG::debug_level >= 4) {
        for (int i = 0; i < total; i++) {
            thr_t *thr = m_thrs.thrs[i];
            pthread_mutex_lock(&thr->mutex);
            DBGF(4, " thread[{}::{}]: ptr:0x{:08x}, id: {}, cmd_to_stop: {}, wakeup:{}\n",
                 i, thr->name, (uintptr_t)thr, thr->id, thr->cmd_to_stop, thr->wakeup);
            pthread_mutex_unlock(&thr->mutex);
        }
    }

    for (int i = 0; i < total; i++) {
        thr_t *t = m_thrs.thrs[i];
        if (t->id != id) {
            continue;
        }
        pthread_mutex_lock(&t->mutex);
        DBGF(4, " wakeup for thr[%d] name: %s, id: %d, emit pthread_cond_signal()\n", i, t->name, t->id);
        t->wakeup = 1;
        pthread_cond_signal(&t->cond);
        pthread_mutex_unlock(&t->mutex);
        break;
    }
}

///// void thr_wakeup_all()
///// {
/////     for (int i = 0; i < m_thrs.total; i++) {
/////         thr_t *t = m_thrs.thrs[i];
/////         thr_wakeup(t->id);
/////     }
///// }
