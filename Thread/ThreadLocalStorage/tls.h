#ifndef TLS_H_
#define TLS_H_

#include <assert.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MAX_THREADS 128

/* data structures */
typedef struct TLS
{
    pthread_t TID;
    unsigned int size;
    unsigned int page_num;
    struct page **pages;

    /* added */
    struct TLS* next;
    struct TLS* prev;
} TLS;

struct page {
    void* address;
    int ref_cnt;
};


/* 
 *  tls_create creates a local storage area for the executing thread
 *  the created LSA stores atleast 'size' bytes
 *  return 0 if success, -1 when error occured
 *  erro could be if a thread already has a LSA
 * */
int tls_create(unsigned int size)__attribute__((unused));


/* 
 *  writes data to the current thread's local 
 *  the written data is comming from 'buffer'
 *  desination is offset bytes from the start of LSA
 *  returns 0 when success, and return -1 when fail
 *  error when wrting data size is bugger than the size of the LSA
 * */
int tls_write(
        unsigned int offset,
        unsigned int length,
        const char* buffer
        )__attribute__((unused));


/* 
 *  same as tls_write, 
 *  the read data will be stored into buffer
 *  error is the same as tls_write
 * */
int tls_read(
        unsigned int offset,
        unsigned int length,
        char *buffer
        )__attribute__((unused));


/*
 *  tls_destory frees a previously allocated LSA of the current thread
 *  return 0 on sucess, and -1 w
 * */
int tls_destroy()__attribute__((unused));


/*
 *  this function clones local storage area of the target thread
 * */
int tls_clone(pthread_t tid)__attribute__((unused));



//helper funtions

struct TLS* search(pthread_t id)__attribute__((unused));

void tls_init()__attribute__((unused));

int toPage(unsigned int length)__attribute__((unused));

void tls_protect(struct page *p)__attribute__((unused));

void tls_unprotect(struct page *p)__attribute__((unused));

#endif
