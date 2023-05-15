#ifndef PQUESTIONS
#define PQUESTIONS

extern char* readTextFile(char path);
extern char** testCode(int *completed, char *path, char *expectedOut, char lastAttempt, char *expectedImage);
extern char** compileCode(int* completed, char* question, char* code, char lastAttempt);
#endif