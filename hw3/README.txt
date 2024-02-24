To compile the main.c file, run the command

    gcc --std=gnu99 -o smallsh main.c command.c builtInCommands.c

Then to run the program, run the following command.

    smallsh

After successfully compiling, the grading script "p3testscript" can now
be ran by using the following command.

    p3testscript