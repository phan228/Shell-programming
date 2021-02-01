#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "shell.h"

extern FILE *stdin;

struct command* readcmd(char* cmdline, FILE *infile);
void call_redirected(char *program, struct command* aline);

struct command* readcmd(char* cmdline, FILE *infile) {

	fgets(cmdline,512,infile); 
		if (feof(infile)) { //end of file error exit(0);
			exit(1);
		}
		if (ferror(infile)){	//other errors
			exit(1);
		}

		return parse_command((char*)cmdline);
}

void call_redirected(char *program, struct command* aline) {

	int status = 0;
	

	//starts external command process
	pid_t f = fork();
	if (f < 0) {
		perror("fork() failed");
		return;
	}
	else if (f == 0) {	//child process
		if (aline->in_redir != NULL) {
			int infile_descriptor = open((const char*)aline->in_redir, O_RDONLY, 0666);
			if (infile_descriptor < 0) {
				perror("Failed to open!");
				exit(1);
			}
			else {
				int infile_dup = dup2(infile_descriptor, 0);
				if (infile_dup < 0) {
					perror("Failed to move file_descriptor!");
					exit(1);
				}
			}
		}
			
		if (aline->out_redir != NULL) {	
			int outfile_descriptor = open((const char*)aline->out_redir, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			if (outfile_descriptor < 0) {
				perror("Failed to open!");
				exit(1);
			}
			else {
				int outfile_dup = dup2(outfile_descriptor, 1);
				if (outfile_dup < 0) {
					perror("Failed to move file_descriptor!");
					exit(1);
				}
			}
		}		
				execvp(program, aline->args);
				perror("execve() failed!");
				exit(1);
	}
	
	else {	//parent process
		waitpid(f, &status, WUNTRACED);
		int killSig = WIFSIGNALED(status);
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				return;		//exited normally
			}
			else {
				fprintf(stderr, "Command returned 3\n");
				return;
			}
		}
		if (killSig == 1) {
			fprintf(stderr, "Command killed: %s\n", strsignal(WTERMSIG(status)));
		}
	}
}


int main(int argc, char **argv) {

	char cmdline[512];
	struct command* line;
	FILE* fi1;
	int goOnce = 1;

	while (1) {
		
		if (argc == 1) {	//run ./shell
			fprintf(stderr, "shell> ");
			fi1 = stdin;
		}

		else if (argc == 2) {	//run ./shell scriptfile

			if (goOnce == 1) {
				FILE *fi = fopen(argv[1], "r");
				fi1 = fi;
			if (fi == NULL) {
				perror("File failed to open!");
				exit(1);
			}
			goOnce = 2;
		}
		}
		line = readcmd((char*) cmdline, (FILE*) fi1);

		//fprintf(stderr,"%s\n",line->args[0]);
		if (line->args[0] == NULL)  return 0;		//ignore empty commands
		
		else if (strcmp(line->args[0],"\n") == 0)
			fprintf(stdout, "only one line\n");
		
		//test 0
		else if (strcmp(line->args[0], "echo") == 0) {
			call_redirected(line->args[0],line);
		}

		// command cd
		else if (strcmp(line->args[0], "cd") == 0) {
			if (line->args[1] == NULL) {	//run cd
				fprintf(stderr,"inredir %s\n",line->args[1]);
				if (getenv("HOME") != NULL) {
					chdir(getenv("HOME"));
				}
				else 
					perror("cannot find home directory\n");
			}
			else {		//run cd dir
				chdir(line->args[1]);
				if (chdir(line->args[1]) == -1) {
					perror("cannot change directory\n");
				}
			}
		}

		//command setenv
		else if (strcmp(line->args[0], "setenv") == 0) {
			if (line->args[1] == NULL) {		//no arguments
				fprintf(stderr,"still alive");
			}
			else if (line->args[2] == NULL) {	//run setenv variable
				unsetenv(line->args[1]);
			}
			
			if (unsetenv(line->args[1]) == -1) {
				perror("unsetenv() failed\n");
			}
			
			else {		//run setenv variable value
				int set = setenv(line->args[1], line->args[2], 1);
				if (set == -1) {
					perror(strerror(errno));
				}
			}
		}

		//command exit
		else if (strcmp(line->args[0], "exit") == 0)
			exit(0);

		//external command
		else call_redirected(line->args[0],line);

		free_command((struct command *) line); 	//free the line returned by parse_cmd() before reading the next one
	}
}