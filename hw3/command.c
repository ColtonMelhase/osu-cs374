#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "command.h"


struct command* parseCommandLine(char* userCommand) {
    // // //
    // Given the user input, allocates a command struct, and parses user input to fill
    // data within the struct.

    // Using a struct to store the data within a user input allows easy getting and 
    // understanding of what data we are accessing. Prevents the need to store
    // many different variables, and puts them all in one package.

    // Uses strtok_r to parse through the user input. Heavily referenced from previous 2
    // projects. 

    // Parameters:
    //     userCommand: string representing user input

    // Return:
    //     pointer to the newly allocated command
    // // //
	struct command *currCommand = malloc(sizeof(struct command));
    
    // Initalize currCommand
    currCommand->command = NULL;
    currCommand->argc = 0;
    for(int i = 0; i < 512; i++) {
        currCommand->args[i] = NULL;
    }
    currCommand->input_file = NULL;
    currCommand->output_file = NULL;
    currCommand->runBackground = 0;

    // Tokenize words separated by " " in command 
    char* tokens[520]; // Since we need to handle 512 arguments, 520 spaces for tokens is given
    char *token;        
    char* saveptr;
    int tokenCount = 0;
    while((token = strtok_r(userCommand, " ", &saveptr))) {
        tokens[tokenCount] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(tokens[tokenCount], token);
        tokenCount++;
        userCommand = saveptr;
    }   // Command line now tokenized in tokens[]

    // Read through list of tokens and organize them within command struct
    for(int i = 0; i < tokenCount; i++) {
        if(i == 0) {    // Set command
            currCommand->command = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->command, tokens[i]);
        } else if(strcmp(tokens[i], "<") == 0) { // Set input file
            i++;
            currCommand->input_file = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->input_file, tokens[i]);
        } else if(strcmp(tokens[i], ">") == 0) { // Set output file
            i++;
            currCommand->output_file = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->output_file, tokens[i]);
        } else if(strcmp(tokens[i], "&") == 0 && i+1 == tokenCount) { // Set execute in background flag
            currCommand->runBackground = 1;
        } else { // Anything else is set as an argument
            currCommand->args[currCommand->argc] = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->args[currCommand->argc], tokens[i]);
            currCommand->argc++;
        }
    }
    
    // Clean up memory allocation
    for(int i = 0; i < tokenCount; i++) {
        free(tokens[i]);
    }

    return currCommand;
}

// Debug print function that displays all data within a command struct. Used only during dev cycle.
void printCommandLine(struct command* aCommand) {
    printf("Command: %s\n", aCommand->command); fflush(stdout);
    printf("Argument count: %d\n", aCommand->argc); fflush(stdout);
    printf("Arguments:\n"); fflush(stdout);
    for(int i = 0; i < aCommand->argc; i++) {
        printf("\t%s\n", aCommand->args[i]); fflush(stdout);
    }
    printf("Input file: %s\n", aCommand->input_file); fflush(stdout);
    printf("Output file: %s\n", aCommand->output_file); fflush(stdout);
    printf("Background process? %s\n", aCommand->runBackground ? "True" : "False"); fflush(stdout);
}

// Free command function that frees all allocated memory within a command struct.
void freeCommand(struct command* aCommand) {
    free(aCommand->command);
    for(int i = 0; i < aCommand->argc; i++) {
        free(aCommand->args[i]);
    }
    free(aCommand->input_file);
    free(aCommand->output_file);
    free(aCommand);
}