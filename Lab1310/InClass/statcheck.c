#include <stdio.h>
#include <sys/stat.h>  // for stat
#include <sys/types.h> // for struct stat

int main (int argc, char **argv) 
{
    struct stat mystat;
    char *type, *readok;

    if (stat(argv[1], &mystat) < 0){
	perror("stat");
	return(1);
    }
    if (S_ISREG(mystat.st_mode))     /* Determine file type */
	type = "regular";
    else if (S_ISDIR(mystat.st_mode))
	type = "directory";
    else
        type = "other";
    if ((mystat.st_mode & S_IRUSR)) /* Check read access */
	readok = "yes";
    else
        readok = "no";

    printf("type: %s, read: %s\n", type, readok);
    return(0);
}
