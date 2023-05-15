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

char *PY_Q = "python/questionset_py.csv"; //qb ques for python
char *C_Q = "c/questionset_c.csv"; //qb ques for c

char prog_lang = 'p';//make seperately

/*
* Returns a list of question ids
* The seed is the unique int that is associated with the string
*/
int question_ids(int*ids, char prog_lang, char num, int64_t seed){

    if(prog_lang != 'c' && prog_lang != 'p'){
        perror("QB does not exist");
        return 1;
    }

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
char **ques_types_ans; //pointer to question, types, and answer 
int get_questions(char prog_lang,int seed, int num){

    ques_types_ans = realloc(ques_types_ans, (NUM_QAT_STRINGS + 1) * sizeof(ques_types_ans[0]));

    int*ids = malloc(num*sizeof(int));
    int get_ids = question_ids(ids,prog_lang,num,seed);

    char questions[BUFSIZ];
    char answers[BUFSIZ];
    char types[BUFSIZ];
    char*q_sep = "\\;"; //question seperator
    char *sep = ","; //general csv seperator

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

        //question string if not coding
        if(type!='0'){
            ans = strstr(line,sep);
            int q_position = ans - line;
            ques = strndup(line,q_position);
            ans = ans+1;//removing ","
        }else{
            ans="";
            ques = line;
        }
        
        //debugging
        // printf("%c\n",type);
        // printf("%s\n",ques);
        // printf("%s\n",ans);

        //adding substrings to respective strings
        if(type=='0'){
            strncat(questions,ques,strlen(ques)-1);//removing trailing \n for coding ques
        }else {strncat(questions,ques,strlen(ques));}
        if(type!= '0'){
            strncat(answers,ans,strlen(ans)-1); //removing trailing \n for mcq answers
        }
        strncat(types,&type,1);

        //adding seperators

        // if(i!=num-1){ //last entry does not have question seperator
        strncat(questions,q_sep,strlen(q_sep));
        if(type!='0'){ //no seperators together
            strncat(answers,q_sep,strlen(q_sep));
        } 
        ++i;
    }

    strncat(types,q_sep,strlen(q_sep));

    ques_types_ans[0] = strdup(questions);
    ques_types_ans[1] = strdup(types);
    ques_types_ans[2] = strdup(answers);

    // printf("%s\n",ques_types_ans[1]);
    // printf("%s\n",ques_types_ans[0]);
    // printf("%s\n",ques_types_ans[2]);

    return 0;
}

/*
int main(int num,int seed){
    num = 10;
    seed = 12;
    
    int ques = get_questions(prog_lang,seed,num);
    // printf("Questions:\n%s\n",ques_types_ans[0]);
    // printf("Types:\n%s\n",ques_types_ans[1]);
    // printf("Answers:\n%s\n",ques_types_ans[2]);

    char *sending_txt = malloc(strlen(ques_types_ans[0])+strlen(ques_types_ans[1])+strlen(ques_types_ans[2]));
    memcpy(sending_txt,ques_types_ans[0],strlen(ques_types_ans[0]));
    memcpy(sending_txt+strlen(ques_types_ans[0]),ques_types_ans[1],strlen(ques_types_ans[1]));
    memcpy(sending_txt+strlen(ques_types_ans[0])+strlen(ques_types_ans[1]),ques_types_ans[2],strlen(ques_types_ans[2]));

    // strncat(sending_txt,ques_types_ans[0],strlen(ques_types_ans[0]));
    // strncat(sending_txt,ques_types_ans[1],strlen(ques_types_ans[1]));
    // strncat(sending_txt,ques_types_ans[2],strlen(ques_types_ans[2]));
    //printf("\nSending text:\n%s\n",sending_txt);

    /*
    * serialising
    */
    /*
    int length = strlen(sending_txt);
    int digits = floor(log10(length)+1)+1;
    char length_str[digits];
    sprintf(length_str,"%d",length);
    // printf("%s\n",length_str);

    char *sep = "\\;";
    char sending_str[length+digits+strlen(sep)+(BUFSIZ/2)];
    strncat(sending_str,length_str,digits);
    strncat(sending_str,sep,strlen(sep));
    strncat(sending_str,sending_txt,length);
    printf("\nSending string:\n%s\n",sending_str);

    //do something with sending str
    return 0;
}*/
