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
#include <string.h>

/*
* Returns a list of mcq question ids
*/
int*questions_mcq(char prog_lang, int mcq, int cod){
    int ids[mcq+cod];

    FILE *fp = fopen("questionset.csv","r");

    char line[500]; //no line is >499 chars
    int i = 0;
    int j = 0;

    //for both programming languages

    while(i<mcq && j<cod){
        q_id = rand()%17;
    }

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
        
    }
    else{
        perror("No mcq's available for this programming language\n");
        exit(1);
    }

    fclose(fp);
    return ids;
}

/*
* Returns a list of question ids
* The seed is the unique int that is associated with the string
*/
int*question_ids(char prog_lang,int num,int seed){

    int ids[num];
    char *filename;

    if(prog_lang != 'c' || prog_lang != 'p'){
        error("QB does not exist");
        exit(1);
    }

    //mitigate to different files or duplicate for each QB
    if(prog_lang == 'c'){
        filename = strndup("questionset_c.csv",18);
    }
    else if(prog_lang == 'p'){
        filename = strndup("questionset_py.csv",20);
    }

    //generating question ids for 
    srand(seed);
    FILE *fp = fopen(filename,"r");
    int upper = 10;//change according to number of available question in the respective QB
    int lower = 0;

    int i = 0;
    while (i<num){
        ids[i] = (rand() % (upper - lower + 1)) + lower;
        ++i;
    }

    return ids;
}


/*
* Returns a list of questions associated with question ids
* The seed is the unique int that is associated with the string
*/
char**questions(int*ids){
    while(fgets(line, sizeof(line), py_fp) != NULL && i<num) {
            sscanf(line,"%i,%c,%s",q_id,q_type,buff);
            printf("%s",buff);
            if(*q_type=='m'){
                ids[i]=*q_id;
                ++i;
            }
        }
}


int main(){
    
}