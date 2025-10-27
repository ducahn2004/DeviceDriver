#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child exiting...\n");
        exit(0);
    } else {
        printf("Parent sleeping, not waiting for child...\n");
        sleep(10); // Child becomes zombie here
    }
    return 0;
}
