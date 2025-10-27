#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [string] [filename]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *message = argv[1];
    const char *filename = argv[2];

    // Mở file để ghi (tạo mới hoặc ghi đè nếu đã tồn tại)
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Ghi chuỗi vào file
    ssize_t bytesWritten = write(fd, message, strlen(message));
    if (bytesWritten < 0) {
        perror("Error writing to file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    printf("Wrote '%s' to %s\n", message, filename);
    return 0;
}
