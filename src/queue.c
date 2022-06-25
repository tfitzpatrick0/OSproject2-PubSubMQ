/* queue.c: Concurrent Queue of Requests */

#include "mq/queue.h"

/**
 * Create queue structure.
 * @return  Newly allocated queue structure.
 */
Queue * queue_create() {
    Queue *q = (Queue *) calloc(1, sizeof(Queue));

    // Set initial size to 0 and initialize primitives
    q->size = 0;
    mutex_init(&q->lock, NULL);
    cond_init(&q->produced, NULL);

    return q;
}

/**
 * Delete queue structure.
 * @param   q       Queue structure.
 */
void queue_delete(Queue *q) {
    while (q->size > 0) {
      Request *r = queue_pop(q);
      request_delete(r);
    }

    free(q);
}

/**
 * Push request to the back of queue.
 * @param   q       Queue structure.
 * @param   r       Request structure.
 */
void queue_push(Queue *q, Request *r) {
    // Acquire the lock
    mutex_lock(&q->lock);

    // Empty queue
    if (q->head == NULL) {
        q->head = r;
        q->tail = r;
    }
    // Non-empty queue
    else {
        q->tail->next = r;
        q->tail = r;
    }

    r->next = NULL;
    ++q->size;

    // Signal that a value has been pushed and release the lock
    cond_signal(&q->produced);
    mutex_unlock(&q->lock);
}

/**
 * Pop request to the front of queue (block until there is something to return).
 * @param   q       Queue structure.
 * @return  Request structure.
 */
Request * queue_pop(Queue *q) {
    // Acquire the lock
    mutex_lock(&q->lock);

    // Wait until there is something in the queue
    while (q->size == 0) {
        cond_wait(&q->produced, &q->lock);
    }

    // Update the head pointer and decrement size
    Request *r = q->head;
    q->head = q->head->next;
    --q->size;

    // Release the lock
    mutex_unlock(&q->lock);
    return r;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
