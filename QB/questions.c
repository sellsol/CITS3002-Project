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
char **ques_types_ans; //pointer to question, types, and answers string
const int NUM_QAT_STRINGS = 3; //number of string in ques_types_ans string

char *PY_Q = "questionset_py.csv"; //qb ques for python
char *C_Q = "questionset_c.csv"; //qb ques for c

/*
* Returns a list of question ids
* The seed is the unique int that is associated with the string
*/
int question_ids(int*ids,char prog_lang,int num,int seed){

    if(prog_lang != 'c' && prog_lang != 'p'){
        perror("QB does not exist");
        return 1;
    }

    //generating question ids for 
    srand(seed);

    int i = 0;
    while (i<num){
        ids[i] = (rand() % (MAX_LINE_INDEX - MIN_LINE_INDEX + 1)) + MIN_LINE_INDEX;
        //unique set of numbers
        bool unique = true;
        int j = 0;
        while(j<i){
            if(ids[j]==ids[i])unique=false;
            ++j;
        }
        if(unique)++i;
    }
    return 0;
}


/*
* Returns a questions associated with the question/line id
*/
char *a_question(char*line,char *filename,int line_index){

    if(line_index>MAX_LINE_INDEX){
        perror("question does not exist\n");
        exit(1);
    }

    FILE *fp = fopen(filename,"r");

    int line_num = 0;

    char buffer[MAX_LINE_LEN];
    while(fgets(buffer,sizeof(buffer),fp) != NULL && line_index!=line_num){
        ++line_num;
        if(line_num==line_index) break;
    }

    fclose(fp);

    if(line_num != line_index){
        perror("Line does not exist in file\n");
        exit(1);
    }
    line = buffer;
    return line;
}

/*
* Returns a list with 3 strings: questions, types, answers
*/
int get_questions(char prog_lang,int seed, int num){

    ques_types_ans = realloc(ques_types_ans, (NUM_QAT_STRINGS + 1) * sizeof(ques_types_ans[0]));

    int*ids = malloc(num*sizeof(int));
    int get_ids = question_ids(ids,prog_lang,num,seed);

    char questions[BUFSIZ];
    char answers[BUFSIZ];
    char types[BUFSIZ];
    char*q_sep = "\\;"; //question seperator
    char *sep = ","; //general and ans seperator

    char *filename;
    if(prog_lang=='p'){
        filename = PY_Q;
    }else if(prog_lang == 'c'){
        filename = C_Q;
    }else{
        printf("%s\n","QB language is not supported");
        return(1);
    }

    int i = 0;
    char*line;
    char*ques;
    char*ans;
    while(i<num){
        int line_index = ids[i];
        line=a_question(line,filename,line_index);
        
        //question type
        line=strstr(line,sep);
        line = line + 1;
        char type = line[0];

        //question and answer strings
        line=strstr(line,sep);
        line = line+1;

        //question string if mcq
        if(type=='m'){
            ans = strstr(line,sep);
            int q_position = ans - line;
            ques = strndup(line,q_position);
            ans = ans+1;//removing ","
        }else{
            ans=" ";
            ques = line;
        }
        
        //debugging
        // printf("%c\n",type);
        // printf("%s\n",ques);
        // printf("%s\n",ans);

        //adding substrings to respective strings
        if(type=='c'){
            strncat(questions,ques,strlen(ques)-1);//removing trailing \n
        }else {strncat(questions,ques,strlen(ques));}
        if(strlen(ans)-1 == 0){
            strncat(answers,ans,strlen(ans));
        }else{strncat(answers,ans,strlen(ans)-1);}//removing trailing \n
        strncat(types,&type,1);

        if(i!=num-1){ //last entry does not have question seperator
            strncat(questions,q_sep,strlen(q_sep));
            strncat(answers,q_sep,strlen(q_sep));
            strncat(types,q_sep,strlen(q_sep));    
        }

        ++i;
    }

    ques_types_ans[0] = strdup(questions);
    ques_types_ans[1] = strdup(types);
    ques_types_ans[2] = strdup(answers);

    return 0;
}


// int main(){
//     int num = 10;
//     int seed = 12;
//     char prog_lang = 'p';
//     // char*filename;

//     // if(prog_lang=='p'){
//     //     filename = PY_Q;
//     // }else if(prog_lang == 'c'){
//     //     filename = C_Q;
//     // }else{
//     //     printf("%s\n","QB language is not supported");
//     //     return(1);
//     // }

//     // // testing question ids
//     // int *ids = malloc(num*sizeof(int));
//     // int get_ids = question_ids(ids,prog_lang,num,seed);
//     // // int i = 0;
//     // // while(i<num){
//     // //     printf("%d\n",ids[i]);
//     // //     ++i;
//     // // }

//     // int line_index = 3;
//     // //testing a question
//     // char *line;
//     // line = a_question(line,filename,line_index);
//     // // printf("%s",line);

//     //testing get_questions
//     int ques = get_questions(prog_lang,seed,num);
//     printf("%s\n",ques_types_ans[0]);
//     printf("%s\n",ques_types_ans[1]);
//     printf("%s\n",ques_types_ans[2]);

//     return 0;
// }