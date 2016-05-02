/* 
* Solution for Lab #3, CS 306 in Spring 2015
* Author: Jessica Van Meter
* 
* Will take any two executable commands with or without options
* and use the output of the first command as the input of the second.
*
* Basically a simplified version of the Linux pipe utility.
*
* Assumes no more than 5 total strings per command including options.
*
* Compile as:  gcc -Wall -std=gnu99 -o mypipe lab3.c
* Call as:  mypipe COMMAND1 COMMAND2
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int command(char *commstr);
void parse(char *str, char *args[]);

int main(int argc, char *argv[]){
	
	pid_t pid; //used to capture child's PID in parent
	pid_t pid2; //used to capture 2nd child's PID in parent
	int status; //captures the status of the wait call
	int status2; //captures 2nd status of 2nd wait call
	
	if (argc != 3){
		fprintf(stderr,"Usage: mypipe COMMAND1 COMMAND2");
        exit(EXIT_FAILURE); 
	}
	//put argument strings into variables for neatness/personal preference
	char *comm1 = argv[1]; 
	char *comm2 = argv[2];
	int pipefd[2]; //to be used with pipe
	
	if((pipe(pipefd)) == -1){
		fprintf(stderr,"Error: Pipe failed.");
		return EXIT_FAILURE;
	}
	switch (pid = fork()){
		case -1: //the error case, fork failed
			fprintf(stderr,"Fork failed");
			exit(EXIT_FAILURE);
		case 0: //child process:
			if((close(pipefd[0])) == -1){
				fprintf(stderr,"Error: Pipe failed to close write end in first child process.");
				return EXIT_FAILURE;
			}
			if((dup2(pipefd[1],1)) == -1){
				fprintf(stderr,"Error: Pipe: File redirection failed.");
				return EXIT_FAILURE;
			}
			if((command(comm1)) == -1){
				fprintf(stderr,"Error: Pipe failed to close write end in first child process.");
				return EXIT_FAILURE;
			}
			exit(EXIT_SUCCESS); //exit() to ensure cannot drop into
								//parent code
		default: //parent process:
			if ((waitpid(pid,&status,0)) == -1){
				fprintf(stderr,"Error: Wait failed");
				return EXIT_FAILURE;
			}
			switch (pid2 = fork()){
				case -1: //the error case, fork failed
					fprintf(stderr,"Fork failed");
					exit(EXIT_FAILURE);
				case 0: //2nd child process:
					if((close(pipefd[1])) == -1){
						fprintf(stderr,"Error: Pipe failed to close read end in second child process.");
						return EXIT_FAILURE;
					}
					if ((dup2(pipefd[0],0)) == -1){
						fprintf(stderr,"Error: Pipe: File redirection failed.");
						return EXIT_FAILURE;
					}
					if((command(comm2)) == -1){
						fprintf(stderr,"Error: Command function failed.");
						return EXIT_FAILURE;
					}
					exit(EXIT_SUCCESS); 
				default: //parent process:
					if((close(pipefd[0])) == -1){
						fprintf(stderr,"Error: Pipe failed to close write end in parent process.");
						return EXIT_FAILURE;
					}
					if((close(pipefd[1])) == -1){
						fprintf(stderr,"Error: Pipe failed to close read end in parent process.");
						return EXIT_FAILURE;
					}
					if ((waitpid(pid2,&status2,0)) == -1){
						fprintf(stderr,"Error: Wait failed");
						return EXIT_FAILURE;
					}
			}
	}
	return EXIT_SUCCESS;
}

int command(char *commstr){
	char *args[6]; 
	
	parse(commstr, args);
	
	execvp(commstr, args);
	//error message will only run if exec fails, as exec replaces the
	//rest of the code once it is executed.
	fprintf(stderr,"Error: execvp failed in command 2.");
	return -1;
}

void parse(char *str, char *args[]){
	int i = 0;
	args[i++] = strtok(str," ");
	while((args[i++] = strtok(NULL," ")) != NULL);
}
