// Colton Melhase
// CS374 WR2024
// 1/31/2024
// Brewster

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PREFIX "movies_"
#define SUFFIX ".csv"

void processFile(char *filePath) {
    // // //
    // Give a string representing the file path to the .csv file, for every movie entry in the .csv file,
    // create a new movie object and initialize it to the value returned by createMovie which processes
    // the data within each line.

    // Parameters:
    //     filePath: The string representing the file path to the .csv file.

    // Return:
    //     Pointer to the head of the linked list of movies.
    // // //

    printf("Now processing the chosen file named %s\n", filePath);

    // Create directory
    int randNum = random() % 99999;
    char dirName[14+sizeof(randNum) + 1];
    sprintf(dirName, "%s%d", "melhasec.movies.", randNum);
    mkdir(dirName, 0750);

    printf("Created directory with name %s\n", dirName);

    // Process file and file(s)
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *saveptr;

    char* title = NULL;
    int year = 0;

    int fd;

    getline(&currLine, &len, movieFile);    // skip first line
    while((nread = getline(&currLine, &len, movieFile)) != -1) {

        // Get title
        char *token = strtok_r(currLine, ",", &saveptr);
        title = calloc(strlen(token) + 2, sizeof(char)); // +2 for \n and \0
        strcpy(title, token);
        strcat(title, "\n");

        // Get year
        token = strtok_r(NULL, ",", &saveptr);
        year = atoi(token);

        char newFile[strlen(dirName) + 1 + sizeof(randNum) + 5];
        sprintf(newFile, "%s/%d.txt", dirName, year);
        fd = open(newFile, O_RDWR | O_CREAT | O_APPEND, 0640);
        // printf("%s\n", newFile);

        int howMany = write(fd, title, strlen(title));

        close(fd);
    }
    fclose(movieFile);

    // printf("Processed file %s and parsed data for %d movies\n", filePath, count);
    
    // free(currLine);
    // fclose(movieFile);
}

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

                // free any memory allocated from the last file path,
                // and allocate more for the new file path.
                free(largestFile);
                largestFile = calloc(2 + strlen(aFile->d_name) + 1, sizeof(char));
                //construct new file path
                largestFile = strcpy(largestFile, aFile->d_name);
                // largestFile = strcat(largestFile, aFile->d_name);
            }
        }
    }
    //printf("%s\n", largestFile);

    processFile(largestFile);
    
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

    DIR* currDir = opendir(".");
    struct dirent *aFile;
    struct stat fileStat;
    int smallest = INT_MAX;
    char* smallestFile;

    while((aFile = readdir(currDir)) != NULL) {
        char* extension = strrchr(aFile->d_name, '.'); // gets file extension
        if(strncmp(PREFIX, aFile->d_name, strlen(PREFIX)) == 0 && !strcmp(SUFFIX, extension)) {
            stat(aFile->d_name, &fileStat);
            if(smallest > fileStat.st_size) {
                smallest = fileStat.st_size;

                //construct file path
                smallestFile = strcpy(smallestFile, "./");
                smallestFile = strcat(smallestFile, aFile->d_name);
            }
        }
    }
    printf("%s\n", smallestFile);
    
    return true;
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
                    
                    choiceFile = -1;
                    scanf("%d", &choiceFile);

                    switch(choiceFile) {
                        
                        case 1:
                            //printf("\n\tChoice 1\n");
                            fileProcessed = processLargest();
                            break;
                        case 2:
                            printf("\n\tChoice 2\n");
                            fileProcessed = processSmallest();
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
