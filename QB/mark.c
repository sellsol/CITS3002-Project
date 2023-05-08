/*
* Marks mcq questions
*/
#include "questions.h"

/*Compile with:
    cc -o mark questions.c mark.c -lm
*/

char *PY_A = "answerset_py.csv"; //qb ans for python
char *C_A = "answerset_c.csv"; //qb ans for c

/*
* Returns true or false if answer is correct and false otherwise
*/
bool mcq_correct(char prog_lang,int index,int seed,char*answer){
    int*ids = malloc(index*sizeof(int));
    question_ids(ids,prog_lang,index+1,seed);
    char *sep = ","; //general and ans seperator

    //printf("%d\n",ids[index]);

    char *filename;
    char *ans_file;
    if(prog_lang=='p'){
        filename = PY_Q;
        ans_file = PY_A;
    }else if(prog_lang == 'c'){
        filename = C_Q;
        ans_file = C_A;
    }else{
        printf("%s\n","QB language is not supported");
        return(1);
    }

    //checking q_type
    char*line=a_question(line,filename,ids[index]);
    //printf("%s\n",line);
    line=strstr(line,sep);
    line = line + 1;
    char type = line[0];
    if(type!='m'){
        perror("not mcq question\n");
        exit(1);
    }

    char ans_line[BUFSIZ];

    FILE *fp = fopen(ans_file,"r");
    bool equal = false;

    while(fgets(ans_line,sizeof(ans_line),fp) != NULL){

        int q_ind;
        char*cor_ans = strstr(ans_line,sep);
        char*ind = strndup(ans_line,cor_ans-ans_line);
        sscanf(ind,"%d",&q_ind);
        cor_ans++;
        cor_ans[strlen(cor_ans)-1]='\0';

        //printf("%s,%s\n",ind,cor_ans);

        if(q_ind==ids[index] && strcmp(cor_ans,answer)==0){
            equal = true;
            break;
        }
    }
    fclose(fp);
    return equal; //re-direct to prog checking if type=='c'
}

// int main(){
//     int index = 5;
//     int seed = 12;
//     char prog_lang = 'p';
//     char*answer="int";
//     bool correct = mcq_correct(prog_lang,index,seed,answer);
//     if(correct){
//         printf("%s\n","Correct!");
//     }else{
//         printf("%s\n","Wrong :(");
//     }
// }