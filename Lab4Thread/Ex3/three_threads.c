#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message(void *arg) {
    char *message = (char *)arg;
    pthread_t tid = pthread_self();  // get thread ID
    printf("%s (Thread ID: %lu)\n", message, tid);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[3];
    char *messages[] = {
        "Thread 1: Hello from thread 1!",
        "Thread 2: Greetings from thread 2!",
        "Thread 3: Hi from thread 3!"
    };

    printf("Main thread is running.\n");

    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, print_message, (void *)messages[i]) != 0) {
            perror("pthread_create failed");
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads finished. Main thread exiting.\n");
    return 0;
}
