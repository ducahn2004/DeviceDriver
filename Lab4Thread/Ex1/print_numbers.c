#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void *print_numbers(void *arg){
    int N = *((int *)arg);
    for(int i = 1; i <= N; i++){
        printf("%d\n", i);
    }
    pthread_exit(NULL);
}
int main(){
    pthread_t thread;
    int N;
    
    printf("Enter a integer N: ");
    scanf("%d", &N);

    if(pthread_create(&thread,NULL,print_numbers,&N) != 0){
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_join(thread,NULL);
    return 0;
}