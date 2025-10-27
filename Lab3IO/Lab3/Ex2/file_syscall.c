#include "sys/types.h" // Thư viện khai báo các kiểu dữ liệu chuẩn (pid_t, off_t, ...)
#include "sys/stat.h" // Thư viện khai báo các hằng số và cấu trúc dữ liệu liên quan đến trạng thái tập tin
#include "fcntl.h" // Thư viện cho open(), 0_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, ...
#include "unistd.h" // Thư viện CHO lseek(), read(), write(), close(), ...
#include <stdio.h>   // Thư viện chuẩn nhập xuất printf(), perror(), ...
#include <stdlib.h> // Thư viện cho malloc(), exit(), ...
#include <string.h> // Thư viện cho strlen(), strcpy(), ...


int main() {
    int choice;
    char targetFile[100], sourceFile[100], buffer[1024];
    ssize_t bytesRead, bytesWritten;

    printf("Enter name of file to create: ");
    scanf("%s", targetFile);
    getchar(); // consume newline

    int fd = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }

    printf("Choose option:\n1. Enter contents from keyboard\n2. Copy from another file\n");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        printf("Enter text (Ctrl+D to finish):\n");
        while (fgets(buffer, sizeof(buffer), stdin)) {
            write(fd, buffer, strlen(buffer));
        }
    } 
    else if (choice == 2) {
        printf("Enter source filename: ");
        scanf("%s", sourceFile);

        int src = open(sourceFile, O_RDONLY);
        if (src < 0) {
            perror("Error opening source file");
            close(fd);
            exit(EXIT_FAILURE);
        }

        while ((bytesRead = read(src, buffer, sizeof(buffer))) > 0) {
            bytesWritten = write(fd, buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                perror("Error writing to file");
                break;
            }
        }

        close(src);
    } 
    else {
        printf("Invalid option.\n");
    }

    close(fd);
    printf("File operation completed.\n");
    return 0;
}