/* thread.h: Thread macros and type definitions */

#ifndef THREAD_H
#define THREAD_H

#include "mq/logging.h"

#include <pthread.h>

/* Macros */

#define PTHREAD_CHECK(f) \
    do { \
        int _rc; \
        if ((_rc = (f)) != 0) { \
            error("%s", strerror(_rc)); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

/* Thread */

typedef pthread_t                   Thread;
#define thread_create(t, a, f, v)   PTHREAD_CHECK(pthread_create(t, a, f, v))
#define thread_join(t, r)           PTHREAD_CHECK(pthread_join(t, r))
#define thread_detach(t)            PTHREAD_CHECK(pthread_detach(t))

/* Mutex */

typedef pthread_mutex_t		          Mutex;
#define mutex_init(l, a)            PTHREAD_CHECK(pthread_mutex_init(l, a))
#define mutex_lock(l)               PTHREAD_CHECK(pthread_mutex_lock(l))
#define mutex_unlock(l)             PTHREAD_CHECK(pthread_mutex_unlock(l))

/* Condition Variables */

typedef pthread_cond_t              Cond;
#define cond_init(c, a)             PTHREAD_CHECK(pthread_cond_init(c, a))
#define cond_wait(c, l)             PTHREAD_CHECK(pthread_cond_wait(c, l))
#define cond_signal(c)              PTHREAD_CHECK(pthread_cond_signal(c))

#endif

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
