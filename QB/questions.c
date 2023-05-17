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
int question_ids(int*ids, int num, uint32_t seed){

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
char *a_question(char *filename,int line_index) {

    if(line_index>MAX_LINE_INDEX){
        perror("question does not exist\n");
        exit(EXIT_FAILURE);
    }

    //Opens file we're reading
    FILE *fp = fopen(filename,"r");

    int line_num = 0;

    //Read lines until we get the one we're looking for
    char *buffer = malloc(MAX_LINE_LEN);
    while(fgets(buffer,MAX_LINE_LEN,fp) != NULL && line_index!=line_num){
        ++line_num;
        //if(line_num==line_index) break; //Why is this here when we have a condition up there??
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
char* get_questions(uint32_t seed, char num){

    char **ques_type_ans; //pointer to question, types, and answer 
    ques_type_ans = malloc((NUM_QAT_STRINGS + 1) * sizeof(char *));
    //ques_type_ans[0] refers to questions, [1] refers to types, [2] refers to answers
    ques_type_ans[0] = calloc(BUFSIZ, sizeof(char));
    ques_type_ans[1] = calloc(BUFSIZ, sizeof(char));
    ques_type_ans[2] = calloc(BUFSIZ, sizeof(char));
    

    int*ids = malloc(num*sizeof(int)); //Should not be doing this
    int get_ids = question_ids(ids,num,seed);


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
            strncat(ques_type_ans[0],ques,strlen(ques)-1);//removing trailing \n for coding ques
        }else {strncat(ques_type_ans[0],ques,strlen(ques));}
        if(type!= '0'){
            strncat(ques_type_ans[2],ans,strlen(ans)-1); //removing trailing \n for mcq answers
        }
        strncat(ques_type_ans[1],&type,1);

        //adding seperators

        // if(i!=num-1){ //last entry does not have question seperator
        strncat(ques_type_ans[0],q_sep,strlen(q_sep));
        if(type!='0'){ //no seperators together
            strncat(ques_type_ans[2],q_sep,strlen(q_sep));
        } 
        ++i;
    }

    strncat(ques_type_ans[1],q_sep,strlen(q_sep));

    // printf("%s\n",ques_type_ans[1]);
    // printf("%s\n",ques_type_ans[0]);
    // printf("%s\n",ques_type_ans[2]);

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

    /*
    int length = strlen(sending_txt);
    int digits = floor(log10(length)+1)+1;
    char length_str[digits];
    sprintf(length_str,"%d",length);
    // printf("%s\n",length_str);

    sep = "\\;";
    sending_str = (char*)realloc(sending_str,length+digits+strlen(sep)+(BUFSIZ/2));
    strncat(sending_str,length_str,digits);
    strncat(sending_str,sep,strlen(sep));
    strncat(sending_str,sending_txt,length);*/

    return sending_txt;
}

// int main(int num,int64_t seed){
//     num = 10;
//     seed = 12;
    
//     int ques = get_questions(seed,num);
//     // printf("Questions:\n%s\n",ques_types_ans[0]);
//     // printf("Types:\n%s\n",ques_types_ans[1]);
//     // printf("Answers:\n%s\n",ques_types_ans[2]);

//     char *sending_txt = malloc(strlen(ques_types_ans[0])+strlen(ques_types_ans[1])+strlen(ques_types_ans[2]));
//     memcpy(sending_txt,ques_types_ans[0],strlen(ques_types_ans[0]));
//     memcpy(sending_txt+strlen(ques_types_ans[0]),ques_types_ans[1],strlen(ques_types_ans[1]));
//     memcpy(sending_txt+strlen(ques_types_ans[0])+strlen(ques_types_ans[1]),ques_types_ans[2],strlen(ques_types_ans[2]));

//     // strncat(sending_txt,ques_types_ans[0],strlen(ques_types_ans[0]));
//     // strncat(sending_txt,ques_types_ans[1],strlen(ques_types_ans[1]));
//     // strncat(sending_txt,ques_types_ans[2],strlen(ques_types_ans[2]));
//     //printf("\nSending text:\n%s\n",sending_txt);

//     /*
//     * serialising
//     */
//     int length = strlen(sending_txt);
//     int digits = floor(log10(length)+1)+1;
//     char length_str[digits];
//     sprintf(length_str,"%d",length);
//     // printf("%s\n",length_str);

//     char *sep = "\\;";
//     char sending_str[length+digits+strlen(sep)+(BUFSIZ/2)];
//     strncat(sending_str,length_str,digits);
//     strncat(sending_str,sep,strlen(sep));
//     strncat(sending_str,sending_txt,length);
//     printf("\nSending string:\n%s\n",sending_str);

//     //do something with sending str
//     return 0;
// }
