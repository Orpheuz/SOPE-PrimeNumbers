all: CircularQueue PrimeNumberGenerator
	mkdir -p bin
	gcc CircularQueue.c -o -pthread bin/CircularQueue -Wall
	gcc PrimeNumberGenerator.c -o bin/PrimeNumberGenerator -Wall
