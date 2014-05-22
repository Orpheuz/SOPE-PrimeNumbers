all: 
	gcc -Wall primes.c CircularQueue.h -o bin/primes -lm -pthread -lrt
