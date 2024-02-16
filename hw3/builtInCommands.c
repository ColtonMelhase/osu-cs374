#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtInCommands.h"


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

char* sh_status(int status) {
    if(WIFEXITED(status)) {
        char* exit = calloc(15, sizeof(char));
        sprintf(exit, "exit value %d", WEXITSTATUS(status));
        return exit;
    } else {
        char* terminated = calloc(25, sizeof(char));
        sprintf(terminated, "terminated by signal %d", WTERMSIG(status));
        return terminated;
    }
}