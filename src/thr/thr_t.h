/**
  \file   thr_t.h
  \author Aleksey Lekhtin
  \date   Jan 29, 2025
  \brief  Threads
*/

#ifndef THR_STRUCT_H
#define THR_STRUCT_H

#include <pthread.h>

typedef struct thr_t {
    char    name[32];
    pthread_t thread;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_condattr_t condattr;
    ///// void (*mutex_lock)(void);
    ///// void (*mutex_unlock)(void);
    void *(*func)(void *);
    void *obj;
    int  id;
    int  inited;
    int  cmd_to_stop;
    int  wakeup;
} thr_t;

typedef struct thrs_t {
    int total;
    ///// int registered_to_die;
    thr_t **thrs;
} thrs_t;

#endif // THR_STRUCT_H
