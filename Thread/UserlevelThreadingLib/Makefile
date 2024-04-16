override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.

# you can uncomment following to add sanitizer for development; note make sure to not have this for submissions
#override CFLAGS :=  -fsanitize=undefined $(CFLAGS) 
#override LDFLAGS := -fsanitize=undefined -fsanitize=leak $(LDLAGS)  

# Add any additional tests here
test_files= $(wildcard test_*.c)
tests = $(addprefix ./, $(test_files:.c=))

all: threads.o

# You might find it useful to write test programs and get them working against the installed pthreads
# to do so, set TSTMYPTHREADS to a value other than 1
TSTMYPTHREADS= 0
ifeq ($(TSTMYPTHREADS),1)
  mythread=threads.o
else
  mythread=
  override LDFLAGS += -pthread
endif

CC = gcc

#build unit test.o
#test_scheduler_init.o : test_scheduler_init.c

# Build the threads.o file
threads.o: threads.c ec440threads.h

.PHONY: clean check checkprogs unit

# tests
$(tests) : ./test_% : test_%.c threads.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(mythread)

# Build all of the test programs
checkprogs: $(tests)


# Run the test programs
test: checkprogs
	/bin/bash run_tests.sh $(tests)

clean:
	rm -f *.o $(tests)
