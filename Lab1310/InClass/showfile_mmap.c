#include <fcntl.h>     // for open
#include <unistd.h>    // for write, close
#include <sys/mman.h>  // for mmap, munmap
#include <sys/stat.h>  // for fstat
#include <sys/types.h> // for struct stat
#include <stdio.h>     // for perror

int main(int argc, char **argv)
{
    struct stat mystat;

    // should handle argc!=2 error here
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return(1);
    }
    if (fstat(fd, &mystat) < 0) {
        perror("fstat"); // should also close fd
        return(1);
    }
    size_t size = mystat.st_size;
    char *buf = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf == MAP_FAILED) {
        perror("mmap"); // should also close fd
        return(1);
    }
    write(1, buf, size);
    munmap(buf, size);  // Unmap memory
    close(fd);		// Close file
    return 0;
}
