#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    char filename[256];
    struct stat fileStat;

    printf("Enter file name: ");
    scanf("%s", filename);

    // Lấy thông tin của file bằng stat()
    if (stat(filename, &fileStat) < 0) {
        perror("Error reading file info");
        exit(EXIT_FAILURE);
    }

    // In ra kích thước file (đơn vị byte)
    printf("\nFile: %s\n", filename);
    printf("Size: %ld bytes\n", fileStat.st_size);

    // In ra thông tin quyền truy cập
    printf("Access permissions: ");

    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf("\n");

    return 0;
}
