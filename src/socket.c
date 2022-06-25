/* socket.c: Socket functions */

#include "mq/logging.h"
#include "mq/socket.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Create socket connection to specified host and port.
 * @param   host    Host string to connect to.
 * @param   port    Port string to connect to.
 * @return  Socket file stream of connection if successful, otherwise NULL.
 */
FILE *  socket_connect(const char *host, const char *port) {
    /* Lookup server address information */
    struct addrinfo *results;
    struct addrinfo  hints = {
	     .ai_family   = AF_UNSPEC,   /* Return IPv4 and IPv6 choices */
  	   .ai_socktype = SOCK_STREAM, /* Use TCP */
    };
    int status;
    if ((status = getaddrinfo(host, port, &hints, &results)) != 0) {
        error("Unable to resolve %s:%s: %s", host, port, gai_strerror(status));
        return NULL;
    }

    /* For each server entry, allocate socket and try to connect */
    int socket_fd = -1;
    for (struct addrinfo *p = results; p != NULL && socket_fd < 0; p = p->ai_next) {
        /* Allocate socket */
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            error("Unable to make socket: %s", strerror(errno));
            continue;
        }

        /* Connect to host */
        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(socket_fd);
            socket_fd = -1;
            continue;
        }
    }

    /* Release allocate address information */
    freeaddrinfo(results);

    if (socket_fd < 0) {
        error("Unable to connect to %s:%s: %s", host, port, strerror(errno));
        return NULL;
    }

    /* Make file stream */
    FILE *fs = fdopen(socket_fd, "r+");
    if (!fs) {
        error("Unable to make file stream: %s", strerror(errno));
        close(socket_fd);
    }
    return fs;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
