#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

static int idata = 1000;  // Global variable (stored in data segment)

int main() {
    int istack = 150;     // Local variable (stored in stack segment)
    int fd, status;
    pid_t childPid;

    // Open the file "Hello.txt" for reading and writing.
    // If it does not exist, create it. If it exists, truncate its content.
    fd = open("Hello.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write initial content to the file
    write(fd, "initialfile", 11);
    printf("\n[Parent]File content: initialfile\n");
    // Create a child process
    childPid = fork();
    if (childPid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (childPid == 0) {  // ===== Child process =====
        printf("\n[Child] Initial values: idata = %d, istack = %d\n", idata, istack);

        // Modify variables
        idata += 100;
        istack += 50;

        printf("[Child] After modification: idata = %d, istack = %d\n", idata, istack);

        // Move file offset to the 7th byte from the beginning
        if (lseek(fd, 7, SEEK_SET) == -1) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }

        // Write new data into the file starting from offset 7
        write(fd, "child", 5);

        _exit(0); // Exit immediately without flushing stdio buffers
    }

    // ===== Parent process =====
    // Wait for the child process to finish
    wait(&status);

    printf("\n[Parent] After child process terminates:\n");
    printf("[Parent] idata = %d, istack = %d\n", idata, istack);

    // Move the file offset back to the beginning
    lseek(fd, 0, SEEK_SET);

    // Read the file content and print it
    char buf[64];
    int n = read(fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("[Parent] File content: %s\n", buf);

    close(fd);
    return 0;
}
