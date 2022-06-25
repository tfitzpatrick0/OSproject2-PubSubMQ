/* application.c: Concurrent Chat Application */

#include "mq/thread.h"
#include "mq/client.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>

/* Globals */
char *PROGRAM = NULL;

/* Constants */
#define BACKSPACE   127

/* Functions
 * https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
 */
void toggle_raw_mode() {
    static struct termios OriginalTermios = {0};
    static bool enabled = false;

    if (enabled) {
    	 tcsetattr(STDIN_FILENO, TCSAFLUSH, &OriginalTermios);
    } else {
	     tcgetattr(STDIN_FILENO, &OriginalTermios);

	     atexit(toggle_raw_mode);

	     struct termios raw = OriginalTermios;
	     raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
	     raw.c_cc[VMIN] = 0;
	     raw.c_cc[VTIME] = 1;

	     tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    	 enabled = true;
    }
}

void usage(int status) {
    printf("Usage: %s <options>\n\n", PROGRAM);
    printf("Options:\n");
    printf("    -p PORTNUMBER\n");
    printf("    -n USERNAME\n");
    printf("    -h\n\n");
    exit(status);
}

void show_options() {
    printf("\nCommand options:\n");
    printf("    sub TOPIC\n");
    printf("    unsub TOPIC\n");
    printf("    publish TOPIC MESSAGE\n");
    printf("    options\n");
    printf("    quit\n\n");
}

/* Threads */

// Run background thread to retrieve and print messages from the server
void *background_thread(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;

    // Run until mq->shutdown is set
    while (!mq_shutdown(mq)) {
        char *body = mq_retrieve(mq);

        // Print message
        if (body) {
            char name[BUFSIZ], topic[BUFSIZ], message[BUFSIZ];
            int scan_status = sscanf(body, "%s %s %[^\t\n]", name, topic, message);

            if (scan_status != 3) {
                printf("\n%s\n", body);
            }
            else {
                printf("\r%-80s", ""); // Erase line
                printf("\r%s> %s: %s\n", name, topic, message);
            }

            free(body);
        }
    }

    return 0;
}

/* Main Execution */

int main(int argc, char *argv[]) {

    // Initialize default arguments
    PROGRAM = argv[0];
    char *name = getenv("USER");
    char *host = "localhost";
    char *port = "9456";

    // Parse command line arguments
    int argindex = 1;
    while (argindex < argc) {
      char *arg = argv[argindex];

      if (strlen(arg) > 1 && arg[0] == '-') {

          // argument: -h --> help
          if (arg[1] == 'h') {
            usage(0);
            break;
          }

          // argument: -p --> port number
          if (arg[1] == 'p') {
            if (argindex+1 < argc) {
              port = argv[++argindex];
            }
            else {
              usage(1);
            }
            break;
          }

          // argument: -n --> name
          if (arg[1] == 'n') {
            if (argindex+1 < argc) {
              name = argv[++argindex];
            }
            else {
              usage(1);
            }
            break;
          }
      }

      ++argindex;
    }

    // Start message queue
    MessageQueue *mq = mq_create(name, host, port);
    mq_start(mq);

    // Display command options
    show_options();
    printf("\n");
    sleep(3);

    toggle_raw_mode();

    // Background thread
    Thread background;
    thread_create(&background, NULL, background_thread, (void *)mq);

    // Foreground thread
    char   input_buffer[BUFSIZ] = "";
    size_t input_index = 0;

    // Receive input commands until the program terminates
    while (true) {
	     char input_char = 0;
    	 read(STDIN_FILENO, &input_char, 1);

       // Wait for newline input to process command
       if (input_char == '\n') {

          if (strcmp(input_buffer, "quit") == 0) {
            break;
          }

          // Unsubscribe mq from TOPIC
          else if (strstr(input_buffer, "unsub")) {
            char topic[BUFSIZ];
            int scan_status = sscanf(input_buffer, "unsub %s", topic);

            // Check sscanf
            if (scan_status != 1) {
              printf("\r%s> Invalid command %s\n", name, input_buffer);
              goto endline;
            }

            printf("\r%s> Unsubscribed from %s", name, topic);
            mq_unsubscribe(mq, topic);
          }

          // Subscribe mq to TOPIC
          else if (strstr(input_buffer, "sub")) {
            char topic[BUFSIZ];
            int scan_status = sscanf(input_buffer, "sub %s", topic);

            // Check sscanf
            if (scan_status != 1) {
              printf("\r%s> Invalid command %s\n", name, input_buffer);
              goto endline;
            }

            printf("\r%s> Subscribed to %s\n", name, topic);
            mq_subscribe(mq, topic);
          }

          // Publish a MESSAGE to TOPIC
          else if (strstr(input_buffer, "publish")) {
            char topic[BUFSIZ], body[BUFSIZ];
            int scan_status = sscanf(input_buffer, "publish %s %[^\n]", topic, body);

            // Check sscanf
            if (scan_status != 2) {
              printf("\r%s> Invalid command %s\n", name, input_buffer);
              goto endline;
            }

            char message[BUFSIZ];
            int sp_status = sprintf(message, "%s %s %s", name, topic, body);

            // Check sprintf
            if (sp_status < 0) {
              printf("\r%s> Unable to create message\n", name);
              goto endline;
            }

            mq_publish(mq, topic, message);
          }

          // Show command options
          else if (strstr(input_buffer, "options")) {
            show_options();
            sleep(3);
            goto endline;
          }

          // Invalid command
          else {
            printf("\r%s> Command: %s not recognized\n", name, input_buffer);
          }

          // Reset input index/buffer
          input_index = 0;
          input_buffer[0] = 0;
       }

       // Update input buffer in real time
       else if (input_char == BACKSPACE && input_index) {
          input_buffer[--input_index] = 0;
       }

       else if (!iscntrl(input_char) && input_index < BUFSIZ) {
          input_buffer[input_index++] = input_char;
          input_buffer[input_index] = 0;
       }

// Reset prompt
endline:
       if (input_char == '\n') {
          input_index = 0;
          input_buffer[0] = 0;
       }

       printf("\r%-80s", "");			// Erase line (hack!)
     	 printf("\r%s", input_buffer);		// Write
     	 fflush(stdout);
    }

    // Stop and delete mq
    printf("\n");
    mq_stop(mq);
    mq_delete(mq);

    return 0;
}
