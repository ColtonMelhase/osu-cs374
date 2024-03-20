// Colton Melhase
// melhasec
// CS374 W2024
// Project 5 - One-Time Pads

// PSEUDO CODE - keygen
// Iterate through given length
    // print (char) of rand() int within ASCII ranges of capital letters and ' '.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char *argv[]) {
    srand(time(NULL));
    if(argc > 2 || argc == 1) {
        // printf("Enter 1 argument for the keylength\n"); fflush(stdout);
        exit(1);
    }
    for(int i = 0; i < atoi(argv[1]); i++) {
        int random = rand() % 27 + 1; // get 1-27
        if(random == 27) {
            printf("%c", (char)32); fflush(stdout); // space char
        } else {
            printf("%c", (char)(random+64)); fflush(stdout); // A-Z
        }
    }
    printf("\n");
}