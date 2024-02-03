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

int processFile(char *filePath) {
    // // //
    // Given a string representing the file path to the .csv file, for every movie entry in the .csv file,
    // read the title and year published. Then create (or append if already exists) a file year.txt, and append
    // the title on a new line within the .txt file. txt files are stored in newly created directory with a format
    // of onid.movies.random.

    // Parameters:
    //     filePath: The string representing the file path to the .csv file.

    // Return:
    //     Int representing error code if the file to be read exists. Returns -5 if file can't be opened.
    // // //

    int fd;
    // Ensure file exists, if not, return -5
    fd = open(filePath, O_RDONLY);
    if(fd == -1) {
        return -5;
    }
    close(fd);

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
        
        // construct filePath of file to be created.
        // And open, creating if it doesn't exist, or appending if exists.
        char newFile[strlen(dirName) + 1 + sizeof(randNum) + 5];
        sprintf(newFile, "%s/%d.txt", dirName, year);
        fd = open(newFile, O_RDWR | O_CREAT | O_APPEND, 0640);

        // Appends movie title to opened .txt file.
        int fileWrite = write(fd, title, strlen(title));

        close(fd);
        free(title);
    }
    
    fclose(movieFile);

    return 0;
}

bool processLargest() {
    // // //
    // Finds the largest .csv file and processes it

    // Parameters:
    //     None

    // Return:
    //     boolean representing if the file was successfully processed
    // // //

    int status = 0;

    // open current directory
    DIR* currDir = opendir(".");
    struct dirent *aFile;
    struct stat fileStat;
    int largest = -1;
    char* largestFile = NULL;

    while((aFile = readdir(currDir)) != NULL) {
        char* extension = strrchr(aFile->d_name, '.'); // gets file extension
        // if file matches prefix 'movies_' and suffix '.csv'
        if(strncmp(PREFIX, aFile->d_name, strlen(PREFIX)) == 0 && strcmp(SUFFIX, extension) == 0 && extension != NULL) {
            stat(aFile->d_name, &fileStat); // get meta data
            if(largest < fileStat.st_size) {
                largest = fileStat.st_size;

                // free any memory allocated from the last file path,
                // and allocate more for the new file path.
                free(largestFile);
                largestFile = calloc(2 + strlen(aFile->d_name) + 1, sizeof(char));
                //construct new file path
                largestFile = strcpy(largestFile, aFile->d_name);
            }
        }
    }

    processFile(largestFile);

    if(status == -5) { // file failed to open or process
        printf("The file %s was not found. Try again\n", largestFile);
        free(largestFile);
        closedir(currDir);
        return false;
    }
    free(largestFile);
    closedir(currDir);
    return true;
}

bool processSmallest() {
    // // //
    // Finds the smallest .csv file and processes it

    // Parameters:
    //     None

    // Return:
    //     boolean representing if the file was successfully processed
    // // //

    int status = 0;

    // open current directory
    DIR* currDir = opendir(".");
    struct dirent *aFile;
    struct stat fileStat;
    int smallest = INT_MAX;
    char* smallestFile = NULL;

    // iterate through directory entries
    while((aFile = readdir(currDir)) != NULL) {
        char* extension = strrchr(aFile->d_name, '.'); // gets file extension

        // if file matches prefix 'movies_' and suffix '.csv'
        if(strncmp(PREFIX, aFile->d_name, strlen(PREFIX)) == 0 && !strcmp(SUFFIX, extension)) {
            stat(aFile->d_name, &fileStat); // get meta data
            if(smallest > fileStat.st_size) {
                smallest = fileStat.st_size;

                // free any memory allocated from the last file path,
                // and allocate more for the new file path.
                free(smallestFile);
                smallestFile = calloc(2 + strlen(aFile->d_name) + 1, sizeof(char));
                //construct new file path
                smallestFile = strcpy(smallestFile, aFile->d_name);
            }
        }
    }

    processFile(smallestFile);

    if(status == -5) { // file failed to open or process
        printf("The file %s was not found. Try again\n", smallestFile);
        free(smallestFile);
        closedir(currDir);
        return false;
    }
    free(smallestFile);
    closedir(currDir);
    return true;
}

bool processSpecific(char* specificFile) {
    // // //
    // Checks if the given file exists, and processes it.
    // processFile() contains the checking if it exists.
    // processFile() returns -5 if it does not exist and/or can't open.

    // Parameters:
    //     None

    // Return:
    //     None
    // // //

    int status = processFile(specificFile);

    if(status == -5) {
        printf("The file %s was not found. Try again\n", specificFile);
        return false;
    }
    return true;
}


void displayMainMenu() {
    // // //
    // Displays top level menu choices.

    // Parameters:
    //     None

    // Return:
    //     None
    // // //
    printf("\n1. Select file to process\n"
            "2. Exit the program\n\n"

            "Enter a choice 1 or 2: ");
}

void displayFileMenu() {
    // // //
    // Displays Select File level menu choices.

    // Parameters:
    //     None

    // Return:
    //     None
    // // //
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
    char specificFile[256]; // 255 max file path length
    do {
        displayMainMenu();
        // choiceMain = -1;
        scanf("%d", &choiceMain);

        switch(choiceMain) {
            case 1:
                
                do {
                    displayFileMenu();
                    
                    //choiceFile = -1;
                    scanf("%d", &choiceFile);
                    
                    switch(choiceFile) {
                        
                        case 1:
                            //printf("\n\tChoice 1\n");
                            fileProcessed = processLargest();
                            break;
                        case 2:
                            //printf("\n\tChoice 2\n");
                            fileProcessed = processSmallest();
                            break;
                        case 3:
                            //printf("\n\tChoice 3\n");
                            printf("Enter the complete file name: ");
                            scanf("%s", &specificFile);
                            fileProcessed = processSpecific(specificFile);
                            break;
                        default: 
                            printf("You entered an incorrect choice. Try again.\n");
                            break;
                    }
                } while (!fileProcessed);
                break;
            case 2:
                printf("\nQuitting...\n");
                exit(0);
                break;
            default: 
                printf("You entered an incorrect choice. Try again.\n");
                break;
        }
    } while (choiceMain != 2);
}
