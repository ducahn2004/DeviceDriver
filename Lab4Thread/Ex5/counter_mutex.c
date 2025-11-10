#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int counter = 0;                       // Global shared counter
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for synchronization

void *increment_thread(void *arg) {
    for (int i = 0; i < 100; i++) {
        pthread_mutex_lock(&mutex);    // Lock before modifying shared variable
        counter++;
        pthread_mutex_unlock(&mutex);  // Unlock after modification
    }
    pthread_exit(NULL);
}

void *decrement_thread(void *arg) {
    for (int i = 0; i < 50; i++) {
        pthread_mutex_lock(&mutex);
        counter--;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t incThread, decThread;

    // Create two threads
    if (pthread_create(&incThread, NULL, increment_thread, NULL) != 0) {
        perror("pthread_create (increment) failed");
        exit(1);
    }

    if (pthread_create(&decThread, NULL, decrement_thread, NULL) != 0) {
        perror("pthread_create (decrement) failed");
        exit(1);
    }

    // Wait for both threads to finish
    pthread_join(incThread, NULL);
    pthread_join(decThread, NULL);

    printf("Final counter value: %d\n", counter);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}
