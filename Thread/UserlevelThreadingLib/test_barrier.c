#include <stdio.h>
#include <ec440threads.h>

#define NUM_THREADS 2

pthread_barrier_t barrier;

void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d: Before barrier\n", thread_id);
    
    // Wait for all threads to reach the barrier
    pthread_barrier_wait(&barrier);
    
    printf("Thread %d: After barrier\n", thread_id);
    
    pthread_exit(NULL);
}

void* print_hello(void* arg) {
    for(int ii = 0; ii < 4; ++ii) {
        printf("hello! %d\n", *(int*)arg);
        pause();
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[2 * NUM_THREADS];
    // int thread_ids[2 * NUM_THREADS];

    pthread_t hellothread;
    // int hello;
    
    // Initialize the barrier
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    
    // Create threads
    pthread_create(&hellothread, NULL, print_hello, &hellothread);
    for (int i = 0; i < 2 * NUM_THREADS; i++) {
        // thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &threads[i]);
    }
    
    // Wait for all threads to finish
    for (int i = 0; i < 2 * NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(hellothread, NULL);
    
    // Destroy the barrier
    pthread_barrier_destroy(&barrier);
    
    return 0;
}