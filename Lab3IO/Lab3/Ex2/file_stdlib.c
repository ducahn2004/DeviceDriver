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

    printf("Enter name of file to create: ");
    scanf("%s", targetFile);
    getchar();

    FILE *fp = fopen(targetFile, "w");
    if (!fp) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }

    printf("Choose option:\n1. Enter contents from keyboard\n2. Copy from another file\n");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        printf("Enter text (Ctrl+D to finish):\n");
        while (fgets(buffer, sizeof(buffer), stdin)) {
            fputs(buffer, fp);
        }
    } 
    else if (choice == 2) {
        printf("Enter source filename: ");
        scanf("%s", sourceFile);

        FILE *src = fopen(sourceFile, "r");
        if (!src) {
            perror("Error opening source file");
            fclose(fp);
            exit(EXIT_FAILURE);
        }

        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytesRead, fp);
        }

        fclose(src);
    } 
    else {
        printf("Invalid option.\n");
    }

    fclose(fp);
    printf("File operation completed.\n");
    return 0;
}