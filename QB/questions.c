/*
* Fetches the questions ids and questions from questions sets for a session
*/

/*Compile with:
    cc -o questions questions.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


/*
* Returns a list of question ids
* The seed is the unique int that is associated with the string
*/
int question_ids(int*ids,char prog_lang,int num,int seed){

    if(prog_lang != 'c' && prog_lang != 'p'){
        perror("QB does not exist");
        exit(1);
    }

    //generating question ids for 
    srand(seed);
    int upper = 11;//change according to number of available question in the respective QB
    int lower = 1;

    int i = 0;
    while (i<num){
        ids[i] = (rand() % (upper - lower + 1)) + lower;
        printf("%d",ids[i]);
        ++i;
    }

    return 0;
}


/*
* Returns a questions associated with the question/line id
*/
char*a_question(char prog_lang,int line_index){

    FILE *fp;
    int max_index = 11;

    if(prog_lang =='c'){
        fp = fopen("questionset_c.csv","r");
    }else if (prog_lang == 'p')
    {
        fp = fopen("questionset_py.csv","r");
    }else{
        perror("Unavailable QB\n");
        exit(1);
    }

    if(line_index>max_index){
        perror("This index does not exist\n");
        exit(1);
    }

    int*q_ind;
    char*q_type;

    char buffer[BUFSIZ];
    char line[BUFSIZ/2];
    while(fgets(buffer,sizeof(buffer),fp) != NULL){
        sscanf(buffer,"%d,%c,%s",q_ind,q_type,line);
        printf("%s",line);//debug
        if(*q_ind==line_index){
            break;
        }
    }

    fclose(fp);

    if(*q_ind != line_index){
        perror("Line does not exist in file\n");
        exit(1);
    }
    
    printf("%s",buffer);
    return buffer;
}

/*
* Returns a list with 3 strings: questions, types, answers
*
*/
char**get_questions(char prog_lang,int seed, int num){
    int*ids = malloc(num*sizeof(int));
    ids = question_ids(ids,prog_lang,num,seed);

    char questions[BUFSIZ];
    char answers[BUFSIZ/2];
    char types[num + (num-1)];
    char*q_sep = "\;"; //question seperator
    //char a_sep = ","; //answer seperator

    int i = 0;
    while (i<num){
        char*question;
        char*answer;
        char*type;
        int*q_id;
        char buffer[BUFSIZ/2];
        char*line = a_question(prog_lang,ids[i]);
        sscanf(line,"%i,%c,%s",q_id,type,buffer);

        if(*type=='m'){
            sscanf(buffer,"%s,%s",question,answer);
        }else if(*type == 'c'){
            sscanf(buffer,"%s",question);
            answer = " "; //for coding questions
        }else{
            perror("Invalid question type\n");
            exit(1);
        }
        strcat(question,q_sep);
        strcat(answer,q_sep);
        strcat(type,q_sep);

        strcat(types,type);
        strcat(questions,question);
        strcat(answers,answer);

        ++i;
    }
    realloc(questions,strlen(questions));
    realloc(answers,strlen(answers));
    realloc(types,strlen(types));

    char**ques_typ_ans;
    ques_typ_ans[0] = questions;
    ques_typ_ans[1] = types;
    ques_typ_ans[2] = answers;

    printf("%s",questions);
    printf("%s",answers);
    printf("%s",types);

    return ques_typ_ans;
}

/*testing*/
int main(){
    char **info = get_questions('p',12,5);
}