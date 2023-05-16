#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/stat.h>
#include <ftw.h>

#include "mode.h"

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	if (strcmp(fpath + 14, "code") == 0 || strcmp(fpath + 14, "code.py") == 0) {
		return 0;
	}
	int rv = remove(fpath);

	if (rv) {
		perror(fpath);
	}

	return rv;
}

int unlink_cb2(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int rv = remove(fpath);

	if (rv) {
		perror(fpath);
	}

	return rv;
}
//Reads text file to string and returns pointer to string. Pointer must be freed after use.
char* readTextFile(char *path) {
	struct stat *buf = malloc(sizeof(struct stat));
	stat(path, buf); //Need to error handle
	int size = buf->st_size;

	char *out = malloc(size + 1); //Need to really make sure this works Feels like I should add 1
	int oFd = open(path, O_RDONLY);
	int got = read(oFd, out, size);
	while (got < size) {
		got += read(oFd, out + got, size - got);
	}

	out[got] = '\0';

	return out;
}

//Returns NULL if lastAttempt = 0, else returns output.
//Modifies completed to be 1 if succeeded, 0 if not
char** testCode(char *completed, char *path, char lastAttempt, char *in[], char *expectedOut, char *expectedImage) {
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
            if (PROGRAM_MODE == C) {
			    execv(path, in);
            } else {
                execv("/usr/bin/python3", in);
            }
			//Should probably use execv for multiple parameters
			perror("User program crashed");
			exit(EXIT_FAILURE);
		default:
			close(thepipe[1]); //Will never write

			//If this is an image, we check it like so
			if (expectedImage != NULL) {
				close(thepipe[0]);

				char *imagePath = calloc(strlen("./code/XXXXXX/image.png"), sizeof(char));
				strncat(imagePath, path, strlen("./code/XXXXXX/"));
				strcat(imagePath, "image.png");
				char *outputImage = readTextFile(imagePath);

				*completed = (strcmp(expectedOut, outputImage) == 0) ? 1 : 0;

				if (lastAttempt == 1) {
					char **r = malloc(2*sizeof(char*));
					r[0] = expectedOut;
					r[1] = outputImage;
					return r;
				}

				free(outputImage);
				return NULL;
			}
			//Else it's some text response, so we check as shown below

			//Initialise variables
			int expectedLen = 10000; //Not allowed to have 10000 or more chars
			char *output = malloc(expectedLen);

			//Read from the pipe until we are finished or we have read more data than we were expecting
			int pos = 0;
			int got = read(thepipe[0], output, expectedLen);
			while (got != 0 || pos == expectedLen) {
				pos += got;
				got = read(thepipe[0], output + pos, expectedLen - pos);
			}
			output[pos] = '\0';
			wait(NULL);
			close(thepipe[0]);
			
			//We got more input than we were expecting, return failure
			if (pos > expectedLen) {
				*completed = 0;
				if (lastAttempt == 1) { //Return output if this is the last attempt
					// concat ... and then we stopped reading to the end of this
					char **r = malloc(2 * sizeof(char *));
					r[0] = expectedOut;
					r[1] = output;
					return r;
				}

				free(output);
				return NULL;
			}

			//Mark whether the test was successfuly completed or not
			*completed = (strcmp(output, expectedOut) == 0) ? 1 : 0; 
			
			//Return result if this is the last attempt
			if (lastAttempt == 1) {
				char **r = malloc(2 * sizeof(char *));
				r[0] = expectedOut;
				r[1] = output;
				return r;
			}

			free(output);
			return NULL;
	}
}

//Returns 0 if any of the tests fail, 1 otherwise. See testCode for my comments on this
//lastAttempt is 1 if it's the last attempt (and therefore needs to return output error)
char** compileCode(char* completed, char* question, char* code, char lastAttempt) {
	//Create temporary directory for running question
	char tempPath[14] = "./code/XXXXXX";
	char *dirPath = mkdtemp(tempPath);

    char *vl;
    if (PROGRAM_MODE == C) {
        vl = "/code.c";
    } else {
        vl = "/code.py";
    }

    //Create path for the compiled code
	char *codePath = calloc(strlen("./code/XXXXXX") + strlen(vl) , sizeof(char));

	strcat(codePath, dirPath);
    strcat(codePath, vl);

    //Write the code to file
    int pFd = creat(codePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //Error handle here
    write(pFd, code, strlen(code));
    close(pFd);

    if (PROGRAM_MODE == C) {
	    //Get the executable path
	    char *execPath = strndup(codePath, strlen("./code/XXXXXX/code"));

        switch(fork()) {
            case -1:
                exit(EXIT_FAILURE);
            case 0: //Child process - execv
			    execl("/usr/bin/gcc", "gcc", codePath, "-o", execPath, (char *) NULL);
			    perror("/usr/bin/gcc");
			    exit(EXIT_FAILURE);
            default:
                wait(NULL);
        }
		unlink(codePath);
        free(codePath);
        codePath = execPath;
    }

    char *questionPath = calloc(strlen("./progq//") + strlen(question) + 1, sizeof(char));
	strcat(questionPath, "./progq/");
	strcat(questionPath, question);
	strcat(questionPath, "/");

	//Open relevant folder, start running all tests
	DIR *d = opendir(questionPath);

	//Go through every entry in the directory stream
	for (struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
			//printf("%s\n", dir->d_name);

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

			//printf("%s\n", outPath);

			strcat(pngPath, questionPath);
			strcat(pngPath, dir->d_name);
			strcat(pngPath, "/png");

			//Get input file if it exists
			char **inArgs = NULL;
			if (access(inPath, F_OK) == 0) {
				printf("READING INPUT...\n");
				char *in = readTextFile(inPath);
				//Handle reading input file

				//Count all the instances of '\n'
				int len = strlen(in);
				int count = 1;
				for (int i = 0; i < len; i++) {
					if (in[i] == '\n') {
						count += 1;
					}
				}

				if (PROGRAM_MODE == PYTHON) {
					count += 2;
				} else {
					count += 1;
				}
				

				//Create array with count + 1
				inArgs = malloc((count + 1) * sizeof(char *));

				int i = 1;
				if (PROGRAM_MODE == PYTHON) {
					inArgs[0] = "/usr/bin/python3";
					inArgs[1] = codePath;
					i = 2;
				} else {
					inArgs[0] = "code";
				}
				printf("%i\n", count + 1);
				//strtok all values to array
				char *token = strtok(in, "\n");
				while (token != NULL) {
					printf("%i - %s\n",i , token);
					inArgs[i] = strdup(token);
					i += 1;
					token = strtok(NULL, "\n");
				}
				inArgs[count] = NULL;
				free(in);
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

			char ret;
			printf("TESTING...\n");
			char **output = testCode(&ret, codePath, lastAttempt, inArgs, out, png);
			nftw(dirPath, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
			//printf("%s\n", output[1]);

			//Free provided data
			//free(in);
			free(png);

			if (ret == 0) {
				*completed = 0;
				closedir(d);
				unlink(codePath);
				nftw(dirPath, unlink_cb2, 64, FTW_DEPTH | FTW_PHYS);
				if (lastAttempt == 1) {
					return output;
				}

				free(out);
				free(output);
				return NULL;
			}
			free(out);
		}

		*completed = 1;

		closedir(d);
		unlink(codePath);
		nftw(dirPath, unlink_cb2, 64, FTW_DEPTH | FTW_PHYS);
		return NULL;
}