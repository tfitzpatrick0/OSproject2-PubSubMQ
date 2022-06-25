/* echo_client.c: Message Queue Echo Client test */

#include "mq/client.h"

#include <assert.h>
#include <time.h>
#include <unistd.h>

/* Constants */

const char * TOPIC     = "testing";
const size_t NMESSAGES = 10;

/* Threads */

void *incoming_thread(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;
    size_t messages = 0;

    while (!mq_shutdown(mq)) {
    	char *message = mq_retrieve(mq);
	if (message) {
	    assert(strstr(message, "Hello from"));
	    free(message);
	    messages++;
	}
    }

    assert(messages == NMESSAGES);
    return NULL;
}

void *outgoing_thread(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;
    char body[BUFSIZ];

    for (size_t i = 0; i < NMESSAGES; i++) {
    	sprintf(body, "%lu. Hello from %lu\n", i, time(NULL));
    	mq_publish(mq, TOPIC, body);
    }

    sleep(5);
    mq_stop(mq);
    return NULL;
}

/* Main execution */

int main(int argc, char *argv[]) {
    /* Parse command-line arguments */
    char *name = getenv("USER");
    char *host = "localhost";
    char *port = "9620";

    if (argc > 1) { host = argv[1]; }
    if (argc > 2) { port = argv[2]; }
    if (!name)    { name = "echo_client_test";  }

    /* Create and start message queue */
    MessageQueue *mq = mq_create(name, host, port);
    assert(mq);

    mq_subscribe(mq, TOPIC);
    mq_unsubscribe(mq, TOPIC);
    mq_subscribe(mq, TOPIC);
    mq_start(mq);

    /* Run and wait for incoming and outgoing threads */
    Thread incoming;
    Thread outgoing;
    thread_create(&incoming, NULL, incoming_thread, mq);
    thread_create(&outgoing, NULL, outgoing_thread, mq);
    thread_join(incoming, NULL);
    thread_join(outgoing, NULL);

    mq_delete(mq);
    return 0;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */ 
