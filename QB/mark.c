/*
* Marks mcq questions
*/
#include "mark.h"
#include "pquestions.h"

/*Compile with:
    cc -o mark questions.c pquestions.c mark.c -lm
*/

char *PY_A = "./questions/python/answerset_py.csv"; //qb ans for python
char *C_A = "./questions/c/answerset_c.csv"; //qb ans for c

/*
* returns (0,1,2) for (correct,error,server error)
* modifies sending string to the serialised string to be sent to TM
* sending string: length//;<t/f>//;(if is last attempt)expected_answer//;student_answer
*/
struct FileData question_correct(uint64_t seed, char index,char lastAttempt,char*answer){
    int *ids = question_ids(index+1,seed);
    char *sep = ","; //general and ans seperator

    //printf("%d\n",ids[index]);

    //Should set these as globals
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
    char*line=a_question(filename,ids[index]); //SHould free it
    printf("%s\n", line);

    //Get question type (i.e number of options)
    //line=strstr(line,sep);
    //line = line + 1;
    //char type = line[0];
    char *num = strtok(line, sep);
    char *type = strtok(NULL, sep);
    
    int is_correct = 0;

    char ans_line[BUFSIZ];
    char**output = malloc(2 * sizeof(char *)); //output[0] = expected output, output[1]=answer output

    if(strcmp(type, "c") == 0){ // coding questions
        printf("Marking question %i...\n", ids[index]);

        free(ids);

        char returnCode;
        struct FileData *outputs = compileCode(&returnCode,num,answer,lastAttempt);

        struct FileData codeOutput = {sizeof(char) + 2 * sizeof(int) + outputs[0].len + outputs[1].len + 1, NULL};
        
        //Ugly way of piecing together a message, but there's no other way to do it
        //Format: char returnCode, int length of expectedOutput, expectedOutput, 
                //length of output, output
        codeOutput.data = malloc(codeOutput.len);
        memcpy(codeOutput.data, &returnCode, sizeof(char));
        memcpy(codeOutput.data + sizeof(char), &outputs[0].len, sizeof(int));
        memcpy(codeOutput.data + sizeof(char) + sizeof(int), outputs[0].data, outputs[0].len);
        memcpy(codeOutput.data + sizeof(char) + sizeof(int) + outputs[0].len, &outputs[1].len, sizeof(int));
        memcpy(codeOutput.data + sizeof(char) + 2 * sizeof(int) + outputs[0].len, outputs[1].data, outputs[1].len);

        return codeOutput;

    }else{ // mcq questions
        FILE *fp = fopen(ans_file,"r");
        printf("Opened file %s\n", ans_file);
        while(fgets(ans_line,sizeof(ans_line),fp) != NULL){
            int q_ind;
            //char*cor_ans = strstr(ans_line,sep);
            //char*ind = strndup(ans_line,cor_ans-ans_line);
            char *ind = strtok(ans_line, sep);
            char *cor_ans = strtok(NULL, sep);
            sscanf(ind,"%d",&q_ind);
            //cor_ans++;
            //cor_ans[strlen(cor_ans)-1]='\0';

            //printf("%s,%s\n",ind,cor_ans);
            printf("%s\n", ans_line);
            printf("%s\n", cor_ans);
            printf("%s\n", answer);

            if(q_ind==ids[index]){
                printf("SOMETHING\n");
                output[0] = cor_ans;
                output[1] = answer;
                printf("%s - %s\n", cor_ans, answer);
                if(strcmp(output[0], output[1]) == 0) {
                    is_correct = 1;
                }
                break;
            }
            
        }   
        fclose(fp);     
    }

    //for serialisation
    char *str_sep = "\\;";

    //Format??: char 't/f', expected output, string seperator, output
    char correct = (is_correct) ? 't' : 'f';
    struct FileData mChoiceOutput;
    if (lastAttempt == 1) {
        mChoiceOutput.len = strlen("t") + strlen(output[0]) + strlen(output[1]) + strlen(str_sep);
        mChoiceOutput.data = calloc(mChoiceOutput.len, sizeof(char));
        sprintf(mChoiceOutput.data, "%c%s%s%s%s", correct, output[0], str_sep, output[1]);
        printf("\nSending string:\n%s\n", mChoiceOutput.data);
    } else {
        mChoiceOutput.len = sizeof(char);
        mChoiceOutput.data = malloc(sizeof(char));
        *mChoiceOutput.data = correct;
        printf("\nSending string:\n%c\n", *mChoiceOutput.data);
    }

    free(ids);

    return mChoiceOutput;
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
