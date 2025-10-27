#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5        // Size of shared buffer
int buffer[BUFFER_SIZE];     // Shared buffer
int count = 0;               // Number of items in the buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    int i = 0;
    while (1) {
        sleep(1);  // simulate production time
        pthread_mutex_lock(&mutex);

        // Wait while buffer is full
        while (count == BUFFER_SIZE) {
            printf("Buffer full! Producer waiting...\n");
            pthread_cond_wait(&cond_full, &mutex);
        }

        // Produce item
        buffer[count] = i;
        printf("Producer produced: %d (buffer count: %d)\n", buffer[count], count + 1);
        count++;
        i++;

        // Signal that buffer is not empty
        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *consumer(void *arg) {
    while (1) {
        sleep(2);  // simulate consumption time
        pthread_mutex_lock(&mutex);

        // Wait while buffer is empty
        while (count == 0) {
            printf("Buffer empty! Consumer waiting...\n");
            pthread_cond_wait(&cond_empty, &mutex);
        }

        // Consume item
        count--;
        int item = buffer[count];
        printf("Consumer consumed: %d (buffer count: %d)\n", item, count);

        // Signal that buffer is not full
        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    // Create threads
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // Join threads (never returns in this example)
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    return 0;
}
