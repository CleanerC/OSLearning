#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 10

pthread_barrier_t barrier;

void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d: Before barrier\n", thread_id);
    
    // Wait for all threads to reach the barrier
    pthread_barrier_wait(&barrier);
    
    printf("Thread %d: After barrier\n", thread_id);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // Initialize the barrier
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Destroy the barrier
    pthread_barrier_destroy(&barrier);
    
    return 0;
}