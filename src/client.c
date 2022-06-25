/* client.c: Message Queue Client */

#include "mq/client.h"
#include "mq/logging.h"
#include "mq/socket.h"
#include "mq/string.h"

/* Internal Constants */

#define SENTINEL "SHUTDOWN"

/* Internal Prototypes */

void * mq_pusher(void *);
void * mq_puller(void *);

/* External Functions */

/**
 * Create Message Queue withs specified name, host, and port.
 * @param   name        Name of client's queue.
 * @param   host        Address of server.
 * @param   port        Port of server.
 * @return  Newly allocated Message Queue structure.
 */
MessageQueue * mq_create(const char *name, const char *host, const char *port) {
    MessageQueue *mq = calloc(1, sizeof(MessageQueue));

    if (mq) {

      // Check that each attributes exist and set values
      if (name)
        snprintf(mq->name, sizeof(mq->name), "%s", name);
      if (host)
        snprintf(mq->host, sizeof(mq->host), "%s", host);
      if (port)
        snprintf(mq->port, sizeof(mq->port), "%s", port);

      // Create queues and initialize shutdown to false
      mq->outgoing = queue_create();
      mq->incoming = queue_create();
      mq->shutdown = false;

      // Initialize lock
      mutex_init(&mq->lock_stop_mq, NULL);

      return mq;
    }

    return NULL;
}

/**
 * Delete Message Queue structure (and internal resources).
 * @param   mq      Message Queue structure.
 */
void mq_delete(MessageQueue *mq) {
    queue_delete(mq->incoming);
    queue_delete(mq->outgoing);
    free(mq);
}

/**
 * Publish one message to topic (by placing new Request in outgoing queue).
 * @param   mq      Message Queue structure.
 * @param   topic   Topic to publish to.
 * @param   body    Message body to publish.
 */
void mq_publish(MessageQueue *mq, const char *topic, const char *body) {
    char publish_uri[BUFSIZ];
    int status = sprintf(publish_uri, "/topic/%s", topic);

    if (status < 0)
      return;

    // Create request and push onto outgoing
    Request *r = request_create("PUT", publish_uri, body);
    queue_push(mq->outgoing, r);
}

/**
 * Retrieve one message (by taking Request from incoming queue).
 * @param   mq      Message Queue structure.
 * @return  Newly allocated message body (must be freed).
 */
char * mq_retrieve(MessageQueue *mq) {
    Request *r = queue_pop(mq->incoming);

    // Check that request attributes exist and
    // the body is not SENTINEL
    if (!r)
      return NULL;
    if (!r->body) {
      request_delete(r);
      return NULL;
    }
    if (streq(r->body, SENTINEL)) {
      request_delete(r);
      return NULL;
    }

    // Set body, delete the request and return
    char *body = strdup(r->body);
    request_delete(r);
    return body;
}

/**
 * Subscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to subscribe to.
 **/
void mq_subscribe(MessageQueue *mq, const char *topic) {
    char subscribe_uri[BUFSIZ];
    int status = sprintf(subscribe_uri, "/subscription/%s/%s", mq->name, topic);

    if (status < 0)
      return;

    // Create request and push onto outgoing
    Request *r = request_create("PUT", subscribe_uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Unubscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to unsubscribe from.
 **/
void mq_unsubscribe(MessageQueue *mq, const char *topic) {
    char unsubscribe_uri[BUFSIZ];
    int status = sprintf(unsubscribe_uri, "/subscription/%s/%s", mq->name, topic);

    if (status < 0)
      return;

    // Create request and push onto outgoing
    Request *r = request_create("DELETE", unsubscribe_uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Start running the background threads:
 *  1. First thread should continuously send requests from outgoing queue.
 *  2. Second thread should continuously receive reqeusts to incoming queue.
 * @param   mq      Message Queue structure.
 */
void mq_start(MessageQueue *mq) {
    // Subscribe to topic = SENTINEL and run threads
    mq_subscribe(mq, SENTINEL);
    thread_create(&mq->pusher, NULL, mq_pusher, (void *)mq);
    thread_create(&mq->puller, NULL, mq_puller, (void *)mq);
}

/**
 * Stop the message queue client by setting shutdown attribute and sending
 * sentinel messages
 * @param   mq      Message Queue structure.
 */
void mq_stop(MessageQueue *mq) {
    // Publish SENTINEL message
    mq_publish(mq, SENTINEL, SENTINEL);

    // Set shutdown and join threads
    mutex_lock(&mq->lock_stop_mq);
    mq->shutdown = true;
    mutex_unlock(&mq->lock_stop_mq);

    thread_join(mq->pusher, NULL);
    thread_join(mq->puller, NULL);
}

/**
 * Returns whether or not the message queue should be shutdown.
 * @param   mq      Message Queue structure.
 */
bool mq_shutdown(MessageQueue *mq) {
    mutex_lock(&mq->lock_stop_mq);
    bool shutdown = mq->shutdown;
    mutex_unlock(&mq->lock_stop_mq);

    return shutdown;
}

/* Internal Functions */

/**
 * Pusher thread takes messages from outgoing queue and sends them to server.
 **/
void * mq_pusher(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;

    // Run until mq->shutdown is set
    while (!mq_shutdown(mq)) {
        FILE *fs = socket_connect(mq->host, mq->port);
        if (!fs) {
          continue;
        }

        // Write request to server
        Request *r = queue_pop(mq->outgoing);
        request_write(r, fs);
        request_delete(r);

        char buffer[BUFSIZ];

        // Read response from server
        if (!fgets(buffer, BUFSIZ, fs)) {
          fclose(fs);
          continue;
        }
        fclose(fs);
    }

    return 0;
}

/**
 * Puller thread requests new messages from server and then puts them in
 * incoming queue.
 **/
void * mq_puller(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;

    // Run until mq->shutdown is set
    while (!mq_shutdown(mq)) {
      char get_uri[BUFSIZ];
      int status = sprintf(get_uri, "/queue/%s", mq->name);

      if (status < 0)
        continue;

      FILE *fs = socket_connect(mq->host, mq->port);
      if (!fs) {
        continue;
      }

      // Write request to server
      Request *r = request_create("GET", get_uri, NULL);
      request_write(r, fs);

      char buffer[BUFSIZ];
      int length;

      // Check for response from server
      if (!fgets(buffer, BUFSIZ, fs)) {
        request_delete(r);
        fclose(fs);
        continue;
      }

      // Check for correct status code and scan for content length
      if (strstr(buffer, "200 OK")) {
        while (fgets(buffer, BUFSIZ, fs) && !streq(buffer, "\r\n")) {
          sscanf(buffer, "Content-Length: %d", &length);
        }

        // Read response from server into r->body and push onto incoming
        char *response = calloc(1, length+1);
        fread(response, 1, length, fs);

        r->body = response;
        queue_push(mq->incoming, r);
      }
      else {
        request_delete(r);
        continue;
      }

      fclose(fs);
    }

    return 0;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
