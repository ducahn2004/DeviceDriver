#include <stdio.h>  
#include <unistd.h> 
int main(){
    pid_t pid = fork();
    if(pid == 0){
        printf("I am the child\n");
    }
    else{
        printf("I am the parent\n");
    }
    return 0;
}