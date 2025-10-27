#include <stdio.h>   // For perror(), printf()
#include <unistd.h>  // For open(), close()
#include <fcntl.h>   // For O_WRONLY, O_CREAT

int main() {
int retval;
//int fd = open("greetings.txt", O_RDWR|O_CREAT);
int fd = open("greetings.txt", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if ((retval = close(fd)) < 0) {
        perror("close");
        return 1;
    }
    return 0;
}
