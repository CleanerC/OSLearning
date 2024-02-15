override CFLAGS := -std=gnu99 -O0 -Wall -Werror -g  -fsanitize=undefined $(CFLAGS) -I.
override LDFLAGS := -fsanitize=undefined -fsanitize=leak $(LDLAGS)  
CC = gcc

# I generally make the first rule run all the tests
all: check

# rule for making the parser.o and helper.o
myshell_parser.o: myshell_parser.c myshell_parser.h
shell_helper.o: shell_helper.c shell_helper.h myshell_parser.c myshell_parser.h

# rule for making the test.o
test_parser.o: test_parser.c myshell_parser.h
test_execute_wrapper.o: test_execute_wrapper.c myshell_parser.h shell_helper.h
test_everything.o: test_everything.c myshell_parser.h shell_helper.h
myshell.o: myshell.c myshell_parser.h shell_helper.h

# each of the test programs executables
test_parser : test_parser.o myshell_parser.o
test_execute_wrapper : test_execute_wrapper.o myshell_parser.o shell_helper.o
test_everything: test_everything.o myshell_parser.o shell_helper.o
myshell : myshell.o myshell_parser.o shell_helper.o

test_files= ./test_parser ./test_execute_wrapper ./test_everything
myshell= ./myshell

txt_files= foo1.txt foo2.txt foo3.txt

.PHONY: clean check checkprogs all 

# Build all of the test program
checkprogs: $(test_files)

check: checkprogs
	/bin/bash run_tests.sh $(test_files)

clean:
	rm -f *~ *.o $(test_files) $(test_o_files) $(myshell) $(txt_files)

build: $(myshell)
