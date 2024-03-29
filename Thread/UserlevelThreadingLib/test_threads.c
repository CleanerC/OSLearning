#include <stdio.h>
#include <stdlib.h>
#include "ec440threads.h"

#define THREAD_CNT 3

// waste some time
void *func(void *arg) {
    printf("thread %lu: arg = %lu\n", (unsigned long)pthread_self(), (unsigned long)arg);
    unsigned long i;
    for(i = 0; i < 2; i++) {
        printf("thread %lu: print\n", (unsigned long)pthread_self());
        //pause();
        /* comment pause when compiling with pthread library linked */
    }
    return 0;
}

int main(int argc, char **argv) {
    pthread_t threads[THREAD_CNT];
    unsigned long i;

    //create THREAD_CNT threads
    for(i = 0; i < THREAD_CNT; i++) {
        pthread_create(&threads[i], NULL, func, (void *)(i + 100));
        printf("thread %lu created\n", (unsigned long)(threads[i]));
    }

    for(i = 0; i < 3; i++) {
        printf("thread main: print\n");
    }

    pthread_exit(NULL);
}
