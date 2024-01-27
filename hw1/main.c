// Colton Melhase
// CS374 WR2024
// 1/25/2024
// Brewster

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct movie {
    char *title;        // Title
    int year;           // Year of release
    int langCount;      // Number of languages
    char *languages[5]; // Languages
    double rating;      // Rating
    struct movie *next; // Points to the next movie struct
};

struct movie *createMovie(char *currLine) {
    // // //
    // Given a string representing each line in the .csv file, delimits them between ',', and processes
    // the contents by storing them into the newly made movie struct. Then appends the created movie to the
    // end of the linked list.

    // Parameters:
    //     currLine: The string of a whole line from the .csv file.

    // Return:
    //     Pointer to the newly created movie.
    // // //

    // Allocate memory space and initialize struct to default values.
    struct movie *currMovie = malloc(sizeof(struct movie));
    currMovie->title = NULL;
    currMovie->year = 0;
    currMovie->langCount = 0;
    for(int i = 0; i < 5; i++) {
        currMovie->languages[i] = NULL;
    }
    currMovie->rating = 0;
    currMovie->next = NULL;

    char *saveptr;

    // Process title
    char *token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // Process year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = atoi(token);

    // Process languages
    token = strtok_r(NULL, ",", &saveptr);  // Get list of languages 
                                            // ex. '[English;French]'
    char *langSaveptr = NULL;
    char *langToken = NULL;
    char *languages = token;                
    languages++;                            // Strip first character '['
    languages[strlen(languages)-1] = '\0';  // Strip last character ']'
    while(langToken = strtok_r(languages, ";", &langSaveptr)) {
        currMovie->languages[currMovie->langCount] = calloc(strlen(langToken) + 1, sizeof(char));
        strcpy(currMovie->languages[currMovie->langCount], langToken);
        currMovie->langCount++;
        languages = langSaveptr;
    }
    
    // Process rating
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rating = strtod(token, NULL);

    return currMovie;
}

struct movie *processFile(char *filePath) {
    // // //
    // Give a string representing the file path to the .csv file, for every movie entry in the .csv file,
    // create a new movie object and initialize it to the value returned by createMovie which processes
    // the data within each line.

    // Parameters:
    //     filePath: The string representing the file path to the .csv file.

    // Return:
    //     Pointer to the head of the linked list of movies.
    // // //
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;

    struct movie *head = NULL;
    struct movie *tail = NULL;

    int count = 0;

    getline(&currLine, &len, movieFile);    // skip first line
    while((nread = getline(&currLine, &len, movieFile)) != -1) {
        struct movie *newNode = createMovie(currLine);
        count++;

        if(head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    printf("Processed file %s and parsed data for %d movies\n", filePath, count);
    
    free(currLine);
    fclose(movieFile);
    return head;
}


void printMovie(struct movie *aMovie) {
    // // //
    // Prints the given movie's complete details

    // Parameters:
    //     aMovie: Pointer to a movie struct

    // Return:
    //     None
    // // //
    printf("\t%s,%d,[", aMovie->title, aMovie->year);
    for(int i = 0; i < aMovie->langCount; i++) {
        printf("%s;", aMovie->languages[i]);
    }

    printf("],%0.2f\n", aMovie->rating);
}

void printMovieList(struct movie *list) {
    // // //
    // Given the head of a movie linked list, iterates down linked list calling printMovie for each node

    // Parameters:
    //     list: head pointer of a linked list of movies

    // Return:
    //     None
    // // //
    while(list != NULL) {
        printMovie(list);
        list = list->next;  // set list to the next movie 
    }
}

void freeMovie(struct movie *aMovie) {
    // // //
    // Frees the memory allocated by the given movie.

    // Parameters:
    //     aMovie: Pointer to a movie struct

    // Return:
    //     None
    // // //
    free(aMovie->title);
    for(int i = 0; i < aMovie->langCount; i++) {
        free(aMovie->languages[i]);
    }
    free(aMovie);
}
void freeMovieList(struct movie *list) {
    // // //
    // Given the head of a movie linked list, iterates down linked list freeing all
    // the memory allocated by each movie by calling freeMovie on each node.

    // Parameters:
    //     list: head pointer of a linked list of movies

    // Return:
    //     None
    // // //
    while(list != NULL) {
        struct movie *delMovie = list;
        list = list->next;
        freeMovie(delMovie);
    }
}

void displayMenu() {
    // // //
    // Displays the menu prompt at the beginning of program run and after
    // each choice.

    // Parameters:
    //     None

    // Return:
    //     None
    // // //
    printf("\n1. Show movies released in the specified year\n"
            "2. Show highest rated movie for each year\n"
            "3. Show the title and year of release of all movies in a specific language\n"
            "4. Exit from the program\n\n"
            "Enter a choice from 1 to 4: ");
}

void displaySpecificYear(struct movie *list, int year) {
    // // //
    // Given the head of a movie linked list and the year, loops through
    // linked list and print each movie that has the matching year of release.

    // Parameters:
    //     list: head pointer of a linked list of movies
    //     year: the year of release

    // Return:
    //     None
    // // //
    bool noResults = true;

    while(list != NULL) {
        
        if(list->year == year) {
            noResults = false;
            printf("%s\n", list->title);
        }
        list = list->next;
    }
    if(noResults) {
                printf("No data about movies released in the year %d\n", year);
    }
}

bool isInArray(int arr[], int size, int val) {
    // // //
    // Given an array of integers, the size of the array, and the value to look for,
    // return true if the integer is already in the array, and false if it is not.

    // Parameters:
    //     arr: The array of numbers
    //     size: The size of the array
    //     val: The value to look for

    // Return:
    //     boolean reporting if val is in arr or not.
    // // //
    for(int i = 0; i < size; i++) {
        if(arr[i] == val) {
            return true;
        }
    }
    return false;
}

void displayHighestRatedInYear(struct movie *list) {
    // // //
    // Given the head of the linked list of movies, print out
    // each movie that is highest rated in that year.

    // Parameters:
    //     list: head pointer of a linked list of movies

    // Return:
    //     None
    // // //
    struct movie *head = list;
    struct movie *highestRated = NULL;
    int yearsProcessed[121] = {};       // Array to store processed years

    while(head != NULL) {
        while(isInArray(yearsProcessed, 121, head->year)) { // If the movie's year is already processed,
            head = head->next;                              // skip
            if(head == NULL) {
                return;
            }
        }
        yearsProcessed[head->year % 121] = head->year;      // Store the year into yearsProcessed
        highestRated = head;
        list = head->next;
        while(list != NULL) {                               // Find the highest rated movie
            if(highestRated->year == list->year) {
                if(highestRated->rating < list->rating) {
                    highestRated = list;
                }
            }
            list = list->next;
        }
        
        printf("%d %0.1f %s\n", highestRated->year, highestRated->rating, highestRated->title);

        head = head->next;
    }
}

void displaySpecificLanguage(struct movie *list, char *language) {
    // // //
    // Given the head of the linked list of movies, print out
    // each movie that has the given language

    // Parameters:
    //     list: head pointer of a linked list of movies
    //     language: the language to print movies about

    // Return:
    //     None
    // // //
    bool noResults = true;

    while(list != NULL) {
        for(int i = 0; i < list->langCount; i++) {
            if(strcmp(list->languages[i], language) == 0) {
                printf("%d %s\n", list->year, list->title);
                noResults = false;
            }
        }
        list = list->next;
    }
    if(noResults) {
        printf("No data about movies released in %s\n", language);
    }
}


int main(int argc, char *argv[]) {
    struct movie *list = processFile(argv[1]);
    int choice = -1;
    do {
        displayMenu();
        choice = -1;
        scanf("%d", &choice);

        switch(choice) {
            int specificYear = -1;
            char *specificLanguage = NULL;
            case 1: printf("Enter the year for which you want to see movies: ");
                    scanf("%d", &specificYear);
                    displaySpecificYear(list, specificYear); 
                    break;
            case 2: displayHighestRatedInYear(list);
                    break;
            case 3: printf("Enter the language for which you want to see movies: ");
                    scanf("%s", specificLanguage);
                    displaySpecificLanguage(list, specificLanguage);
                    break;
            case 4: printf("Quitting program\n");
                    freeMovieList(list);
                    exit(0);
                    break;
            default: printf("You entered an incorrect choice. Try again.\n");
                     break;
        }
    } while (choice != 4);
}