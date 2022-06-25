# Configuration

CC		= gcc
LD		= gcc
AR		= ar
CFLAGS		= -g -std=gnu99 -Wall -Iinclude -fPIC
LDFLAGS		= -Llib -pthread
ARFLAGS		= rcs

# Variables

CLIENT_HEADERS  = $(wildcard include/mq/*.h)
CLIENT_SOURCES  = $(wildcard src/*.c)
CLIENT_OBJECTS  = $(CLIENT_SOURCES:.c=.o)
CLIENT_LIBRARY  = lib/libmq_client.a

TEST_SOURCES    = $(wildcard tests/test_*.c)
TEST_OBJECTS    = $(TEST_SOURCES:.c=.o)
TEST_PROGRAMS   = $(subst tests,bin,$(basename $(TEST_OBJECTS)))

# Rules

all:	bin/application

%.o:			%.c $(CLIENT_HEADERS)
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(CLIENT_LIBRARY):	$(CLIENT_OBJECTS)
	@echo "Linking   $@"
	@$(AR) $(ARFLAGS) $@ $^

bin/%:  		tests/%.o $(CLIENT_LIBRARY)
	@echo "Linking   $@"
	@$(LD) $(LDFLAGS) -o $@ $^

bin/application:	src/application.o $(CLIENT_LIBRARY)
		@echo "Linking	$@"
		@$(LD) $(LDFLAGS) -o $@ $^

test:			$(TEST_PROGRAMS)
	@$(MAKE) -sk test-all

test-all:   		test-request-unit test-queue-unit test-queue-functional test-echo-client

test-request-unit:	bin/test_request_unit
	@bin/test_request_unit.sh

test-queue-unit:	bin/test_queue_unit
	@bin/test_queue_unit.sh
	
test-queue-functional:	bin/test_queue_functional
	@bin/test_queue_functional.sh
	
test-echo-client:	bin/test_echo_client
	@bin/test_echo_client.sh

clean:
	@echo "Removing  objects"
	@rm -f $(CLIENT_OBJECTS) $(TEST_OBJECTS)

	@echo "Removing  libraries"
	@rm -f $(CLIENT_LIBRARY)
	
	@echo "Removing  test programs"
	@rm -f $(TEST_PROGRAMS)

.PRECIOUS: %.o
