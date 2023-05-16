#ifndef PQUESTIONS
#define PQUESTIONS

extern const char *c_path;
extern const char *py_path;
extern const char* q_path;

struct FileData {
	int len;
	char *data;
};

extern struct FileData readFile(char *path);
extern struct FileData testCode(char *completed, char lastAttempt, char *in[], char *expectedOut, struct FileData expectedImage);
extern struct FileData* compileCode(char *completed, char *question, char *code, char lastAttempt);
#endif