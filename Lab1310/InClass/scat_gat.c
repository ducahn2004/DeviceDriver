#include <stdio.h>   // For printf()
#include <string.h>  // For strlen()
#include <sys/uio.h> // For writev and struct iovec
#include <unistd.h>  // For open(), close()
#include <fcntl.h>   // For O_WRONLY, O_CREAT

int main() {
    int fd = open("output.txt", O_RDWR|O_CREAT, 0664);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Our scattered data buffers
    char *header = "HEADER_DATA\n";
    char *payload = "This is the main content of the message.\n";
    char *footer = "FOOTER_DATA\n";

    // Create an array of iovec structures
    struct iovec iov[3];
    ssize_t bytes_written;

    // First vector: the header
    iov[0].iov_base = header;
    iov[0].iov_len = strlen(header);

    // Second vector: the payload
    iov[1].iov_base = payload;
    iov[1].iov_len = strlen(payload);

    // Third vector: the footer
    iov[2].iov_base = footer;
    iov[2].iov_len = strlen(footer);

    // Write all three buffers in a single system call
    bytes_written = writev(fd, iov, 3);

    if (bytes_written == -1) {
        perror("writev");
    } else {
        printf("Successfully wrote %zd bytes.\n", bytes_written);
    }

    close(fd);
    return 0;
}
