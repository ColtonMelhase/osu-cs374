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
        } else if(strcmp(tokens[i], "&") == 0 && i+1 == tokenCount) { // Set execute in background flag
            currCommand->execBackground = 1;
        } else { // Anything else is set as an argument
            currCommand->args[currCommand->argc] = calloc(strlen(tokens[i]) + 1, sizeof(char));
            strcpy(currCommand->args[currCommand->argc], tokens[i]);
            currCommand->argc++;
        }
    }
    return currCommand;
}

void printCommandLine(struct command* aCommand) {
    printf("Command: %s\n", aCommand->command);
    printf("Argument count: %d\n", aCommand->argc);
    printf("Arguments:\n");
    for(int i = 0; i < aCommand->argc; i++) {
        printf("\t%s\n", aCommand->args[i]);
    }
    printf("Input file: %s\n", aCommand->input_file);
    printf("Output file: %s\n", aCommand->output_file);
    printf("Background process? %s\n", aCommand->execBackground ? "True" : "False");
}

void sh_exit() {
    // terminate any other processes

    // exit
    exit(0);
}

void sh_cd(struct command* command) {
    if(command->argc > 1) {
        printf("cd: too many arguments\n"); fflush(stdout);
        return;
    } else if(command->argc == 1) {
        // printf("\nargument: %s\n", command->args[0]);
        int err = chdir(command->args[0]);
        if(err == -1) {
            printf("cd: %s: No such file or directory", command->args[0]); fflush(stdout);
        }
    } else {
        chdir(getenv("HOME"));
    }

	printf("\n%s\n", getcwd(NULL, 256));
}

void sh_status() {

}

int main() {

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);
	struct command* command;
	while(1) { // run until exit command


		printf(": "); fflush(stdout); // prompt
		fgets(userCommand, sizeof(char) * 2049, stdin);	// get user input
		userCommand[strcspn(userCommand, "\n")] = 0;	// strip fgets' trailing \n
		command = parseCommandLine(userCommand);        // parse user input

        // printCommandLine(command);

		if(userCommand[0] == '#' || userCommand[0] == '\n') { // if command is blank or a comment
			// do nothing
		} else if(strcmp(command->command, "exit") == 0) {
            sh_exit();
		} else if(strcmp(command->command, "cd") == 0) {
            sh_cd(command);
		} else if(strcmp(command->command, "status") == 0) {
            sh_status();
		}
	}
}