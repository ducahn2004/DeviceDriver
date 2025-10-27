#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 4   // Number of threads

int count = 0;                 // Counter for number of threads that reached the barrier
pthread_mutex_t mutex;         // Mutex to protect the counter
pthread_cond_t cond;           // Condition variable to signal threads

void* thread_func(void* arg) {
    int id = *(int*)arg;

    printf("Thread %d reached the barrier\n", id);

    // Enter critical section
    pthread_mutex_lock(&mutex);
    count++;

    if (count < N) {
        // Not all threads have arrived yet → wait
        pthread_cond_wait(&cond, &mutex);
    } else {
        // Last thread arrived → wake up everyone
        printf("All threads reached the barrier! Releasing them...\n");
        pthread_cond_broadcast(&cond);
    }

    // Exit critical section
    pthread_mutex_unlock(&mutex);

    // After being released
    printf("Thread %d passed the barrier\n", id);

    return NULL;
}

int main() {
    pthread_t threads[N];
    int ids[N];

    // Initialize mutex and condition variable
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create N threads
    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy synchronization primitives
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("All threads finished execution.\n");
    return 0;
}
