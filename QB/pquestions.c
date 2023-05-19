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
#include "pquestions.h"

const char *c_path = "./questions/c/";
const char *py_path = "./questions/python/";
const char* q_path;

const int MAX_OUTPUT_LEN = 10000;

//File tree walk to clear a folder after a test - preserving the code
int testUnlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	//Preserve code files
	if (strcmp(fpath + 14, "code") == 0 || strcmp(fpath + 14, "code.py") == 0) {
		return 0;
	}
	//Else remove
	int rv = remove(fpath);

	if (rv) {
		perror(fpath);
	}

	return rv;
}

//File tree walk to clear a folder after running all tests - complete annihilation
int compileUnlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int rv = remove(fpath);

	if (rv) {
		perror(fpath);
	}

	return rv;
}

//Reads text file to string and returns pointer to string. Pointer must be freed after use.
struct FileData readFile(char *path) {
	if (access(path, F_OK) != 0) {
		return (struct FileData){0, NULL};
	}
	//Stat to get file size
	struct stat *buf = malloc(sizeof(struct stat));
	stat(path, buf); //Need to error handle
	int size = buf->st_size;

	char *out = malloc(size + 1); //Plus 1 is for end-of-line character
	int oFd = open(path, O_RDONLY); //Open file

	//Initialise reader variables
	int ret = read(oFd, out, size);
	int got = ret; //This should be done better

	//Read entire file
	while (got < size) {
		ret = read(oFd, out + got, size - got);
		if (ret == -1) {
			return (struct FileData){0, NULL};
		}
		got += ret;
	}

	out[got] = '\0';

	//Format data in struct and return
	return (struct FileData){size, out};
}

//Returns NULL if lastAttempt = 0, else returns output.
//Modifies completed to be 1 if succeeded, 0 if not. 2 means we've had some error
struct FileData testCode(char *completed, char lastAttempt, char *in[], char *expectedOut, struct FileData expectedImage) {
	//Create pipe for running the program
	int thepipe[2];

	if (pipe(thepipe) != 0) {
		perror("pipe");
		*completed = 3;
		return (struct FileData){0, NULL};
	}

	//Fork to execute program
	switch (fork()) {
		case -1:
			perror("fork");
			*completed = 3;
			return (struct FileData){0, NULL};
		case 0: //Child process - execv
			close(thepipe[0]);
			dup2(thepipe[1], 1);
			dup2(thepipe[1], 2);

			close(thepipe[1]);

			//Run the program using execv depending on it's type
            if (PROGRAM_MODE == C) {
			    execv("./code", in);
            } else {
                execv("/usr/bin/python3", in);
            }
			//Should handle this better
			perror("User program crashed");
			exit(EXIT_FAILURE);
		default:
			close(thepipe[1]); //Will never write

			//Initialise variables
			struct FileData output;
			output.data = calloc(MAX_OUTPUT_LEN, sizeof(char));

			//Read from the pipe until we are finished or we have read more data than we were expecting
			int pos = 0;
			int got = read(thepipe[0], output.data, MAX_OUTPUT_LEN);
			while (got != 0 || pos == MAX_OUTPUT_LEN) {
				pos += got;
				got = read(thepipe[0], output.data + pos, MAX_OUTPUT_LEN - pos);
			}
			output.data[MAX_OUTPUT_LEN - 1] = '\0';

			wait(NULL);
			close(thepipe[0]);

			//If this is an image, we check it like so
			if (expectedImage.data != NULL) {
				struct FileData outputImage = readFile("./image.png");

				//If not the same length, return
				if (expectedImage.len != outputImage.len) {
					if (lastAttempt == 1) {
						*completed = 2;
						return outputImage;
					}
					*completed = 1;
					return (struct FileData){0, NULL};
				}

				//Compare images. Cannot use strcmp due to null characters present
				for (int i = 0; i < outputImage.len; i++) {
					if (outputImage.data[i] != expectedImage.data[i]) {
						if (lastAttempt == 1) {
							*completed = 2;
							return outputImage;
						}
						*completed = 1;
						return (struct FileData){0, NULL};
					}
				}
				*completed = 0;

				free(outputImage.data);
				return (struct FileData){0, NULL};
			}
			
			//We got more input than we were expecting, return failure
			if (pos > MAX_OUTPUT_LEN) {
				*completed = 1;
				if (lastAttempt == 1) { //Return output if this is the last attempt
					return output;
				}

				free(output.data);
				return (struct FileData){0, NULL};
			}

			//Mark whether the test was successfuly completed or not
			*completed = (strcmp(output.data, expectedOut) == 0) ? 0 : 1; 
			
			//Return result if this is the last attempt
			if (lastAttempt == 1) {
				return output;
			}

			free(output.data);
			return (struct FileData){0, NULL};
	}
}

//Returns 0 if any of the tests fail, 1 otherwise, 2 if it's a failure and image, 3 if we fail. See testCode for my comments on this
//lastAttempt is 1 if it's the last attempt (and therefore needs to return output error)
struct FileData* compileCode(char* completed, char* question, char* code, char lastAttempt) {
	//Create temporary directory for running question
	char tempPath[14] = "./code/XXXXXX";
	char *dirPath = mkdtemp(tempPath);

	//Error handle mkdtemp
	if (dirPath == NULL) {
		*completed = 3;
		return (struct FileData []){(struct FileData){0, NULL}, (struct FileData){0, NULL}};
	}
    //Create path for the compiled code
	char *codePath = calloc(strlen("./code/XXXXXX/code.py"), sizeof(char));

	//Concatenate path
	strcat(codePath, dirPath);
	if (PROGRAM_MODE == C) {
		strcat(codePath, "/code.c");
	} else {
		strcat(codePath, "/code.py");
	}

    //Write the code to file
    int pFd = creat(codePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //Error handle here
    write(pFd, code, strlen(code));
    close(pFd);

    if (PROGRAM_MODE == C) {
		int thepipe[2];

		if (pipe(thepipe) != 0) {
			perror("pipe");
			*completed = 3;
			return NULL;
		}
		
	    //Get the executable path
	    char *execPath = strndup(codePath, strlen("./code/XXXXXX/code"));

		//Fork and compile using gcc
        switch(fork()) {
            case -1:
                perror("fork");
				*completed = 3;
				return (struct FileData []){(struct FileData){0, NULL}, (struct FileData){0, NULL}};
            case 0: //Child process - execv
				close(thepipe[0]);
				dup2(thepipe[1], 2);

				close(thepipe[1]);

			    execl("/usr/bin/gcc", "gcc", codePath, "-o", execPath, (char *) NULL);
			    perror("/usr/bin/gcc");
			    exit(EXIT_FAILURE); //Definitely need to report any compile errors
            default:
                close(thepipe[1]); //Will never write

				//Initialise variables
				struct FileData output;
				output.data = calloc(MAX_OUTPUT_LEN, sizeof(char));

				//Read from the pipe until we are finished or we have read more data than we were expecting
				int pos = 0;
				int got = read(thepipe[0], output.data, MAX_OUTPUT_LEN);
				while (got != 0 || pos == MAX_OUTPUT_LEN) {
					pos += got;
					got = read(thepipe[0], output.data + pos, MAX_OUTPUT_LEN - pos);
				}
				output.data[MAX_OUTPUT_LEN - 1] = '\0';

				wait(NULL);
				close(thepipe[0]);
				printf("%s\n", output.data);

				//Process data
				if (pos != 0) {
					printf("FAILED TO COMPILE\n");
					*completed = 1;
					if (lastAttempt == 1) {
						return (struct FileData []){(struct FileData){0, NULL}, (struct FileData){pos, output.data}};
					}
					return NULL;
				}
        }
		unlink(codePath);
        free(codePath);
        codePath = execPath;
    }

	//Get path to questions
    char *questionPath = calloc(strlen(q_path) + strlen(question) + 2, sizeof(char));
	strcat(questionPath, q_path);
	strcat(questionPath, question);
	strcat(questionPath, "/");

	//Open relevant folder, start running all tests
	DIR *d = opendir(questionPath);


	//Go through every entry in the directory stream
	for (struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

			//Get paths for test part 1
			char *inPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/in"), sizeof(char));
			char *outPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/out"), sizeof(char));
			char *pngPath = calloc(strlen(questionPath) + strlen(dir->d_name) + strlen("/png"), sizeof(char));

			//Get paths for test part 2
			sprintf(inPath, "%s%s%s", questionPath, dir->d_name, "/in");
			sprintf(outPath, "%s%s%s", questionPath, dir->d_name, "/out");
			sprintf(pngPath, "%s%s%s", questionPath, dir->d_name, "/png");

			//Get input file if it exists
			char **inArgs = NULL;
			if (access(inPath, F_OK) == 0) {
				printf("READING INPUT...\n");
				struct FileData in = readFile(inPath);
				//Handle reading input file

				//Count all the instances of '\n'
				int count = 1;
				for (int i = 0; i < in.len; i++) {
					if (in.data[i] == '\n') {
						count += 1;
					}
				}

				//Change arguments based on program mode
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
					inArgs[1] = "./code.py";
					i = 2;
				} else {
					inArgs[0] = "code";
				}

				//strtok all values to array
				char *token = strtok(in.data, "\n");
				while (token != NULL) {
					printf("%i - %s\n",i , token);
					inArgs[i] = strdup(token);
					i += 1;
					token = strtok(NULL, "\n");
				}
				inArgs[count] = NULL;
				free(in.data);

			} else if (PROGRAM_MODE == PYTHON) {
				//Set Python program default inputs if there's no in file
				inArgs = malloc(2 * sizeof(char *));
				inArgs[0] = "/usr/bin/python3";
				inArgs[1] = "code.py";
				inArgs[2] = NULL;
			}

			//Read png and output files
			struct FileData png = readFile(pngPath);	
			struct FileData out = readFile(outPath);
				
			//Free paths
			free(inPath);
			free(outPath);
			free(pngPath);

			//Change working directory to code folder to run the code
			char ret;
			char cwd[1024];
			chdir(dirPath);
    		getcwd(cwd, sizeof(cwd));
    		printf("Current working dir: %s\n", cwd);
			struct FileData output = testCode(&ret, lastAttempt, inArgs, out.data, png);

			//Change back, clear any loose files
			chdir("../..");
			nftw(dirPath, testUnlink_cb, 64, FTW_DEPTH | FTW_PHYS);

			//Free provided data
			free(inArgs);
			printf("DONE TEST\n");

			//Handle if the return is a failure
			if (ret > 0) {
				*completed = ret;
				closedir(d);
				unlink(codePath);
				free(codePath);
				nftw(dirPath, compileUnlink_cb, 64, FTW_DEPTH | FTW_PHYS);

				//Return last attempt
				if (lastAttempt == 1 && ret != 3) {
					printf("RETURNING LAST ATTEMPT %i\n", ret);
					struct FileData *r = malloc(2 * sizeof(struct FileData));
					if (ret == 1) {
						r[0] = out;
					} else {
						r[0] = png;
					}
					r[1] = output;
					return r;
				}

				free(out.data);
				free(png.data);
				return (struct FileData []){(struct FileData){0, NULL}, (struct FileData){0, NULL}};
			}
			free(out.data);
			free(png.data);
		}

		*completed = 0; //Set completed to true

		closedir(d);
		unlink(codePath);
		free(codePath);
		nftw(dirPath, compileUnlink_cb, 64, FTW_DEPTH | FTW_PHYS); //Clean up file
		return (struct FileData []){(struct FileData){0, NULL}, (struct FileData){0, NULL}};
}