#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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