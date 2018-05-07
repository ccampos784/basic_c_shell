/*
 * shell.c
 *
 *  Created on: Mar 19, 2018
 *      Author: Christopher Campos
 *	MIT License

 * 	Copyright (c) 2018 Christopher Campos

 * 	Permission is hereby granted, free of charge, to any person obtaining a copy
 * 	of this software and associated documentation files (the "Software"), to deal
 * 	in the Software without restriction, including without limitation the rights
 * 	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * 	copies of the Software, and to permit persons to whom the Software is
 * 	furnished to do so, subject to the following conditions:

 * 	The above copyright notice and this permission notice shall be included in all
 * 	copies or substantial portions of the Software.

 * 	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * 	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * 	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#define MAX_ARGS 127
#define DEFAULT_BUF_SIZE 512
#define SHELL_VERSION 1

/*
 * void printinfo() - When the user provided the help or info command
 * this function will print out some info about the shell.
 */
void printinfo() {
	printf("**********************************************************\n");
	printf("Basic UNIX shell - version %d\n", SHELL_VERSION);
	printf("By Christopher Campos\n\n");
	
	//print license info
	printf("MIT License\nCopyright (c) 2018 Christopher Campos\nPermission is hereby granted, free of charge, to any person obtaining a copy\nof this software and associated documentation files (the 'Software'), to deal\nin the Software without restriction, including without limitation the rights\nto use, copy, modify, merge, publish, distribute, sublicense, and/or sell\ncopies of the Software, and to permit persons to whom the Software is\nfurnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all\ncopies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE.\n\n");
	
	printf("Suppored features:\n");
	printf(" - Interactive mode (no arguments)\n");
	printf(" - Batch mode (batch file as single argument)\n");
	printf(" - Output redirection to file (use > or >> operator)\n");
	printf(" - Running background process (use & operator)\n\n");
	printf("To wait for background processes use 'barrier' command.\n");
	printf("To quit use 'exit' or 'quit' command.\n");
	printf("**********************************************************\n");
	return;
}

/*
 * char** parsecmd(char*)
 * This function takes in a string as input. The string is a user provided command. This function will split
 * the function into substrings with spaces as delimiters for use with execvp().
 */
char** parsecmd(char* input, int* argc) {
	int count = 0;
	char* delim = " \t\n";
	char* token;
	char* ip = input;
	/* array of c-strings to store token arguments */
	char** tokens = malloc( sizeof(char*) * MAX_ARGS );
	
	/* tokenize the input string using strtok */
	while ( (token = strtok(ip, delim)) != NULL ) {
		tokens[count] = token;
		count++;
		ip = NULL;
	}

	/*return the array */
	tokens[count] = NULL;
	*argc = count;
	return tokens;
}

/*
 * void executeCmd(char*) - This command will run the process provided by the user.
 */
void executeCmd(char* input) {
	int redir_flag = 0, bg_flag = 0, status = 0, outp = 0;
	char* outf;
	int* arg_count = malloc(sizeof(int));
	char** args = parsecmd(input, arg_count);
	pid_t wpid;

	/*if input is blank do nothing and return */
	if (*arg_count == 0) {
		return;
	}
	
	/* if first character is # then it is a comment so skip*/
	if (input[0] == '#') {
		return;
	} 

	/* we need to check for redirection
	 * if the command has "> somefile.txt" then we need
	 * to redirect output from stdout to that file
	 */
	if ((*arg_count > 2) && (strcmp(args[(*arg_count)-2], ">") == 0) ) {
		/* open a file for output using UNIX system call */
		outf = malloc( sizeof(args[(*arg_count)-1]) );
		strcpy(outf, args[(*arg_count)-1]);
		redir_flag = 1;

		/* the argument list contains "> filename" as arguments
		 * We should get rid of these by truncating the array.
		 * The end of the array is specified using a NULL pointer.
		 * Also reduce arg_count by 2. */

		args[(*arg_count)-2] = NULL;
		args[(*arg_count)-1] = NULL;
		*arg_count = *arg_count - 2;
	}
	
	/* we should also support an append mode
	 * to make our lives easier :)
	 */
	 
	if ((*arg_count > 2) && (strcmp(args[(*arg_count)-2], ">>") == 0) ) {
		/* open a file for output using UNIX system call */
		redir_flag = 2;
		outf = malloc( sizeof(args[(*arg_count)-1]) );
		strcpy(outf, args[(*arg_count)-1]);

		/* the argument list contains ">> filename" as arguments
		 * We should get rid of these by truncating the array.
		 * The end of the array is specified using a NULL pointer.
		 * Also reduce arg_count by 2. */

		args[(*arg_count)-2] = NULL;
		args[(*arg_count)-1] = NULL;
		*arg_count = *arg_count - 2;
	}


	/* we have to check for the & symbol
	 * if that exists we just let the program run
	 * in the background */
	if ( args[(*arg_count)-1][strlen(args[(*arg_count)-1])-1] == '&' ) {
		bg_flag = 1;
		args[(*arg_count)-1][strlen(args[(*arg_count)-1])-1] = '\0';
	}

	/* check for exit or quit command */
	if ( strcmp(args[0], "quit") == 0 || strcmp(args[0], "exit") == 0) {
		exit(0);
	}

	/* check for barrier command */
	if ( strcmp(args[0], "barrier") == 0 ) {
		while ((wpid = wait(&status)) > 0);
		return;
	}
	
	/* check for barrier command */
	if ( strcmp(args[0], "help") == 0 || strcmp(args[0], "info") == 0) {
		printinfo();
		return;
	}
	
	/* fork a new process */
	pid_t pid = fork();

	/* if pid == 0 then we are in the child process */
	if (pid == 0) {
		/* if redir_flag is set to 1 or 2 we should redirect
		 * stdout to the file specified. At the end close
		 * the file since only the descriptor is necessary */
		
		if (redir_flag == 1) {
			outp = open(outf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (outp < 0) {
				perror("Error");
				return;
			}
			close(STDOUT_FILENO);
			dup2(outp, STDOUT_FILENO);
			close(outp);
		}
		
		if (redir_flag == 2) {
			outp = open(outf, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (outp < 0) {
				perror("Error");
				return;
			}
			close(STDOUT_FILENO);
			dup2(outp, STDOUT_FILENO);
			close(outp);
		}

		if ( execvp(args[0], args) < 0 ) {
			perror("Error");
			exit(1);
		}

	} else {
		if (bg_flag == 0) {
			while ((wpid = wait(&status)) > 0);
		}
	}
}

/* void interactive()
 * This function is run if no commands are passed to the shell program. We will run the shell
 * in interactive mode in an infinite loop.
 */
void interactive() {
	while (1) {
		char prompt[] = "prompt> ";
		char input[DEFAULT_BUF_SIZE];

		printf("%s", prompt);
		/* read in input */
		if ( fgets(input, sizeof(input), stdin) != NULL ) {
			executeCmd(input);
		}
		else {
			return;
		}
	}
}

/* void batch()
 * This function is run if a filename argument is passed to
 * the shell function. It will run the commands in the batch file.
 */
void batch(char* filename) {
	char input[DEFAULT_BUF_SIZE];
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Error opening file");
		return;
	}

	while ( fgets(input, sizeof(input), fp) != NULL ) {
		executeCmd(input);
	}

	fclose(fp);
}


int main (int argc, char* argv[]) {
	/* no arguments - interactive mode */
	if (argc == 1) {
		printinfo();
		interactive();
		return 0;
	/* 1 argument - batch mode */
	} else if (argc == 2) {
		batch(argv[1]);
		return 0;
	} else {
		printf("Invalid arguments\n");
		return 1;
	}
}
