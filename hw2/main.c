// Colton Melhase
// CS374 WR2024
// 1/31/2024
// Brewster

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

            "Enter a choice from 1 to 3:");
}

int main() {
    int choiceMain = -1;

    do {
        displayMainMenu();
        choiceMain = -1;
        scanf("%d", choiceMain);

        switch(choiceMain) {
            case 1:
                bool fileProcessed = false;
                do {
                    displayFileMenu();
                    int choiceFile = -1;
                    scanf("%d", choiceFile);

                    switch(choiceFile) {
                        
                        case 1:
                            printf("\nChoice 1");
                            fileProcessed = true;
                            break;
                        case 2:
                            printf("\nChoice 2");
                            fileProcessed = true;
                            break;
                        case 3:
                            printf("\nChoice 3");
                            fileProcessed = true;
                            break;
                        default:
                            printf("\nThe file null was not found. Try again");
                            break;
                    }
                } while (!fileProcessed);
                break;
            case 2:
                printf("\nQuitting...");
                exit(0);
                break;
        }
    } while (choiceMain != 2);
}
