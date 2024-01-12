
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    printf("%s,%d,[", aMovie->title, aMovie->year);
    for(int i = 0; i < aMovie->langCount; i++) {
        printf("%s;", aMovie->languages[i]);
    }

    printf("],%0.2f\n", aMovie->rating);
}

void printMovieList(struct movie *list) {
    while(list != NULL) {
        printMovie(list);

        // save pointer to movie printed to free memory
        struct movie *delMovie = list;
        list = list->next;  // set list to the next movie

        // free memory of printed movie
        free(delMovie->title);
        for(int i = 0; i < delMovie->langCount; i++) {
            free(delMovie->languages[i]);
        }
        free(delMovie);
    }
}
int main(int argc, char *argv[]) {
    struct movie *list = processFile(argv[1]);
    printMovieList(list);
}