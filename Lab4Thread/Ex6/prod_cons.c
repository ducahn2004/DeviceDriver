#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;

// random sleep between min and max seconds
void random_sleep(int min, int max) {
    int t = rand() % (max - min + 1) + min;
    sleep(t);
}

void* producer(void* arg) {
    int item = 0;
    while (1) {
        item++; // generate next item

        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            printf("Buffer is full, producer is waiting...\n");
            pthread_cond_wait(&cond_full, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Producer produced: %d (count = %d)\n", item, count);

        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);

        // random delay to simulate different speeds
        random_sleep(0, 3);
    }
    return NULL;
}

void* consumer(void* arg) {
    int item;
    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            printf("Buffer is empty, consumer is waiting...\n");
            pthread_cond_wait(&cond_empty, &mutex);
        }

        item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumer consumed: %d (count = %d)\n", item, count);

        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);

        // random delay to simulate different speeds
        random_sleep(0, 3);
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t prod_thread, cons_thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_full, NULL);
    pthread_cond_init(&cond_empty, NULL);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_full);
    pthread_cond_destroy(&cond_empty);

    return 0;
}
