#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int sharedVar = 5; 

int main() {
    pid_t pid = fork();  // Create child process

    if (pid == 0) { // Child process
        sharedVar += 10;
        printf("Child Process: sharedVar = %d\n", sharedVar);
        exit(0);
    } 
    else if (pid > 0) { // Parent process
        wait(NULL);  // Wait for child to finish
        printf("Parent Process: sharedVar = %d\n", sharedVar);
    } 
    else {
        perror("fork failed");
        exit(1);
    }

    return 0;
}
