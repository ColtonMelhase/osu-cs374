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
// start off, incrementing myCount. It will then signal the consumer that it is now its turn to
// increment, then the consumer will increment and signal to the producer to increment. This will
// repeat until myCount == 10, and the program will end.

void* consumer(void *args) {

}

int main() {
    
}
