#include <stdio.h>
#include <pthread.h>
#include <unistd.h> 
#include <stdlib.h>

void *sum_numbers(void *arg){
    int N = *((int *)arg);
    int sum = 0;
    for(int i = 1; i <= N; i++){
        sum += i;
    }
    int *result = malloc(sizeof(int));
    *result = sum;
    pthread_exit((void *)result);
}

int main(){
    pthread_t thread;
    int N;

    printf("Enter a integer N: ");
    scanf("%d", &N);

    if(pthread_create(&thread, NULL, sum_numbers, &N) != 0){
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }

    int *result;
    pthread_join(thread, (void **)&result);
    printf("Sum of numbers from 1 to %d is %d\n", N, *result);
    free(result);
    return 0;
}