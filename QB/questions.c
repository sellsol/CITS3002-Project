/*
* Fetches the questions ids and questions from questions sets for a session
*/

/*Compile with:
    cc -o questions questions.c -lm
*/

#include "questions.h"

const int MAX_LINE_INDEX = 11; //update according to number of questions in QB's
const int MIN_LINE_INDEX = 1; //lowest line number in QB
const int MAX_LINE_LEN = 500; //max length of a line in question set
const int NUM_QAT_STRINGS = 3; //number of string in ques_types_ans string

char *PY_Q = "./questions/python/questionset_py.csv"; //qb ques for python
char *C_Q = "./questions/c/questionset_c.csv"; //qb ques for c


/*
* Returns a list of question ids
* The seed is the unique int that is associated with the string
*/
int* question_ids(int num, uint64_t seed){

    int *ids = malloc(num * sizeof(int));
    //generating question ids for 
    srand(seed);

    char i = 0;
    while (i<num){
        ids[i] = (rand() % (MAX_LINE_INDEX - MIN_LINE_INDEX + 1)) + MIN_LINE_INDEX;
        //unique set of numbers
        bool unique = true;
        char j = 0;
        while(j<i){
            if(ids[j]==ids[i])unique=false;
            ++j;
        }
        if(unique)++i;
    }

    return ids;
}


/*
* Returns a questions associated with the question/line id
*/
char *a_question(char *filename,int line_index) {

    if(line_index>MAX_LINE_INDEX){
        perror("question does not exist\n");
        exit(EXIT_FAILURE);
    }

    //Opens file we're reading
    FILE *fp = fopen(filename,"r");

    int line_num = 1;

    //Read lines until we get the one we're looking for
    char *buffer = malloc(MAX_LINE_LEN);
    while(fgets(buffer,MAX_LINE_LEN,fp) != NULL && line_index!=line_num){
        ++line_num;
    }

    fclose(fp);

    if(line_num != line_index){
        perror("Line does not exist in file\n");
        exit(1);
    }
    return buffer;
}

/*
* sends questions string back
*/
char* get_questions(uint64_t seed, char num){

    char **ques_type_ans; //pointer to question, types, and answer 
    ques_type_ans = malloc((NUM_QAT_STRINGS + 1) * sizeof(char *));
    //ques_type_ans[0] refers to questions, [1] refers to types, [2] refers to answers
    ques_type_ans[0] = calloc(BUFSIZ, sizeof(char));
    ques_type_ans[1] = calloc(BUFSIZ, sizeof(char));
    ques_type_ans[2] = calloc(BUFSIZ, sizeof(char));
    

    int *ids = question_ids(num,seed);


    char*q_sep = "\\;"; //question seperator
    char *sep = ","; //general csv seperator

    char *filename;
    if(PROGRAM_MODE==PYTHON){
        filename = PY_Q;
    }else{
        filename = C_Q;
    }
    
    int i = 0;
    char*line;
    char*ques;
    char*ans;
    while(i<num){
        int line_index = ids[i];
        line=a_question(filename,line_index);
        
        //question type
        line=strstr(line,sep);
        line = line + 1;
        char type = line[0];

        //question and answer strings
        line=strstr(line,sep);
        line = line+1;

        //question string if not coding
        if(type!='c'){
            ans = strstr(line,sep);
            int q_position = ans - line;
            ques = strndup(line,q_position);
            ans = ans+1;//removing ","
        }else{
            ans="";
            ques = line;
        }
        
        //adding question info to relevant strings, adding separators
        if(type == 'c') strncat(ques_type_ans[0],ques,strlen(ques)-1);//removing trailing \n for coding ques
        else strncat(ques_type_ans[0],ques,strlen(ques));
        strncat(ques_type_ans[0],q_sep,strlen(q_sep));

        if (type!= 'c') strncat(ques_type_ans[2],ans,strlen(ans)-1); //removing trailing \n for mcq answers
        strncat(ques_type_ans[2],q_sep,strlen(q_sep));

        strncat(ques_type_ans[1], &type,1);
        strncat(ques_type_ans[1], q_sep, strlen(q_sep));

        ++i;
    }

    /*
    * Serialising
    */
    char *sending_txt = calloc(strlen(ques_type_ans[0])+strlen(ques_type_ans[1])+strlen(ques_type_ans[2]), sizeof(char));
    memcpy(sending_txt,ques_type_ans[0],strlen(ques_type_ans[0]));
    memcpy(sending_txt+strlen(ques_type_ans[0]),ques_type_ans[1],strlen(ques_type_ans[1]));
    memcpy(sending_txt+strlen(ques_type_ans[0])+strlen(ques_type_ans[1]),ques_type_ans[2],strlen(ques_type_ans[2]));
    strcat(sending_txt, "\0");

    free(ques_type_ans[0]);
    free(ques_type_ans[1]);
    free(ques_type_ans[2]);
    //free(ques_type_ans); //TODO: intesting
    //free(ids); //TODO: intesting
    //free(line); //TODO: intesting (right place?) - not working, invalid free

    return sending_txt;
}