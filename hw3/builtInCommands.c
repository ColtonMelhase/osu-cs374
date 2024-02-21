#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "builtInCommands.h"


void sh_exit(pid_t fgPid, pid_t bgPids[], int bgCount) {
    // terminate any other processes
    if(fgPid) {
        kill(fgPid, SIGTERM);
    }
    for(int i = 0; i < bgCount; i++) {
        kill(bgPids[i], SIGTERM);
    }

    // exit
    exit(0);
}

void sh_cd(struct command* command) {
    if(command->argc > 1) {
        printf("cd: too many arguments\n"); fflush(stdout);
        return;
    } else if(command->argc == 1) {
        int err = chdir(command->args[0]);
        if(err == -1) {
            printf("smallsh: cd: %s: No such file or directory\n", command->args[0]); fflush(stdout);
        }
    } else {
        chdir(getenv("HOME"));
    }
}

void sh_status(int status) {
    if(WIFEXITED(status)) {
        char* exit = calloc(15, sizeof(char));
        sprintf(exit, "exit value %d", WEXITSTATUS(status));
        printf("%s", exit); fflush(stdout);
        free(exit);
    } else {
        char* terminated = calloc(25, sizeof(char));
        sprintf(terminated, "terminated by signal %d", WTERMSIG(status));
        printf("%s", terminated); fflush(stdout);
        free(terminated);
    }
}