#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

//TODO: Fix perror messages? They're kind of bland right now.

char* readTextFile(char *path1, char *path2, int len) {
	char *output = malloc(len);

	strcpy();

	return output;
}

//Returns 1 on failed, 0 on success. Returning error messages is also possible, just return char*
int testCode(char *path, char *indicator, char *in, char *expectedOut) {
	int thepipe[2];

	if (pipe(thepipe) != 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	switch (fork()) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE); //Should we just crash the program? Not sure
		case 0: //Child process - execl
			close(thepipe[0]);
			dup2(thepipe[1], 1);

			close(thepipe[1]);
			execl(path, indicator, (char *) NULL);
			//Should probably use execv for multiple parameters
			perror("User program crashed");
			exit(EXIT_FAILURE);
		default:
			close(thepipe[1]); //Will never write
			
			//Initialie variables
			int expectedLen = strlen(expectedOut) + 1;
			char* output = malloc(expectedLen);

			//Read from the pipe until we are finished or we have read more data than we were expecting
			int pos = 0;
			int got = read(thepipe[0], output, expectedLen);
			while (got != 0 || pos > expectedLen) {
				pos += got;
				got = read(thepipe[0], output + pos, expectedLen - pos);
			}

			//We got more input than we were expecting, return failure
			if (pos > expectedLen) {
				return 1;
			}

			int ret = 1;
			if (strcmp(output, expectedOut) == 0) {
				ret = 0;
			}

			free(output);
			wait(NULL);
			return ret;
	}
}

//Returns 1 if any of the tests fail, 0 otherwise. See testCode for my comments on this
int compileCode(char* code) {
	//int fd = creat("./code/hey.c", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	//write(fd, code, strlen(code)); //Need to do error handling here

	//close(fd);

	switch (fork()) {
		case -1:
			//Halt and catch fire
		case 0: //Child process - execv
			execl("/usr/bin/gcc", "gcc", "./code/hey.c", "-o", "./code/a", (char *) NULL);
			printf("UH OH STINKY GCC ERROR\n");
			exit(EXIT_FAILURE);
		default: //Parent process - run
			wait(NULL); //Wait until program has finished executing

			//Open relevant folder, start running all tests
			DIR *d = opendir("./progq/q1/tests");

			//Go through every entry in the directory stream
			for (struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
				if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
				printf("%s\n", dir->d_name);

				char *inPath = malloc(strlen("./progq/q1/tests/") + strlen(dir->d_name) + strlen("/in"));
				char *outPath = malloc(strlen("./progq/q1/tests/") + strlen(dir->d_name) + strlen("/out"));
				//optimise

				//string concatenate path names

				//assess if input/output files exist, else equals nullpointer

				//use stat to read all data, pass to ret

				//wrap this in a function

				int ret = testCode("./code/a", "a", );

				free(inPath);
				free(outPath);
				
				if (ret == 1) {
					return ret;
				}
			}

			closedir(d);
			return 0;
	}
}

int main(void) {
	printf("%i\n", compileCode(NULL));
}
