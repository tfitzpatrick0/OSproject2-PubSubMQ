/* test_queue_functional.c: Test Concurrent Queue of Requests (Functional) */

#include "mq/logging.h"
#include "mq/thread.h"
#include "mq/queue.h"
#include "mq/string.h"

#include <assert.h>

/* Constants */

const size_t NCONSUMERS = 2;
const size_t NPRODUCERS = 4;
const size_t NMESSAGES  = 1<<10;

/* Threads */

void *consumer(void *arg) {
    Queue *q = (Queue *)arg;
    size_t messages = 0;
    size_t methods  = 0;
    size_t uris     = 0;
    size_t bodies   = 0;

    while (messages < NMESSAGES) {
        Request *r = queue_pop(q);
        assert(r);

        methods += atoi(r->method);
        uris    += atoi(r->uri);
        bodies  += atoi(r->body);

        assert(streq(r->method, "1"));
        assert(streq(r->uri   , "2"));
        assert(streq(r->body  , "3"));

        request_delete(r);
        messages++;
    }

    assert(methods ==   NMESSAGES);
    assert(uris    == 2*NMESSAGES);
    assert(bodies  == 3*NMESSAGES);
    return NULL;
}

void *producer(void *arg) {
    Queue *q = (Queue *)arg;
    char method[BUFSIZ];
    char uri[BUFSIZ];
    char body[BUFSIZ];

    for (size_t m = 0; m < NMESSAGES; m++) {
    	strcpy(method, "1");
    	strcpy(uri   , "2");
    	strcpy(body  , "3");
    	queue_push(q, request_create(method, uri, body));
    }

    return NULL;
}

/* Main execution */

int main(int arg, char *argv[]) {
    Thread consumers[NCONSUMERS];
    Thread producers[NPRODUCERS];
    Queue *q = queue_create();

    for (size_t c = 0; c < NCONSUMERS; c++) {
    	thread_create(&consumers[c], NULL, consumer, q);
    }

    for (size_t p = 0; p < NPRODUCERS; p++) {
    	thread_create(&producers[p], NULL, producer, q);
    }

    for (size_t c = 0; c < NCONSUMERS; c++) {
    	thread_join(consumers[c], NULL);
    }

    for (size_t p = 0; p < NPRODUCERS; p++) {
    	thread_join(producers[p], NULL);
    }

    queue_delete(q);
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
