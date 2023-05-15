#ifndef PQUESTIONS
#define PQUESTIONS

extern char* readTextFile(char path);
extern char** testCode(char *completed, char *path, char lastAttempt, char **in, char *expectedOut, char *expectedImage);
extern char** compileCode(char *completed, char *question, char *code, char lastAttempt);
#endif