// Colton Melhase
// CS374 WR2024
// 1/31/2024
// Brewster

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PREFIX "movies_"
#define SUFFIX ".csv"

bool processLargest() {
// // //
    // Finds the largest .csv file and processes it

    // Parameters:
    //     None

    // Return:
    //     None
    // // //

    DIR* currDir = opendir(".");
    struct dirent *aFile;
    struct stat fileStat;
    int largest = -1;
    char* largestFile;

    while((aFile = readdir(currDir)) != NULL) {
        char* extension = strrchr(aFile->d_name, '.'); // gets file extension
        if(strncmp(PREFIX, aFile->d_name, strlen(PREFIX)) == 0 && !strcmp(SUFFIX, extension)) {
            stat(aFile->d_name, &fileStat);
            if(largest < fileStat.st_size) {
                largest = fileStat.st_size;
                largestFile = strcpy(largestFile, "./");
                largestFile = strcat(largestFile, aFile->d_name);
            }
        }
    }
    printf("%s\n", largestFile);
    
    return true;
}

bool processSmallest() {
// // //
    // Finds the smallest .csv file and processes it

    // Parameters:
    //     None

    // Return:
    //     None
    // // //
}

bool processSpecific() {
// // //
    // Checks if the given file exists, and processes it.
    // Else, write an error message

    // Parameters:
    //     None

    // Return:
    //     None
    // // //
}


void displayMainMenu() {
    printf("\n1. Select file to process\n"
            "2. Exit the program\n\n"

            "Enter a choice 1 or 2: ");
}

void displayFileMenu() {
    printf("\nWhich file you want to process?\n"
            "Enter 1 to pick the largest file\n"
            "Enter 2 to pick the smallest file\n"
            "Enter 3 to specify the name of a file\n\n"

            "Enter a choice from 1 to 3: ");
}

int main() {
    int choiceMain = -1;
    int choiceFile = -1;
    bool fileProcessed = false;
    do {
        displayMainMenu();
        // choiceMain = -1;
        scanf("%d", &choiceMain);

        switch(choiceMain) {
            case 1:
                
                do {
                    displayFileMenu();
                    
                    // choiceFile = -1;
                    scanf("%d", &choiceFile);

                    switch(choiceFile) {
                        
                        case 1:
                            printf("\n\tChoice 1\n");
                            fileProcessed = processLargest();
                            break;
                        case 2:
                            printf("\n\tChoice 2\n");
                            fileProcessed = true;
                            break;
                        case 3:
                            printf("\n\tChoice 3\n");
                            fileProcessed = true;
                            break;
                        default:
                            printf("\n\tThe file null was not found. Try again\n");
                            break;
                    }
                } while (!fileProcessed);
                break;
            case 2:
                printf("\nQuitting...\n");
                exit(0);
                break;
        }
    } while (choiceMain != 2);
}
