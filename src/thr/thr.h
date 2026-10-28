/**
  \file   thr.h
  \author Aleksey Lekhtin
  \date   Jul 26, 2024
  \brief  Threads
*/
#ifndef THR_H
#define THR_H

#include "thr_t.h"

#ifndef OK
#define OK      true
#endif

#ifndef FAIL
#define FAIL    false
#endif

/////typedef enum THR_ERR {
/////    THR_OK = 0,
/////    THR_ALREADY = 1,
/////} THR_ERR;

thr_t *thr_create(const char *name, void *func(void *arg), void *obj);
int  thr_run(thr_t *thr);
void thr_destroy(thr_t *thr);
void thr_destroy_all();
void thr_destroy_all_later();
void thr_wakeup(int id);
///// void thr_wakeup_all();

#endif // THR_H
