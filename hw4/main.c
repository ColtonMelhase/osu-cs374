// Colton Melhase
// CS374 WR2024
// 2/28/2024
// Assignment 4: Multi-threaded Producer Consumer Counter

// Write a C program called "myCounter" that will introduce the use of threads, mutual
// exclusion, and condition variables.

// Using 2 threads, 1 mutex, and 2 condition variables to control and protect the counting
// of a number. The number must count to 10. Implementation of how the myCount variable
// reaches 10 is up to the student.

// My implementation will make each thread take turns in incrementing myCount. The producer will
// start off, incrementing myCount. It will then signal the consumer that it is now the consumer's turn to
// increment, then the consumer will increment and signal to the producer to increment. This will
// repeat until myCount == 10, and the program will end.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Global var to store the count
int myCount = 0

// Initialize the mutex
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the condition variables
pthread_cond_t prodsTurn = PTHREAD_COND_INITIALIZER;
pthread_cond_t consTurn = PTHREAD_COND_INITIALIZER;

void* consumer(void *args) {

}

int main() {
    // Set up program
    printf("PROGRAM START\n"); fflush(stdout);

    // main() is already thread 1
    // Create thread 2
    pthread_t con_tid;
    pthread_create(&con_tid, NULL, consumer, NULL);
    printf("CONSUMER THREAD CREATED\n"); fflush(stdout);
}
