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
// start off incrementing myCount. It will then signal the consumer that it is now the consumer's turn to
// increment, then the consumer will increment and signal to the producer to increment. This will
// repeat until myCount == 10, and the program will end.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define COUNT_MAX 10

// Global var to store the count
int myCount = 0;

// Global var to toggle which thread shall increment.
// 1 == producer increments, 0 == consumer increments
int toggleCounter = 1;

// Initialize the mutex
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the condition variables
pthread_cond_t prodsTurn = PTHREAD_COND_INITIALIZER;
pthread_cond_t consTurn = PTHREAD_COND_INITIALIZER;

void* consumer(void *args) {
    int value = 0;
    while(value < COUNT_MAX-1) {
        
        pthread_mutex_lock(&myMutex); // Lock myMutex
        printf("CONSUMER: myMutex locked\n"); fflush(stdout);

        while(toggleCounter) { // if 1, wait for consTurn signal
            printf("CONSUMER: waiting on consTurn\n"); fflush(stdout);
            pthread_cond_wait(&consTurn, &myMutex);
        }
        
        myCount++; // increment counter
        value = myCount; // update scoped value to check myCount in top while()
        toggleCounter = 1; // toggle who increments
        printf("CONSUMER: myCount: %d -> %d\n", myCount - 1, myCount); fflush(stdout);

        pthread_mutex_unlock(&myMutex); // Relieve the mutex
        printf("CONSUMER: myMutex unlocked\n"); fflush(stdout);
        pthread_cond_signal(&prodsTurn); // Send prodsTurn
        printf("CONSUMER: signaling prodsTurn\n"); fflush(stdout);
    }
}

int main() {
    // Set up program
    printf("PROGRAM START\n"); fflush(stdout);

    // main() is already thread 1
    // Create thread 2
    pthread_t cons_tid;
    pthread_create(&cons_tid, NULL, consumer, NULL);
    printf("CONSUMER THREAD CREATED\n"); fflush(stdout);

    // Start Producer
    int value = 0;
    while(value < COUNT_MAX-1) {

        pthread_mutex_lock(&myMutex); // Lock myMutex
        printf("PRODUCER: myMutex locked\n"); fflush(stdout);

        while(!toggleCounter) { // if 0, wait for prodsTurn signal
            printf("PRODUCER: waiting on prodsTurn\n"); fflush(stdout);
            pthread_cond_wait(&prodsTurn, &myMutex);
        }

        myCount++; // increment counter
        value = myCount; // update scoped value to check myCount in top while()
        toggleCounter = 0; // toggle who increments
        printf("PRODUCER: myCount: %d -> %d\n", myCount - 1, myCount); fflush(stdout);

        pthread_mutex_unlock(&myMutex); // Relieve the mutex
        printf("PRODUCER: myMutex unlocked\n"); fflush(stdout);
        pthread_cond_signal(&consTurn); // Send consTurn
        printf("PRODUCER: signaling consTurn\n"); fflush(stdout);
    }

    // Wait/check for consumer thread to terminate
    pthread_join(cons_tid, NULL);
    // Print program end message
    printf("PROGRAM END\n"); fflush(stdout);
    return 0;
}
