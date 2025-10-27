#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int n = 0;

void sigint_handler(int sig) {
    printf("Handler catches signal %d\n", sig);
    n = 0; 
}

void main() {
    signal(SIGINT, sigint_handler);
    for (int i = 0; i < 10; i++) {
        printf("Main count = %d\n", n++);
        sleep(1);
    }
}
