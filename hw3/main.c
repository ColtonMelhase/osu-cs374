#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "command.h"

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
        //printf("\nargument: %s\n", command->args[0]);
        int err = chdir(command->args[0]);
        if(err == -1) {
            printf("smallsh: cd: %s: No such file or directory\n", command->args[0]); fflush(stdout);
        }
    } else {
        chdir(getenv("HOME"));
    }
    // char buf[256];
	// printf("\n%s\n", getcwd(buf, 256));
}

void sh_status(int status) {
    if(WIFEXITED(status)) {
        printf("exit value %d\n", WEXITSTATUS(status));
    } else {
        printf("terminated by signal %d\n", WTERMSIG(status));
    }
}

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

int main() {

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);
	struct command* command;

    int childStatus = 0;

    char buf[256];

	while(1) { // run until exit command


		printf("%s: ", getcwd(buf, 256)); fflush(stdout); // prompt
		fgets(userCommand, sizeof(char) * 2049, stdin);	// get user input
		userCommand[strcspn(userCommand, "\n")] = 0;	// strip fgets' trailing \n
        char* pid_str = malloc(sizeof(char) * 5);       // expand $$
        while(strstr(userCommand, "$$") != NULL) {
            sprintf(pid_str, "%d", getpid());
            strreplace(userCommand, "$$", pid_str);
        }
		command = parseCommandLine(userCommand);        // parse user input

        // printCommandLine(command);

		if(userCommand[0] == '#' || userCommand[0] == '\n') { // if command is blank or a comment
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
            char bin[300] = "/bin/";
            newargv[0] = strcat(bin, command->command);
            for(int i = 0; i < command->argc; i++) {
                newargv[i+1] = command->args[i];
            }
            newargv[command->argc + 1] = NULL;

            int childStatus;
            pid_t spawnPid = fork();

            switch(spawnPid) {
                case -1:
                        perror("fork()\n");
                        break;
                case 0:
                        // In the child process
                        execv(newargv[0], newargv);
                        perror("execv");
                        break;
                default:
                        // In the parent process
                        // Wait for child's termination
                        spawnPid = waitpid(spawnPid, &childStatus, 0);
                        break;
            }

        }

        free(pid_str);
        freeCommand(command);
	}
}