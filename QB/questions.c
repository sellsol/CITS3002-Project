/*
* Fetches the questions ids and questions from questions sets for a session
*
*/

/*Compile with:
    cc -o questions questions.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
* Returns a list of mcq question ids
*/
int*questions_mcq(char prog_lang, int num){
    int*ids=malloc(num*sizeof(int));

    FILE *py_fp = fopen("questionset_py.csv","r");
    FILE *c_fp = fopen("questionset_c.csv","r");

    char line[500]; //no line is >499 chars
    int i = 0;

    //for both programming languages
    FILE *fp;
    if(prog_lang = 'a'){
        while (i<num)
        {
            if(rand()%2==1) fp = py_fp;
            else fp = c_fp;

            int *q_id;
            char *q_type;
            char buff[200];
            while(fgets(line, sizeof(line), fp) != NULL) {
                sscanf(line,"%i,%c,%s",q_id,q_type,buff);
                if(*q_type=='m'){
                    ids[i]=*q_id;
                    ++i;
                }
            }
            
        }
    }
    //for c
    else if (prog_lang = 'c')
    {
        int *q_id;
        char *q_type;
        char buff[200];
        while(fgets(line, sizeof(line), c_fp) != NULL && i<num) {
            sscanf(line,"%i,%c,%s",q_id,q_type,buff);
            if(*q_type=='m'){
                ids[i]=*q_id;
                ++i;
            }
        }
    }
    //for python
    else if (prog_lang = 'p')
    {
        int *q_id;
        char *q_type;
        char buff[200];
        while(fgets(line, sizeof(line), py_fp) != NULL && i<num) {
            sscanf(line,"%i,%c,%s",q_id,q_type,buff);
            printf("%s",buff);
            if(*q_type=='m'){
                ids[i]=*q_id;
                ++i;
            }
        }
    }
    else{
        perror("No mcq's available for this programming language\n");
        exit(1);
    }
    
    return ids;
}

/*
* Returns a list of coding question ids
*/
int*questions_cod(char prog_lang, int num){
    int*ids=malloc(num*sizeof(int));

    FILE *py_fp = fopen("questionset_py.csv","r");
    FILE *c_fp = fopen("questionset_c.csv","r");

    char line[500]; //no line is >499 chars
    int i = 0;

    //for both programming languages
    FILE *fp;
    if(prog_lang = 'a'){
        while (i<num)
        {
            if(rand()%2==1) fp = py_fp;
            else fp = c_fp;

            int *q_id;
            char *q_type;
            char buff[200];
            while(fgets(line, sizeof(line), fp) != NULL) {
                sscanf(line,"%i,%c,%s",q_id,q_type,buff);
                if(*q_type=='c'){
                    ids[i]=*q_id;
                    ++i;
                }
            }
            
        }
    }
    //for c
    else if (prog_lang = 'c')
    {
        int *q_id;
        char *q_type;
        char buff[200];
        while(fgets(line, sizeof(line), c_fp) != NULL && i<num) {
            sscanf(line,"%i,%c,%s",q_id,q_type,buff);
            if(*q_type=='c'){
                ids[i]=*q_id;
                ++i;
            }
        }
    }
    //for python
    else if (prog_lang = 'p')
    {
        int *q_id;
        char *q_type;
        char buff[200];
        while(fgets(line, sizeof(line), py_fp) != NULL && i<num) {
            sscanf(line,"%i,%c,%s",q_id,q_type,buff);
            if(*q_type=='c'){
                ids[i]=*q_id;
                ++i;
            }
        }
    }
    else{
        perror("No coding questions available for this programming language\n");
        exit(1);
    }
    
    return ids;
}

/*
* Returns a list of question ids
*/
int*question_ids(char prog_lang,int mcq, int cod, bool randomise){
    int num = mcq+cod;
    int*ids = malloc(num*sizeof(int));
    int*cod_ids = malloc(cod*sizeof(int));

    ids = questions_mcq(prog_lang,mcq);
    cod_ids = questions_cod(prog_lang,cod);

    for (int i=0;i<cod;i++){
        ids[mcq+i]=cod_ids[i];
    }

    if(randomise){
        // Ben Pfaff's 04 April 2004 list shuffling algorithm
        // https://benpfaff.org/writings/clc/shuffle.html
        size_t i;
        for (i = 0; i < num - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (num - i) + 1);
          int t = ids[j];
          ids[j] = ids[i];
          ids[i] = t;
        }
    }
    return ids;
}

int main(){
    int mcq = 3;
    int cod = 2;
    char prog_lang = 'p';
    bool random = true;

    int num = mcq + cod;//for testing only
    int*ques_ids;
    ques_ids = malloc(num*sizeof(int));
    ques_ids = question_ids(prog_lang,mcq,cod,random);

    for (int i=0;i<num;i++){
        printf("%i",ques_ids[i]);
    }
}