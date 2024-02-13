struct command {
	char* command;
	int argc;
	char* args[512];
	char* input_file;
	char* output_file;
	int runBackground;
};


struct command* parseCommandLine(char* userCommand);

void printCommandLine(struct command* aCommand);

void freeCommand(struct command* aCommand);