#include<stdio.h>
#include<pthread.h>
#include <unistd.h> // For sleep()

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // The "key"
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;   // The "bell"
int ready = 0;                                    // The condition

void *prodfun(void *arg){
printf("Producer: Hello, preparing data...\n");
sleep(1); // Simulate some work 
pthread_mutex_lock(&lock); // Get the lock to modify shared data 
ready = 1;                 // Modify the shared condition
printf("Producer: Ready, signal consumer\n");
pthread_cond_signal(&cond);// Ring the bell to wake up the consumer
pthread_mutex_unlock(&lock);} 

void *consfun(void *arg){
printf("Consumer: Waiting...\n");
while (!ready)        
   pthread_cond_wait(&cond, &lock); // wait() atomically
// Woken, aquired the lock, locked the shared data AUTOMATICALLY
printf("Consumer: Data received.\n");    
pthread_mutex_unlock(&lock);} // Release the lock

void main()
{
pthread_t prod, cons;

pthread_create(&prod, NULL, prodfun, NULL);
pthread_create(&cons, NULL, consfun, NULL);
pthread_join(prod, NULL);
pthread_join(cons, NULL);
pthread_mutex_destroy(&lock);    
pthread_cond_destroy(&cond);                                 
}