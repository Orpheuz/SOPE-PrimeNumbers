#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "CircularQueue.h"

#define queueSize 10

unsigned long N; //maximum number
sem_t canTerminate;
pthread_mutex_t primeListMut;
QueueElem* primeNumbers;
unsigned long primeNumbersSize;
unsigned long arrayIndex;

void showPrimes() {

	int i;
	for(i = 0; i < primeNumbersSize; i++) {
		if(i == primeNumbersSize - 1) {
			printf("%lu. \n", primeNumbers[i]);
		}
		else {
			printf("%lu, \n", primeNumbers[i]);
		}
	}
}


void addPrimesToList(QueueElem prime) {
	
	pthread_mutex_lock(&primeListMut);
	
	primeNumbers[arrayIndex] = prime;
	arrayIndex += 1;
	primeNumbersSize+=1;
	
	pthread_mutex_unlock(&primeListMut);
	
}

void *filterThreadFunc(void *arg) {
	pthread_mutex_lock(&primeListMut);
	CircularQueue* q = arg;
	QueueElem elem = queue_get(q);
	QueueElem first = elem;
	if(first  > (int) sqrt(N)){
		addPrimesToList(first);
		do
		{
			elem = queue_get(q);
			addPrimesToList(elem);
		} while(elem != 0);
		sem_post(&canTerminate);
	}
	else {
		pthread_t filterThreadId;
		CircularQueue* temp;
		queue_init(&temp, queueSize);
		if(pthread_create(&filterThreadId, NULL, filterThreadFunc, (void*) temp)) {
			printf("Thread error.");
		}
		do {
			elem = queue_get(q);
			if(elem % first != 0)
				queue_put(temp, elem);
		} while (elem != 0);
		queue_put(temp, 0);
		addPrimesToList(first);
	}

	pthread_mutex_unlock(&primeListMut);
	queue_destroy(q);
	return NULL;
}

void *initThreadfunc(void *arg) {
	
	pthread_t filterThreadId;
	CircularQueue *q;
	queue_init(&q, queueSize);
	addPrimesToList(2);
	
	if(N == 2) {
		sem_post(&canTerminate);
	}
	else {
		if(pthread_create(&filterThreadId, NULL, filterThreadFunc, (void*) q)) {
			printf("Thread error.");
		}
		int i;
		for(i = 3; i < N; i+=2) {
			queue_put(q, i);
			printf("FODASSE");
		}
		queue_put(q, 0);
	}	
	return NULL;
}
	
int compare(const void * a, const void * b)
{
		return ( *(QueueElem*)a - *(QueueElem*)b );
}

int main(int argc, char *argv[]) {
	
	pthread_t initThreadId;
	arrayIndex = 0;
	N = strtol(argv[1], NULL, 10);
	if(N < 2) {
		sem_post(&canTerminate);
	}
	primeNumbersSize = 1.2*(N/log(N))*sizeof(QueueElem);
	primeNumbers = (QueueElem*) malloc (sizeof(QueueElem) * 1.2 * N / log(N));
	if(sem_init(&canTerminate, 0, 0) != 0) {
		return -1;
	}
	if(pthread_mutex_init(&primeListMut, 0) != 0) {
		return -1;
	}
	
	if(pthread_create(&initThreadId, NULL, initThreadfunc, NULL)) {
		printf("Error creating thread");
	}
	printf("FODASSE1");
	sem_wait(&canTerminate);
	
	qsort(primeNumbers, primeNumbersSize, sizeof(QueueElem), compare);
	
	showPrimes(primeNumbers);
	
	return 0;
	
}
