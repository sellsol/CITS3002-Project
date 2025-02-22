#ifndef QUESTIONS
#define QUESTIONS

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "mode.h"

//globals.c
extern const int MAX_LINE_INDEX; //update according to number of questions in QB's
extern const int MIN_LINE_INDEX; //lowest line number in QB
extern const int MAX_LINE_LEN; //max length of a line in question set
extern char **ques_types_ans; //pointer to question, types, and answers string
extern const int NUM_QAT_STRINGS; //number of string in ques_types_ans string

extern char *PY_Q; //ques for python
extern char *C_Q; //ques for c

//questions.c
extern int *question_ids(int,uint64_t); //a list of question ids
extern char *a_question(char*,int); //Returns pointer to a questions associated with the question/line id
extern char *get_questions(uint64_t,char); //a list with 3 strings: questions, types, answers
#endif
