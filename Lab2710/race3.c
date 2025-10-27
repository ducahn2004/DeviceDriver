#include<stdio.h>
#include<pthread.h>

int glob = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *threadfun(void *arg)
{
int loc,j,loops = *((int *) arg);

for (j = 0; j < loops; j++) {
	pthread_mutex_lock(&mtx);
	loc = glob; 
	loc++;
	glob = loc;
	pthread_mutex_unlock(&mtx);}
}

int main(void)
{
pthread_t t1, t2;
int loops=10000;

pthread_create(&t1, NULL, threadfun, &loops);
pthread_create(&t2, NULL, threadfun, &loops);
pthread_join(t1, NULL);
pthread_join(t2, NULL);

printf("glob = %d\n", glob);                                  
}

