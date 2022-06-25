/* request.c: Request structure */

#include "mq/request.h"

#include <stdlib.h>
#include <string.h>

/**
 * Create Request structure.
 * @param   method      Request method string.
 * @param   uri         Request uri string.
 * @param   body        Request body string.
 * @return  Newly allocated Request structure.
 */
Request * request_create(const char *method, const char *uri, const char *body) {
    Request *r = (Request*) calloc(1, sizeof(Request));

    if (!r) {
        free(r);
        return NULL;
    }

    // Set values
    if (method) {
        r->method = strdup(method);
    }
    if (uri) {
        r->uri = strdup(uri);
    }
    if (body) {
        r->body = strdup(body);
    }

    return r;
}

/**
 * Delete Request structure.
 * @param   r           Request structure.
 */
void request_delete(Request *r) {
    if (r) {

        if (r->method)
          free(r->method);
        if (r->uri)
          free(r->uri);
        if (r->body)
          free(r->body);

        free(r);
    }
}

/**
 * Write HTTP Request to stream:
 *
 *  $METHOD $URI HTTP/1.0\r\n
 *  Content-Length: Length($BODY)\r\n
 *  \r\n
 *  $BODY
 *
 * @param   r           Request structure.
 * @param   fs          Socket file stream.
 */
void request_write(Request *r, FILE *fs) {
    if (r->method != NULL && r->uri != NULL) {
        fprintf(fs, "%s %s HTTP/1.0\r\n", r->method, r->uri);

        if (r->body != NULL) {
            fprintf(fs, "Content-Length: %zu\r\n", strlen(r->body));
            fprintf(fs, "\r\n");
            fprintf(fs, "%s", r->body);
        }
        else {
            fprintf(fs, "\r\n");
        }
    }
}
