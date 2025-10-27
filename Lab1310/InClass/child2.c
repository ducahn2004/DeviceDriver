#include <stdio.h>    /* for printf() */
#include <unistd.h>   /* for fork()  */
#include <sys/wait.h> /* for wait() */

void main()
{
int status, child_pid=fork();
if (child_pid){
   waitpid(child_pid, &status, 0);
   printf("I am the parent\n");
}
else   
   printf("I am the child\n");
}
