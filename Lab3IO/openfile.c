#include "sys/types.h" // Thư viện khai báo các kiểu dữ liệu chuẩn (pid_t, off_t, ...)
#include "sys/stat.h" // Thư viện khai báo các hằng số và cấu trúc dữ liệu liên quan đến trạng thái tập tin
#include "fcntl.h" // Thư viện cho open(), 0_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, ...
#include "unistd.h" // Thư viện CHO lseek(), read(), write(), close(), ...
#include <stdio.h>   // Thư viện chuẩn nhập xuất printf(), perror(), ...
#include <stdlib.h> // Thư viện cho malloc(), exit(), ...
#include <string.h> // Thư viện cho strlen(), strcpy(), ...

int main(int argc, char *argv[]){
    int fd = open("hello.txt", O_RDONLY);
    if(fd == -1){
        write(2, "Cannot open file hello.txt\n", 28);
    }
    char buffer[100];
    int byteReads = read(fd, buffer, sizeof(buffer)-1);
    if(byteReads > 0){
        buffer[byteReads] = '\0';
        write(1, buffer, byteReads);
    }
    close(fd);
    return 0;
}