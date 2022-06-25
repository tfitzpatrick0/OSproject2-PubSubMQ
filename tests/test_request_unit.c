/* test_request_unit.c: Test Requests structure (Unit) */

#include "mq/logging.h"
#include "mq/request.h"
#include "mq/string.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>

/* Constants */

Request REQUESTS[] = {
    { "PUT", "/topic/HOT" , "SOME LIKE IT" },
    { "GET", "/queue/LIVE", "FOREVER" },
    { "DELETE", "/subscription/LIVE/FOREVER", NULL },
    { NULL, NULL, NULL },
};

/* Functions */

int test_00_request_create() {
    for (Request *r = REQUESTS; r->method; r++) {
        Request *n = request_create(r->method, r->uri, r->body);
        assert(n);
        assert(streq(n->method, r->method));
        assert(streq(n->uri   , r->uri));

        if (n->body) {
            assert(streq(n->body, r->body));
        } else {
            assert(n->body == r->body);
        }

        free(n->method);
        free(n->uri);
        free(n->body);
        free(n);
    }

    return EXIT_SUCCESS;
}

int test_01_request_delete() {
    for (Request *r = REQUESTS; r->method; r++) {
        Request *n = request_create(r->method, r->uri, r->body);
        assert(n);
        assert(streq(n->method, r->method));
        assert(streq(n->uri   , r->uri));

        if (n->body) {
            assert(streq(n->body, r->body));
        } else {
            assert(n->body == r->body);
        }

        request_delete(n);
    }

    return EXIT_SUCCESS;
}

int test_02_request_write() {
    char tempfile[BUFSIZ] = "test.XXXXXX";
    int status = EXIT_FAILURE;
    int fd = mkstemp(tempfile);

    if (fd < 0) {
        fprintf(stderr, "mkstemp: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    FILE *fs = fdopen(fd, "w+");
    if (!fs) {
        fprintf(stderr, "fdopen: %s\n", strerror(errno));
        goto failure;
    }

    request_write(&REQUESTS[0], fs);
    fseek(fs, 0, SEEK_SET);

    char buffer[BUFSIZ];

    char *target = "PUT /topic/HOT HTTP/1.0\r\n";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }
    
    target = "Content-Length: 12\r\n";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }
    
    target = "\r\n";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }
    
    target = "SOME LIKE IT";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }

    status = EXIT_SUCCESS;

failure:
    unlink(tempfile);
    if (fs) fclose(fs);
    return status;
}

int test_03_request_write() {
    char tempfile[BUFSIZ] = "test.XXXXXX";
    int status = EXIT_FAILURE;
    int fd = mkstemp(tempfile);

    if (fd < 0) {
        fprintf(stderr, "mkstemp: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    FILE *fs = fdopen(fd, "w+");
    if (!fs) {
        fprintf(stderr, "fdopen: %s\n", strerror(errno));
        goto failure;
    }

    request_write(&REQUESTS[2], fs);
    fseek(fs, 0, SEEK_SET);

    char buffer[BUFSIZ];

    char *target = "DELETE /subscription/LIVE/FOREVER HTTP/1.0\r\n";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }
    
    target = "\r\n";
    if (!fgets(buffer, BUFSIZ, fs)) {
        goto failure;
    }
    if (!streq(buffer, target)) {
        fprintf(stderr, "%s != %s\n", buffer, target);
        goto failure;
    }
    
    status = EXIT_SUCCESS;

failure:
    unlink(tempfile);
    if (fs) fclose(fs);
    return status;
}

/* Main execution */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s NUMBER\n\n", argv[0]);
        fprintf(stderr, "Where NUMBER is right of the following:\n");
        fprintf(stderr, "    0. Test request_create\n");
        fprintf(stderr, "    1. Test request_delete\n");
        fprintf(stderr, "    2. Test request_write (w/ body)\n");
        fprintf(stderr, "    3. Test request_write (w/out body)\n");
        return EXIT_FAILURE;
    }

    int number = atoi(argv[1]);
    int status = EXIT_FAILURE;

    switch (number) {
        case 0:  status = test_00_request_create(); break;
        case 1:  status = test_01_request_delete(); break;
        case 2:  status = test_02_request_write(); break;
        case 3:  status = test_03_request_write(); break;
        default: fprintf(stderr, "Unknown NUMBER: %d\n", number); break;
    }   

    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
