#ifndef PQUESTIONS
#define PQUESTIONS

extern char* readTextFile(char path);
extern char** testCode(char *completed, char *path, char *expectedOut, char lastAttempt, char *expectedImage);
extern char** compileCode(char *completed, char *question, char *code, char lastAttempt);
#endif