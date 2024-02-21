#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "builtInCommands.h"

int mode = 0; // 0 == foreground/background. 1 == foreground only.
int sigChldFlag = 0;
pid_t sigChldPID = 0;
int sigChldStatus = 0;

int bgCount = 0;
int bgProcesses[50];

pid_t fgPid = 0;

bool isInArray(int arr[], int size, int val) {
    // // //
    // Given an array of integers, the size of the array, and the value to look for,
    // return true if the integer is already in the array, and false if it is not.

    // Parameters:
    //     arr: The array of numbers
    //     size: The size of the array
    //     val: The value to look for

    // Return:
    //     boolean reporting if val is in arr or not.
    // // //
    for(int i = 0; i < size; i++) {
        if(arr[i] == val) {
            return true;
        }
    }
    return false;
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

// SIGINT Handler
void handle_SIGINT(int signo) {
    if(fgPid) {
        kill(fgPid, SIGINT);
        return;
    }
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
    if(mode == 0) {
        mode = 1;
        char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, 52);
    } else {
        mode = 0;
        char* message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, 32);
    }
    
}
// Sets the SIGTSTP response to handle_SIGTSTP()
void configure_SIGTSTP() {
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

// void handle_SIGCHLD(int signo) {
//     pid_t childPid;
//     int childStatus;
//     while((childPid = waitpid(-1, &sigChldStatus, WNOHANG)) > 0) {
//         if(isInArray())
//         sigChldFlag = 1;
//         sigChldPID = childPid;
//             // bgCount--;
//             // bgProcesses[bgCount] = -5;
        
//     }
//     // char* message = "\n";
//     // write(STDOUT_FILENO, message, 1);
// }
// void configure_SIGCHLD() {
//     struct sigaction SIGCHLD_action = {0};
//     SIGCHLD_action.sa_handler = handle_SIGCHLD;
//     sigfillset(&SIGCHLD_action.sa_mask);
//     SIGCHLD_action.sa_flags = 0;
//     sigaction(SIGCHLD, &SIGCHLD_action, NULL);
// }
void processHandler(struct command* command, int* childStatus, int* mode) {
    // If not comment/blank or built-in command, perform EXEC

    char errMsg[100];
    // Construct newargv[]
    char* newargv[1 + command->argc + 1];
    newargv[0] = command->command;
    for(int i = 0; i < command->argc; i++) {
        newargv[i+1] = command->args[i];
    }
    newargv[command->argc + 1] = NULL;

    // save stin/stout to redirect back after fork
    int saved_stin = dup(0);
    int saved_stout = dup(1);
    // Set file IO
    // Set input if given
    if(command->input_file != NULL) {
        int sourceFD = open(command->input_file, O_RDONLY);
        if (sourceFD == -1) { 
            sprintf(errMsg, "open(): %s", command->input_file);
            perror(errMsg); 
            return;
        }
        // printf("sourceFD == %d\n", sourceFD); fflush(stdout);
        int dupResult = dup2(sourceFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %s", sourceFD);
            perror(errMsg);
            return;
        }
    } else if (command->runBackground == 1) { // input not given and bg ps -> /dev/null
        int sourceFD = open("/dev/null", O_RDONLY);
        if (sourceFD == -1) { 
            sprintf(errMsg, "open(): %s", "/dev/null");
            perror(errMsg); 
            return;
        }
        // printf("sourceFD == %d\n", sourceFD); fflush(stdout);
        int dupResult = dup2(sourceFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %s", sourceFD);
            perror(errMsg);
            return;
        }
    }
    
    // Set output if given
    if(command->output_file != NULL) {
        int targetFD = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { 
            sprintf(errMsg, "open(): %s", command->output_file);
            perror(errMsg); 
            return;
	    }
        // printf("targetFD == %d\n", targetFD); fflush(stdout);
        int dupResult = dup2(targetFD, 1);
        if (dupResult == -1) { 
            sprintf(errMsg, "dup2(): %s", targetFD);
            perror(errMsg); 
            return;
        }
    } else if (command->runBackground == 1) { // output not given and bg ps -> /dev/null
        int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { 
            sprintf(errMsg, "open(): %s", "/dev/null");
            perror(errMsg); 
            return;
        }
        // printf("targetFD == %d\n", targetFD); fflush(stdout);
        int dupResult = dup2(targetFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %s", targetFD);
            perror(errMsg);
            return;
        }
    }
    
    pid_t spawnPid = fork();

    switch(spawnPid) {
        case -1:
                perror("fork() failed\n");
                break;
        case 0:
                // In the child process
                if(command->runBackground == 1) { // if bg ps
                    struct sigaction SIGINT_action = {0};
                    SIGINT_action.sa_handler = SIG_IGN;
                    sigfillset(&SIGINT_action.sa_mask);
                    SIGINT_action.sa_flags = 0;
                    sigaction(SIGINT, &SIGINT_action, NULL);
                }
                execvp(newargv[0], newargv);
                perror("execvp");
                exit(1);
                break;
        default:
                // In the parent process
                // Redirect back to stdin/stout for the terminal
                dup2(saved_stin, 0);
                dup2(saved_stout, 1);

                // Depending if & is given and/or foreground-only mode is enabled, wait
                if(*mode == 1 || command->runBackground == 0) { // foreground-only mode OR & is not given
                    // printf("FOREGROUND\n"); fflush(stdout);
                    fgPid = spawnPid;
                    spawnPid = waitpid(spawnPid, childStatus, 0);
                } else if(command->runBackground == 1) { // if & is given
                    // printf("BACKGROUND\n"); fflush(stdout);
                    bgProcesses[bgCount] = spawnPid;
                    bgCount++;
                    printf("background pid is %d\n", spawnPid); fflush(stdout);
                    spawnPid = waitpid(spawnPid, childStatus, WNOHANG);
                }
                break;
    }
}

void checkBackgroundProcesses() {
    pid_t checkPid;
    pid_t checkStatus;
    // printf("\nCURRENT BACKGROUND PROCESSES:\n"); fflush(stdout);
    // for(int i = 0; i < bgCount; i++) {
    //     printf("\t%d\n", bgProcesses[i]); fflush(stdout);
    // }
    // printf("\nCOMPLETED BACKGROUND PROCESSES:\n"); fflush(stdout);
    for(int i = 0; i < bgCount; i++) {
        if(checkPid = waitpid(bgProcesses[i], &checkStatus, WNOHANG) > 0) {
            printf("background pid %d is done: ", bgProcesses[i]); fflush(stdout);
            sh_status(checkStatus);
            printf("\n"); fflush(stdout);
        }
    }
}

void checkForegroundProcess() {
    pid_t checkPid;
    pid_t checkStatus;
    if(fgPid) {
        if(checkPid = waitpid(fgPid, &checkStatus, 0) > 0) {
        sh_status(checkStatus);
        printf("\n"); fflush(stdout);
        fgPid = 0;
        }
    }
}

int main() {
    
    // Set SIGINT handler for the shell to ignore ctrl+C
    configure_SIGINT();
    configure_SIGTSTP();
    // configure_SIGCHLD();

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);
	struct command* command;

    int childStatus = 0;

    int promptPresent = 0;

    char buf[256];

	while(1) { // run until exit command

        // if(sigChldFlag) {
        //     if(promptPresent) {
        //         printf("\n"); fflush(stdout);
        //     }
        //     printf("background pid %d is done: %s\n", sigChldPID, sh_status(sigChldStatus)); 
        //     // sh_status(sigChldStatus);
        //     sigChldFlag = 0;
        // }

        checkBackgroundProcesses();
        checkForegroundProcess();
        userCommand = strcpy(userCommand, "");

        printf(": "); fflush(stdout);                        // prompt
        promptPresent = 1;
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
            promptPresent = 0;
            free(userCommand);
            free(pid_str);
            freeCommand(command);
            sh_exit(fgPid, bgProcesses, bgCount);
		} else if(strcmp(command->command, "cd") == 0) {
            promptPresent = 0;
            sh_cd(command);
		} else if(strcmp(command->command, "status") == 0) {
            promptPresent = 0;
            sh_status(childStatus);
            printf("\n"); fflush(stdout);
		} else {
            promptPresent = 0;
            processHandler(command, &childStatus, &mode);
        }
        free(pid_str);
        freeCommand(command);
	}
}