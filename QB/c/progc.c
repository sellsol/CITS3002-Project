#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/stat.h>

//TODO: Fix perror messages? They're kind of bland right now.

//Reads text file to string and returns pointer to string. Pointer must be freed after use.
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

//Returns NULL if lastAttempt = 0, else returns output.
//Modifies completed to be 1 if succeeded, 0 if not
char* testCode(int *completed, char *path, char *expectedOut, int lastAttempt, char *imageName) {
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
			execl(path, "code", (char *) NULL);
			//Should probably use execv for multiple parameters
			perror("User program crashed");
			exit(EXIT_FAILURE);
		default:
			close(thepipe[1]); //Will never write

			//If this is an image, we check it like so
			if (imageName != NULL) {
				close(thepipe[0]);

				char *imagePath = calloc(strlen("./code/XXXXXX/") + strlen(imageName) + 1, sizeof char);
				strcat(imagePath, "./code/XXXXXX/");
				strcat(imagePath, imageName);
				char *outputImage = readTextFile(imagePath);

				completed = (strcmp(expectedOut, outputImage) == 0) ? 1 : 0;

				if (lastAttempt == 1) {
					return outputImage;
				}

				free(outputImage);
				return NULL;
			}
			//Else it's some text response, so we check as shown below

			//Initialise variables
			int expectedLen = strlen(expectedOut) + 1;
			char *output = malloc(expectedLen);

			//Read from the pipe until we are finished or we have read more data than we were expecting
			int pos = 0;
			int got = read(thepipe[0], output, expectedLen);
			while (got != 0 || pos > expectedLen) {
				pos += got;
				got = read(thepipe[0], output + pos, expectedLen - pos);
			}
			wait(NULL);
			close(thepipe[0]);
			
			//We got more input than we were expecting, return failure
			if (pos > expectedLen) {
				completed = 0;
				if (lastAttempt == 1) { //Return output if this is the last attempt
					// concat ... and then we stopped reading to the end of this
					return output;
				}

				free(output);
				return NULL;
			}

			//Mark whether the test was successfuly completed or not
			completed = (strcmp(output, expectedOut) == 0) ? 1 : 0; 
			
			//Return result if this is the last attempt
			if (lastAttempt == 1) {
				return output;
			}

			free(output);
			return NULL;
	}
}

//Returns 0 if any of the tests fail, 1 otherwise. See testCode for my comments on this
//lastAttempt is 1 if it's the last attempt (and therefore needs to return output error)
char* compileCode(int* completed, char* question, char* code, int lastAttempt) {
	//char codePath[20];
	//strcpy(codePath, "./code/XXXXXX.c"); //Make directories for each question
	//int pFd = mkstemps(codePath, 2);
	//write(pFd, code, strlen(code));
	//close(pFd); //Should unlink all codepaths at the end
	//char *execPath = strndup(codePath, strlen("./code/XXXXXX"));
	
	char *dirPath = mkdtemp("./code/XXXXXX");
	char *codePath = calloc(strlen("./code/XXXXXX") + strlen("/code.c"), sizeof char);
	
	strcat(codePath, dirPath);
	strcat(codePath, "/code.c");

	int pFd = creat(codePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //Error handle here
	write(pFd, code, strlen(code));
	close(pFd);

	char *execPath = strndup(codePath, strlen("./code/XXXXXX/code"));

	switch (fork()) {
		case -1:
			//Halt and catch fire
			exit(EXIT_FAILURE);
		case 0: //Child process - execv
			execl("/usr/bin/gcc", "gcc", codePath, "-o", execPath, (char *) NULL);
			perror("/usr/bin/gcc");
			exit(EXIT_FAILURE);
		default: //Parent process - run tests
			//Wait until child program has finished executing
			wait(NULL);

			char *questionPath = calloc(strlen("./progq//") + strlen(question) + 1, sizeof char);
			strcat(questionPath, "./progq/");
			strcat(questionPath, question);
			strcat(questionPath, "/");
			//Open relevant folder, start running all tests
			DIR *d = opendir(questionPath); //Combine this with questionNumber

			//Go through every entry in the directory stream
			for (struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
				if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
				printf("%s\n", dir->d_name);

				char *inPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/in"), sizeof(char));
				char *outPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/out"), sizeof(char));
				char *pngPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/png"), sizeof(char));
				//optimise

				//string concatenate path names		Better to use snprintf?
				strcat(inPath, questionPath);
				strcat(inPath, dir->d_name);
				strcat(inPath, "/in");

				strcat(outPath, questionPath);
				strcat(outPath, dir->d_name);
				strcat(outPath, "/out");

				strcat(pngPath, questionPath);
				strcat(pngPath, dir->d_name);
				strcat(pngPath, "/png");

				//Get input file if it exists
				char *in = NULL;
				if (access(inPath, F_OK) == 0) {
					in = readTextFile(inPath);
				}

				char *png = NULL;
				if (access(pngPath, F_OK) == 0) {
					png = readTextFile(pngPath);
				}

				char *out = NULL;
				if (access(outPath, F_OK) == 0) {
					out = readTextFile(outPath);
				}
				
				//Free paths
				free(inPath);
				free(outPath);
				free(pngPath);

				int ret = 0;
				char *out = testCode(&ret, execPath, out, lastAttempt, png);
				
				//Free provided data
				free(in);
				free(out);
				free(png);

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
