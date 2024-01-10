
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct movie {
    char *title;
    int year;
    char languages[5][20];
    double rating;
    struct movie *next;
};

struct movie *createMovie(char *currLine) {
    struct movie *currMovie = malloc(sizeof(struct movie));

    char *saveptr;

    // Process title
    char *token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // Process year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = atoi(token);

    return currMovie;
}

struct movie *processFile(char *filePath) {

    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

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
    printf("%s, %d\n", aMovie->title, aMovie->year);
}

void printMovieList(struct movie *list) {
    while(list != NULL) {
        printMovie(list);
        list = list->next;
    }
}
int main(int argc, char *argv[]) {
    struct movie *list = processFile(argv[1]);
    printMovieList(list);
}