#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sharedVar = 5;

void *threadFunc(void *arg) {
    sharedVar += 10;   // Modify global variable
    printf("Thread: sharedVar = %d\n", sharedVar);
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;

    // Create thread
    if (pthread_create(&thread, NULL, threadFunc, NULL) != 0) {
        perror("pthread_create failed");
        exit(1);
    }

    // Wait for thread to finish
    pthread_join(thread, NULL);

    printf("Main: sharedVar = %d\n", sharedVar);
    return 0;
}
