#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "builtInCommands.h"

char *strreplace(char *s, const char *s1, const char *s2) {
    char *p = strstr(s, s1);
    if (p != NULL) {
        size_t len1 = strlen(s1);
        size_t len2 = strlen(s2);
        if (len1 != len2)
            memmove(p + len2, p + len1, strlen(p + len1) + 1);
        memcpy(p, s2, len2);
    }
    return s;
}

void handle_SIGINT(int signo) {
    char* message = "\n";
    write(STDOUT_FILENO, message, 1);
}
void configure_SIGINT() {
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = handle_SIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
}

// SIGTSTP Handler
void handle_SIGTSTP(int signo) {
    char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
    write(STDOUT_FILENO, message, 52);
}
// Sets the SIGTSTP response to handle_SIGTSTP()
void configure_SIGTSTP() {
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}


int main() {

    // Set SIGINT handler for the shell to ignore ctrl+C
    configure_SIGINT();
    configure_SIGTSTP();

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);
	struct command* command;

    int* childStatus = 0;

    char buf[256];

	while(1) { // run until exit command
        userCommand = strcpy(userCommand, "");

        printf(": "); fflush(stdout);                        // prompt
		// printf("%s: ", getcwd(buf, 256)); fflush(stdout); // prompt w/ cwd
		fgets(userCommand, sizeof(char) * 2049, stdin);	// get user input
		userCommand[strcspn(userCommand, "\n")] = 0;	// strip fgets' trailing \n
        char* pid_str = malloc(sizeof(char) * 5);       // expand $$
        while(strstr(userCommand, "$$") != NULL) {      // replace '$$' w/ PID
            sprintf(pid_str, "%d", getpid());
            strreplace(userCommand, "$$", pid_str);
        }
		command = parseCommandLine(userCommand);        // parse user input

        // printCommandLine(command);

		if(userCommand[0] == '#' || userCommand[0] == '\n' || command->command == NULL) { // if command is blank or a comment
			// do nothing
		} else if(strcmp(command->command, "exit") == 0) {
            free(userCommand);
            free(pid_str);
            freeCommand(command);
            sh_exit();
		} else if(strcmp(command->command, "cd") == 0) {
            sh_cd(command);
		} else if(strcmp(command->command, "status") == 0) {
            sh_status(childStatus);
		} else {
            // If not comment/blank or built-in command, perform EXEC
            // Construct newargv[]
            char* newargv[1 + command->argc + 1];
            newargv[0] = command->command;
            for(int i = 0; i < command->argc; i++) {
                newargv[i+1] = command->args[i];
            }
            newargv[command->argc + 1] = NULL;

            
            pid_t spawnPid = fork();

            switch(spawnPid) {
                case -1:
                        perror("fork()\n");
                        break;
                case 0:
                        // In the child process
                        execvp(newargv[0], newargv);
                        perror("execvp");
                        exit(2);
                        break;
                default:
                        // In the parent process
                        // Wait for child's termination
                        spawnPid = waitpid(spawnPid, &childStatus, 0);
                        printf("Process PID: %d is done.\n", spawnPid); fflush(stdout);
                        break;
            }

        }
        free(pid_str);
        freeCommand(command);
	}
}