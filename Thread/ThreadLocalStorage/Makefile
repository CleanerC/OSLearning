override CFLAGS := -std=gnu99 -pthread $(CFLAGS)
override LDLIBS := -pthread $(LDLIBS)

all: runtest

.PHONY : clean runtest

test_files = ./test_create ./test_write_basic ./test_read_basic ./test_read_write ./test_one_COW ./test_multiple_COW ./test_multi_clone

CC = gcc

tls.o : tls.c

test_create.o : test_create.c
test_create : test_create.o tls.o

test_write_basic.o : test_write_basic.c
test_write_basic : test_write_basic.o tls.o

test_read_basic.o : test_read_basic.c
test_read_basic : test_read_basic.o tls.o

test_read_write.o : test_read_write.c
test_read_write : test_read_write.o tls.o

test_one_COW.o : test_one_COW.c
test_one_COW : test_one_COW.o tls.o

test_multiple_COW.o : test_multiple_COW.c
test_multiple_COW : test_multiple_COW.o tls.o

test_multi_clone.o : test_multi_clone.c
test_multi_clone : test_multi_clone.o tls.o

gen_file : $(test_files)

runtest : gen_file
	/bin/bash runtest.sh $(test_files)

clean : 
	rm -rf *.o $(test_files)
