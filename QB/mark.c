/*
* Marks mcq questions
*/
#include "mark.h"

/*Compile with:
    cc -o mark questions.c pquestions.c mark.c -lm
*/

char *PY_A = "python/answerset_py.csv"; //qb ans for python
char *C_A = "c/answerset_c.csv"; //qb ans for c

/*
* returns (0,1,2) for (correct,error,..)
* modifies sending string to the serialised string to be sent to TM
* sending string: length//;<t/f>//;(if is last attempt)expected_answer//;student_answer
*/
int question_correct(char*sending_str,int64_t seed,int index,int lastAttempt,char*answer){
    int*ids = malloc((index+1)*sizeof(int));
    question_ids(ids,index+1,seed);
    char *sep = ","; //general and ans seperator

    //printf("%d\n",ids[index]);

    char *filename;
    char *ans_file;
    if(PROGRAM_MODE == PYTHON){
        filename = PY_Q;
        ans_file = PY_A;
    }else{
        filename = C_Q;
        ans_file = C_A;
    }

    //checking q_type
    char*line=a_question(line,filename,ids[index]);
    //printf("%s\n",line);
    line=strstr(line,sep);
    line = line + 1;
    char type = line[0];
    
    int is_correct = 0;

    char ans_line[BUFSIZ];
    char**output; //output[0] = expected output, output[1]=answer output

    if(type=='0'){ // coding questions
        int *completed;
        char*question;
        sprintf(question,"%d",ids[index]);

        output = compileCode(completed,question,answer,lastAttempt);
        if(*completed==1) is_correct = 0;
    }else{ // mcq questions
        FILE *fp = fopen(ans_file,"r");
        while(fgets(ans_line,sizeof(ans_line),fp) != NULL){
        int q_ind;
        char*cor_ans = strstr(ans_line,sep);
        char*ind = strndup(ans_line,cor_ans-ans_line);
        sscanf(ind,"%d",&q_ind);
        cor_ans++;
        cor_ans[strlen(cor_ans)-1]='\0';

        //printf("%s,%s\n",ind,cor_ans);
        output[0] = strdup(cor_ans);
        output[1] = strdup(answer);

        if(q_ind==ids[index]){
            if(strcmp(cor_ans,answer)==0) is_correct = 0;
            break;
        }
        fclose(fp);
        }        
    }

    //for serialisation
    char *str_sep = "\\;";
    char *sending_txt = malloc(strlen(output[0])+strlen(output[1])+strlen("t")+(strlen(str_sep)*4));  
    if(is_correct) strcat(sending_txt,"t");
    else strcat(sending_txt,"f");
    strcat(sending_txt,str_sep);
    if(lastAttempt==1){
        strcat(sending_txt,output[0]);
        strcat(sending_txt,str_sep);
        strcat(sending_txt,output[1]);
        strcat(sending_txt,str_sep);
    }
    
    /*
    * serialising
    */
    int length = strlen(sending_txt);
    int digits = floor(log10(length)+1)+1;
    char length_str[digits];
    sprintf(length_str,"%d",length);
    // printf("%s\n",length_str);
    
    sending_str = malloc((length+digits+strlen(str_sep)+(BUFSIZ/2))*sizeof(char));
    strncat(sending_str,length_str,digits);
    strncat(sending_str,str_sep,strlen(str_sep));
    strncat(sending_str,sending_txt,length);
    printf("\nSending string:\n%s\n",sending_str);

    return is_correct;
}

//debugging
// int main(int index,int seed, int last_attempt){
//     index = 5;
//     seed = 12;
//     last_attempt = 1;
    
//     char prog_lang = 'p';
//     char*answer="int";
//     char*sending_str;
//     bool correct = question_correct(sending_str,prog_lang,index,seed,last_attempt,answer);
//     // if(correct){
//     //     printf("%s\n","Correct!");
//     // }else{
//     //     printf("%s\n","Wrong :(");
//     // }
//     printf("\nSending string:\n%s\n",sending_str);
//     return 0;
//     free(sending_str);
// }
