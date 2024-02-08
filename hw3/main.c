#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

struct command {
	char* command;
	int argc;
	char* args[512];
	char* input_file;
	char* output_file;
	int execBackground;
};

struct command* parseCommandLine(char* userCommand) {
	struct command *currCommand = malloc(sizeof(struct command));

    // Tokenize words separated by " " in command 
    char* tokens[520];
    char *token;        
    char* saveptr;
    int tokenCount = 0;
    while(token = strtok_r(userCommand, " ", &saveptr)) {
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
        } else if(strcmp(tokens[i], "&") == 0) { // Set execute in background flag
            currCommand->execBackground = 1;
        } else { // Anything else is set as an argument
            currCommand->args[currCommand->argc] = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->args[currCommand->argc], tokens[i]);
            currCommand->argc++;
        }
    }
    return currCommand;
}

void sh_exit() {
    exit(1);
}

void sh_cd() {

}

void sh_status() {

}

int main() {

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);

	while(1) { // run until exit command
		printf(": "); fflush(stdout); // prompt
		fgets(userCommand, sizeof(char) * 2049, stdin);	// get user input

		if(userCommand[0] == '#' || userCommand[0] == '\n') { // if command is blank or a comment
			// do nothing
		} else {
			printf("%s", userCommand); fflush(stdout);
		}
	}
}