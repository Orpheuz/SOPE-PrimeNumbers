#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include "CircularQueue.h"
#define queueSize 10

unsigned long N; //maximum number
sem_t canTerminate;
pthread_mutex_t primeListMut;
QueueElem* primeNumbers;
unsigned long primeNumbersSize;
unsigned long arrayIndex;
//------------------------------------------------------------------------------------------
//Shows all prime numbers that the primeNumbers list has. Makes a verification. A random 
//  number between 0 and the size of the list is chosen. That number represents the index in 
//  primes list of the number that is going to be verified. Then if the quocient of
//  that prime number with all numbers between 2 and that number(excludind itself) equals
//  0, that number isn't prime. 
void showPrimes() {
	unsigned long i,randPrime;
	int verification=0;
	QueueElem rPrime;
	randPrime = rand() % (arrayIndex);
	printf("\nThe prime numbers from 2 to %lu are:\n", N);
	for(i = 0; i < arrayIndex; i++) {
		if(i==randPrime)
			rPrime=primeNumbers[i];
		if(i == arrayIndex - 1) {
			printf("%lu. \n", primeNumbers[i]);
		}
		else {
			printf("%lu, ", primeNumbers[i]);
		}
	}
	for(i = 2; i < rPrime; i++) {
		if(rPrime % i==0)
		{
			printf("\nVerification complete: %lu, isn't a prime number!!!\n", rPrime);
			verification=1;
			break;
		}
	}
	if(verification==0)
			printf("\nVerification complete: %lu, is a prime number!!!\n", rPrime);
}
//------------------------------------------------------------------------------------------
// Includes in primeNumbers list the QueueElem that is been passed as a parameter. And updates
//   all variables needed.
void addPrimesToList(QueueElem prime) {
	pthread_mutex_lock(&primeListMut);
	primeNumbers[arrayIndex] = prime;
	arrayIndex += 1;
	primeNumbersSize+=1;
	pthread_mutex_unlock(&primeListMut);
}
//------------------------------------------------------------------------------------------
//Function that all threads(excluding the initThread) will use. This fuction checks if the 
//   front value of the circular queue, that as been passed as a parameter, is greater than
//   sqrt(N). If so, all values of the circular list are prime and are put in primeNumbers list.
//   Otherwise, all values of the circular queue will be analised, and all numbers that are multiple
//   of the first number of the queue will not be included in a new circular queue that will be passed
//	 to the new thread that will be created.

void *filterThreadFunc(void *arg) {
	CircularQueue* q = arg;
	QueueElem elem = queue_get(q);
	QueueElem first = elem;
	addPrimesToList(first);
	if(first  > (int) sqrt(N)){
		elem = queue_get(q);
		while(elem != 0) {
			
			addPrimesToList(elem);
			elem = queue_get(q);
		}
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
	}
	queue_destroy(q);
	return NULL;
}
//------------------------------------------------------------------------------------------
// Function that only initThread use. This thread puts number 2 in the primeNumbers list. If N
// 	 (parameter thats passed by user) equals 2, this thread can terminate and no other thread is 
//   created. If not, a new thread is created and a new circular queue(that will be passed as a
//   parameter) is initialized with all numbers(starting in 3), that aren't multiple of 3. 
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
		for(i = 3; i <= N; i+=2) {
			queue_put(q, i);
		}
		queue_put(q, 0);
	}	
	return NULL;
}
//------------------------------------------------------------------------------------------
// Auxiliar function that is used in qsort, that compares which is the greater number between
//    to given numbers.
int compare(const void * a, const void * b)
{
		return ( *(QueueElem*)a - *(QueueElem*)b );
}
//------------------------------------------------------------------------------------------
// A verification of the input parameter is done. If N is lower than 2 or isn't an integer
//   program will end with a error message. Global variables are also initialized. InitiThread 
//   is created. At the end of the, primeNumbers list is sorted and then all prime numbers are
//   shown.
int main(int argc, char *argv[]) {
	pthread_t initThreadId;
	srand (time(NULL));
	arrayIndex = 0;
	N = strtol(argv[1], NULL, 10);
	if(N < 2) {
		printf("Wrong input. Input must be an integer(2 or higher).\n");
		return 0;																																																																																																																	
	}
	primeNumbersSize = ceil(1.2*(N/log(N))) +1;
	primeNumbers = (QueueElem*) malloc (sizeof(QueueElem) * primeNumbersSize);
	if(sem_init(&canTerminate, 0, 0) != 0) {
		return -1;
	}
	if(pthread_mutex_init(&primeListMut, 0) != 0) {
		return -1;
	}
	
	if(pthread_create(&initThreadId, NULL, initThreadfunc, NULL)) {
		printf("Error creating thread");
	}
	sem_wait(&canTerminate);
	qsort(primeNumbers, arrayIndex, sizeof(QueueElem), compare);
	showPrimes(primeNumbers);
	return 0;
}
