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
struct FileData question_correct(uint64_t seed, char index,char lastAttempt,char* answer){
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
    char*line=a_question(filename,ids[index]); //Should free it
    printf("\tquestion info: %s", line);

    //Get question type (i.e number of options)
    //line=strstr(line,sep);
    //line = line + 1;
    //char type = line[0];
    char *num = strtok(line, sep);
    char *type = strtok(NULL, sep);
    
    char is_correct = 0;

    char ans_line[BUFSIZ];
    //char**output = malloc(2 * sizeof(char *)); //output[0] = expected output, output[1]=answer output
    struct FileData *outputs;

    if(strcmp(type, "c") == 0){ // coding questions
        printf("Marking question %i...\n", ids[index]);

        outputs = compileCode(&is_correct, num,answer,lastAttempt);

        /*struct FileData codeOutput = {sizeof(char) + 2 * sizeof(int) + outputs[0].len + outputs[1].len + 1, NULL};
        
        //Ugly way of piecing together a message, but there's no other way to do it
        //Format: char returnCode, int length of expectedOutput, expectedOutput, 
                //length of output, output
        codeOutput.data = calloc(codeOutput.len, sizeof(char));
        memcpy(codeOutput.data, &returnCode, sizeof(char));
        memcpy(codeOutput.data + sizeof(char), &outputs[0].len, sizeof(int));
        memcpy(codeOutput.data + sizeof(char) + sizeof(int), outputs[0].data, outputs[0].len);
        memcpy(codeOutput.data + sizeof(char) + sizeof(int) + outputs[0].len, &outputs[1].len, sizeof(int));
        memcpy(codeOutput.data + sizeof(char) + 2 * sizeof(int) + outputs[0].len, outputs[1].data, outputs[1].len);

        return codeOutput;*/

    } else { // mcq questions
        outputs = malloc(2 * sizeof(struct FileData));
        //Get expected answer
        char i = '0'; //To cycle through data
        outputs[1].data = strtok(NULL, sep);
        while (answer[0] + 1 > i) {
            i += 1;
            outputs[1].data = strtok(NULL, "\\,");
        }

        outputs[1].len = strlen(outputs[1].data);


        FILE *fp = fopen(ans_file,"r");
        //printf("Opened file %s\n", ans_file);
        while(fgets(ans_line,sizeof(ans_line),fp) != NULL){
            int q_ind;

            char *ind = strtok(ans_line, sep); //Get number from answer key line
            sscanf(ind,"%d",&q_ind);

            if(q_ind==ids[index]){
                outputs[0].data = strtok(NULL, "\n");
                outputs[0].len = strlen(outputs[0].data);
                //printf("%s - %s\n", outputs[0].data, outputs[1].data);
                if(strcmp(outputs[0].data, outputs[1].data) == 0) {
                    is_correct = 1;
                }
                break;
            }
            
        }   
        fclose(fp);     
    }

    //for serialisation
    //char *str_sep = "\\;";

    //Format??: char 't/f', expected output, string seperator, output
    char correct;
    if (is_correct == 0) {
        correct = 'f';
    } else if (is_correct == 1) {
        correct = 't';
    } else if (is_correct == 2) {
        correct = 'i';
    } else {
        correct = 'c';
    }

    struct FileData outputStr;

    if (lastAttempt == 1 && (is_correct == 0 || is_correct == 2)) {
        outputStr.len = sizeof(char) + 2 * sizeof(int) + outputs[0].len + outputs[1].len + 1;
        outputStr.data = calloc(outputStr.len, sizeof(char));

        //Ugly way of piecing together a message, but there's no other way to do it
        //Format: char returnCode, int length of expectedOutput, expectedOutput, 
                //length of output, output
        memcpy(outputStr.data, &correct, sizeof(char));
        memcpy(outputStr.data + sizeof(char), &outputs[0].len, sizeof(int));
        memcpy((unsigned char *)outputStr.data + sizeof(char) + sizeof(int), outputs[0].data, outputs[0].len);
        memcpy((unsigned char *)outputStr.data + sizeof(char) + sizeof(int) + outputs[0].len, &outputs[1].len, sizeof(int));
        memcpy((unsigned char *)outputStr.data + sizeof(char) + 2 * sizeof(int) + outputs[0].len, outputs[1].data, outputs[1].len);

        /*
        printf("\nSending long string:");
        for (int i = 0; i < outputStr.len; i++) {
            printf("%c|", outputStr.data[i]);
        }
        printf("\n");
        */
    } else {
        outputStr.len = sizeof(char);
        outputStr.data = malloc(sizeof(char));
        *outputStr.data = correct;
        printf("Sending reply: %c\n", *outputStr.data);
    }

    free(ids);
    //free(line); //TODO: right place?

    return outputStr;
}