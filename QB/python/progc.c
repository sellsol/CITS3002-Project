#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/stat.h>



//TODO: Fix perror messages? They're kind of bland right now.

char* readTextFile(char *path) {
	struct stat *buf = malloc(sizeof(struct stat));
	stat(path, buf); //Need to error handle
	int size = buf->st_size;

	char *out = malloc(size); //Need to really make sure this works
	int oFd = open(path, O_RDONLY);
	int got = read(oFd, out, size);
	while (got < size) {
		got += read(oFd, out + got, size - got);
	}

	out[got - 1] = '\0';

	return out;
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
			
			//Initialise variables
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
int compileCode(char* code, char* questionId) {
	char codePath[20];
	strcpy(codePath, "./code/XXXXXX.c");
	int pFd = mkstemps(codePath, 2);
	write(pFd, code, strlen(code));
	close(pFd); //Should unlink all codepaths at the end
	char *execPath = strndup(codePath, strlen("./code/XXXXXX"));

	switch (fork()) {
		case -1:
			//Halt and catch fire
			exit(EXIT_FAILURE);
		case 0: //Child process - execv
			execl("/usr/bin/gcc", "gcc", codePath, "-o", execPath, (char *) NULL);
			printf("UH OH STINKY GCC ERROR\n");
			exit(EXIT_FAILURE);
		default: //Parent process - run
			wait(NULL); //Wait until program has finished executing

			char *bugger = calloc(strlen("./tests//") + strlen(questionId), sizeof(char));
			strcat(bugger, "./tests/");
			strcat(bugger, questionId);
			strcat(bugger, "/");
			//Open relevant folder, start running all tests
			DIR *d = opendir(bugger);

			//Go through every entry in the directory stream
			for (struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
				if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, "tests.txt")) continue;
				printf("%s\n", dir->d_name);

				char *inPath = calloc(strlen(bugger) + strlen(dir->d_name) + strlen("/in"), sizeof(char));
				char *outPath = calloc(strlen(bugger) + strlen(dir->d_name) + strlen("/out"), sizeof(char));
				//optimise

				//string concatenate path names
				strcat(inPath, bugger);
				strcat(inPath, dir->d_name);
				strcat(inPath, "/in");

				strcat(outPath, bugger);
				strcat(outPath, dir->d_name);
				strcat(outPath, "/out");

				char *in = NULL;
				if (access(inPath, F_OK) == 0) {
					in = readTextFile(inPath);
				}
				char *out = readTextFile(outPath);
				//assess if input/output files exist, else equals nullpointer

				free(inPath);
				free(outPath);

				int ret = testCode(execPath, execPath + strlen("./code/"), in, out);
				
				free(in);	
				free(out);

				if (ret == 1) {
					closedir(d);
					unlink(codePath);
					unlink(execPath);
					return ret;
				}
			}

			closedir(d);
			unlink(codePath);
			unlink(execPath);
			return 0;
	}
}

int main(void) {
	printf("%i\n", compileCode("#include<stdio.h>\n\nint main(void) {\n\tint a = 1;\n\tprintf(\"1\\n1\\n2\\n3\\n5\\n8\\n13\\n21\\n34\\n55\");\n}", "1"));
}
