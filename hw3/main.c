// Colton Melhase
// melhasec
// CS374 W2024
// Project 3 - Small Shell

// TODO
// Put input/output redirection after fork, so unneccessary redirection of parent shell is removed.
// Put input/output redirection into separate functions.
// Refactor replacePid with sprintf.
// Put signal handlers within separate files.
// Dynamically manage background processes. Map? Linked list?


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

// Global variables to keep track of meta data like the shell mode and processes.
int mode = 0;           // 0 -> foreground/background. 1 -> foreground only.

int bgCount = 0;        // tracks the # of background processes executed
int bgProcesses[50];    // tracks the history of background processes

pid_t fgPid = 0;        // if non-zero, foreground process currently executing.

bool isInArray(int arr[], int size, int val) {
    // // //
    // Given an array of integers, the size of the array, and the value to look for,
    // return true if the integer is already in the array, and false if it is not.

    // Used in checkBackgroundProcess() to check if a PID in bgProcesses[] is completed

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

void replacePid(char *s) {
    // // //
    // Given a string representing the user input, uses strstr to check if "$$" is present.
    // Manipulates memory within the string to inject the PID.

    // Used in checkBackgroundProcess() to check if a PID in bgProcesses[] is completed

    // Parameters:
    //     s: string representing the user input
    // // //
    char* pid_str = malloc(sizeof(char) * 10);
    sprintf(pid_str, "%d", getpid());
    while(strstr(s, "$$") != NULL) {
        char *p = strstr(s, "$$");
        if (strlen(pid_str) != 2) // strlen("$$") == 2
            memmove(p + strlen(pid_str), p + 2, strlen(p + 2) + 1);
        memcpy(p, pid_str, strlen(pid_str));
    }
    free(pid_str);
}


void handle_SIGINT(int signo) {
    // SIGINT Handler
    // If a foreground process is active, SIGINT from the parent will forward the SIGINT
    // to the child process. The checkForegroundProcess()'s waitpid will now catch the end
    // of the process, and display the status.
    // Else, create a newline, and prompt again.
    if(fgPid) {
        kill(fgPid, SIGINT);
        return;
    }
    char* message = "\n";
    write(STDOUT_FILENO, message, 1);
}
void configure_SIGINT() {
    // Sets the SIGINT response to handle_SIGINT()
    struct sigaction SIGINT_action = {{0}};
    SIGINT_action.sa_handler = handle_SIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
}


void handle_SIGTSTP(int signo) {
    // SIGTSTP Handler
    // Toggles the global mode variable. If mode = 0, then it will toggle to 1, representing
    // foreground-only mode, and print a message.
    // If mode = 1, it will toggle to 0, representing background/foreground mode, and print a message
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
void configure_SIGTSTP() {
    // Sets the SIGTSTP response to handle_SIGTSTP()
    // Note the flag SA_RESTART to not interrupt any current process.  
    struct sigaction SIGTSTP_action = {{0}};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

void processHandler(struct command* command, int* childStatus, int* mode) {
    // // //
    // The processHandler is called when the command does not match any built-in functions.
    // The handler will direct the IO to the proper files and/or stream, and then call
    // fork() and execvp() and handle child and parent processes.

    // Packaging all of these operations within this handler allows for a less-bloated main function,
    // and ensures all children are managed the same way.

    // Parameters:
    //     command: the struct containing the command details
    //     childStatus: the pointer containing the status code for the child
    //     mode: the pointer to the global int representing foreground-only mode

    // Return:
    //     None
    // // //

    char errMsg[100];
    // Construct newargv[] for execvp() call.
    char* newargv[1 + command->argc + 1];
    newargv[0] = command->command;
    for(int i = 0; i < command->argc; i++) {
        newargv[i+1] = command->args[i];
    }
    newargv[command->argc + 1] = NULL;

    // save stin/stout to redirect parent back after fork
    int saved_stin = dup(0);
    int saved_stout = dup(1);

    // Set file IO
    
    // Input
    if(command->input_file != NULL) { // Set input if given
        int sourceFD = open(command->input_file, O_RDONLY);
        if (sourceFD == -1) { 
            sprintf(errMsg, "open(): %s", command->input_file);
            perror(errMsg); 
            return;
        }
        int dupResult = dup2(sourceFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %d", sourceFD);
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
        int dupResult = dup2(sourceFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %d", sourceFD);
            perror(errMsg);
            return;
        }
    }
    
    // Output
    if(command->output_file != NULL) { // Set output if given
        int targetFD = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { 
            sprintf(errMsg, "open(): %s", command->output_file);
            perror(errMsg); 
            return;
	    }
        int dupResult = dup2(targetFD, 1);
        if (dupResult == -1) { 
            sprintf(errMsg, "dup2(): %d", targetFD);
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
        int dupResult = dup2(targetFD, 0);
        if (dupResult == -1) {
            sprintf(errMsg, "dup2(): %d", targetFD);
            perror(errMsg);
            return;
        }
    }
    
    // IO now configured. Call fork() and handle child/parent
    pid_t spawnPid = fork();

    switch(spawnPid) {
        case -1:
                perror("fork() failed\n");
                break;
        case 0:
                // In the child process
                if(command->runBackground == 1) { // if bg ps, ignore SIGINT
                    struct sigaction SIGINT_action = {{0}};
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
                    fgPid = spawnPid;                       // update fg process
                    spawnPid = waitpid(spawnPid, childStatus, 0);
                } else if(command->runBackground == 1) {    // if & is given
                    bgProcesses[bgCount] = spawnPid;        // update bg processes
                    bgCount++;
                    // printf("\nBG processes: [ "); fflush(stdout);
                    // for(int i = 0; i < bgCount; i++) {
                    //     printf("%d, ", bgProcesses[i]); fflush(stdout);
                    // }
                    // printf("]\n"); fflush(stdout);


                    printf("background pid is %d\n", spawnPid); fflush(stdout);
                    spawnPid = waitpid(spawnPid, childStatus, WNOHANG);
                }
                break;
    }
}

void checkBackgroundProcesses() {
    // // //
    // Checks bgProcesses[] and bgCount global variables and scans through them
    // calling waitpid with the WNOHANG flag to see if they ended. 
    // If they have, print out a status message before
    // the next prompt is displayed.
    // // //
    pid_t checkPid;
    pid_t checkStatus;
    for(int i = 0; i < bgCount; i++) {
        if((checkPid = waitpid(bgProcesses[i], &checkStatus, WNOHANG)) > 0) {
            printf("background pid %d is done: ", bgProcesses[i]); fflush(stdout);
            sh_status(checkStatus);
            printf("\n"); fflush(stdout);
        }
    }
}

void checkForegroundProcess() {
    // // //
    // Checks fgPid global variable and calls waitpid if a foreground process
    // was executed. Prints a status message if and only if the foreground
    // process was interrupted by SIGINT. Otherwise, if the foreground process
    // ends normally, the waitpid within the processHandler will catch.
    // // //
    pid_t checkPid;
    pid_t checkStatus;
    if(fgPid) {
        if((checkPid = waitpid(fgPid, &checkStatus, 0)) > 0) {
        sh_status(checkStatus);
        printf("\n"); fflush(stdout);
        fgPid = 0;
        }
    }
}

// Execution pseudo-code
    // Configure signal handlers

    // initialize user input, data struct for command, and status variable.

    // while loop forever until built-in exit is called.
    //     check for any processes completed
    //         display status messages if needed

    //     display prompt
    //     get user input, parse, and create command struct

    //     check if user input is blank or a comment
    //         do nothing
    //     check if user input is built-in command
    //         call built-in command
    //     otherwise,
    //         call the process handler to fork and pass command to executed

    //      repeat

int main() {
    
    // Set SIGINT handler for the shell to ignore ctrl+C
    configure_SIGINT();
    configure_SIGTSTP();

	// allocate memory to store command. Command line must
	// support max length of 2048. Thus 2049 bytes given. (+1 for \0)
	char* userCommand = malloc(sizeof(char) * 2049);
	struct command* command;

    int childStatus = 0;

	while(1) { // run until exit command

        checkBackgroundProcesses();
        checkForegroundProcess();
        userCommand = strcpy(userCommand, "");

        printf(": "); fflush(stdout);                   // prompt
		fgets(userCommand, sizeof(char) * 2049, stdin);	// get user input
		userCommand[strcspn(userCommand, "\n")] = 0;	// strip fgets' trailing \n
        replacePid(userCommand);
		command = parseCommandLine(userCommand);        // parse user input

		if(userCommand[0] == '#' || userCommand[0] == '\n' || command->command == NULL) { // if command is blank or a comment
			// do nothing
		} else if(strcmp(command->command, "exit") == 0) {
            free(userCommand);
            freeCommand(command);
            sh_exit(fgPid, bgProcesses, bgCount);
		} else if(strcmp(command->command, "cd") == 0) {
            sh_cd(command);
		} else if(strcmp(command->command, "status") == 0) {
            sh_status(childStatus);
            printf("\n"); fflush(stdout);
		} else {
            processHandler(command, &childStatus, &mode);
        }
        freeCommand(command);
	}
}