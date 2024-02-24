#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "builtInCommands.h"


void sh_exit(pid_t fgPid, pid_t bgPids[], int bgCount) {\
    // // //
    // Built-in command exit. Must kill all current processes before
    // exit of smallsh. Reads the fgPid and bgProcesses[] global
    // vars to send SIGTERM to all processes. calls exit(0) when done.

    // Parameters:
    //     fgPid: fgPid global var
    //     bgPids: bgProccess[] global array
    //     bgCount: bgCount global var
    // // //
    if(fgPid) {
        kill(fgPid, SIGTERM);
    }
    for(int i = 0; i < bgCount; i++) {
        kill(bgPids[i], SIGTERM);
    }
    exit(0);
}

void sh_cd(struct command* command) {
    // // //
    // Built-in command cd. When called without an argument,
    // sets current directory to the environment HOME variable.
    // If an argument is present, sets the current directory to
    // the argument.

    // uses chdir to change directory, and getenv to get HOME env variable.

    // Parameters:
    //     command: The command struct representing the user input
    // // //
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
    // // //
    // Built-in command status. Given an int representing the raw
    // status code given from waitpid, use the macros WIFEXITED and
    // WIFISIGNALED to determine type of exit, and WEXITSTATUS and WTERMSIG
    // to get the exit or signal number.

    // Uses a simple if/else chain referenced from the Exploration: Process
    // API - Monitoring Child Processes module.

    // Parameters:
    //     status: an int representing the raw exit code given from waitpid
    // // //
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