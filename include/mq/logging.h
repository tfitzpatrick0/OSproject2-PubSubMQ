/* logging.h: Logging macros */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#ifndef NDEBUG
#define debug(M, ...) \
    fprintf(stderr, "[%09lu] DEBUG %s:%d:%s: " M "\n", pthread_self(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#define info(M, ...) \
    fprintf(stderr, "[%09lu] INFO  " M "\n", pthread_self(), ##__VA_ARGS__)

#define error(M, ...) \
    fprintf(stderr, "[%09lu] ERROR " M "\n", pthread_self(), ##__VA_ARGS__)

#endif

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
