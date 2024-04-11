override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.	# add -Wextra to be added at the end
override LDLIBS := -fsanitize=leak -fsanitize=undefined $(LDLIBS)

tests_files = $(wildcard test_*.c)
tests = $(addprefix ./, $(tests_files:.c=)) # append "./" before each file name

all: run_test

$(tests) : % : %.o disk.o fs.o

.PHONY : run_test clean

disk.o : disk.c disk.h

fs.o : fs.c fs.h

run_test : $(tests)
	/bin/bash runtest.sh $(tests)

clean : 
	rm -rf *.o $(tests) mydisk
