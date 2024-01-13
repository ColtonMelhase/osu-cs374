
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct movie {
    char *title;
    int year;
    int langCount;
    char *languages[5];
    double rating;
    struct movie *next;
};

struct movie *createMovie(char *currLine) {
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

    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;

    struct movie *head = NULL;
    struct movie *tail = NULL;

    getline(&currLine, &len, movieFile);
    while((nread = getline(&currLine, &len, movieFile)) != -1) {
        struct movie *newNode = createMovie(currLine);

        if(head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    free(currLine);
    fclose(movieFile);
    return head;
}

void printMovie(struct movie *aMovie) {

    printf("\t%s,%d,[", aMovie->title, aMovie->year);
    for(int i = 0; i < aMovie->langCount; i++) {
        printf("%s;", aMovie->languages[i]);
    }

    printf("],%0.2f\n", aMovie->rating);
}

void printMovieList(struct movie *list) {
    while(list != NULL) {
        printMovie(list);
        list = list->next;  // set list to the next movie 
    }
}

void freeMovie(struct movie *aMovie) {
    free(aMovie->title);
    for(int i = 0; i < aMovie->langCount; i++) {
        free(aMovie->languages[i]);
    }
    free(aMovie);
}
void freeMovieList(struct movie *list) {
    while(list != NULL) {
        struct movie *delMovie = list;
        list = list->next;
        freeMovie(delMovie);
    }
}

void displayMenu() {
    printf("\n1. Show movies released in the specified year\n"
            "2. Show highest rated movie for each year\n"
            "3. Show the title and year of release of all movies in a specific language\n"
            "4. Exit from the program\n\n"
            "Enter a choice from 1 to 4: ");
}

void displaySpecificYear(struct movie *list, int year) {
    bool noResults = true;

    printf("\n");
    while(list != NULL) {
        
        if(list->year == year) {
            noResults = false;
            printf("\t%s\n", list->title);
        }
        list = list->next;
    }
    if(noResults) {
                printf("\tNo data about movies released in the year %d\n", year);
    }
}

bool isInArray(int arr[], int size, int val) {
    for(int i = 0; i < size; i++) {
        if(arr[i] == val) {
            return true;
        }
    }
    return false;
}

void displayHighestRatedInYear(struct movie *list) {
    struct movie *head = list;
    struct movie *highestRated = NULL;
    int yearsProcessed[121] = {};

    while(head != NULL) {
        while(isInArray(yearsProcessed, 121, head->year)) {
            head = head->next;
            if(head == NULL) {
                return;
            }
        }
        yearsProcessed[head->year % 121] = head->year;
        highestRated = head;
        list = head->next;
        while(list != NULL) {
            if(highestRated->year == list->year) {
                if(highestRated->rating < list->rating) {
                    highestRated = list;
                }
            }
            list = list->next;
        }
        
        printf("\t%d %0.1f %s\n", highestRated->year, highestRated->rating, highestRated->title);

        head = head->next;
    }
}

void displaySpecificLanguage(struct movie *list, char *language) {
    bool noResults = true;
    while(list != NULL) {
        for(int i = 0; i < list->langCount; i++) {
            if(strcmp(list->languages[i], language) == 0) {
                printf("\t%d %s\n", list->year, list->title);
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
            default: printf("\nYou entered an incorrect choice. Try again.");
                     break;
        }
    } while (choice != 4);
}