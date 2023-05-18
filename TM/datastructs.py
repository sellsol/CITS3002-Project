import fileinput
import sys
import hashlib
from config import *
from TaskManager import *

# gets questions info for this student from the QBs
def get_questions(username):
    seed = int(hashlib.sha256(username.encode('utf-8')).hexdigest(), 16) % 10**8
    questions = []
    types = []
    choices = []
    
    for QB in range(num_qbs):
        start, end = indexes_in_qb(QB)
        ret_questions, ret_types, ret_choices = GenQuestionsRequest(QB, end - start + 1, seed)
        questions.extend(ret_questions)
        types.extend(ret_types)
        choices.extend(ret_choices)
        
    return questions, types, choices
  
# reads current answers info for this student from the TM_database
def get_answers(username):
    current_finished = 0
    current_marks = 0
    attempts = []
    marks = []
    
    try:
        # gets answers from DM if student already exists there
        found = False
        TM_database = open(DB_filepath, "r")
        for line in TM_database:
            line_answers = line.split(";")
            if line_answers[0] == username:
                attempts = list(map(int, line_answers[1].split(",")))
                marks = list(map(int, line_answers[2].split(",")))
                found = True
                break
        TM_database.close()
        # if student not yet in DB initialise to new test and record in DB
        if not found:
            attempts = [0] * num_questions
            marks = [0] * num_questions
            init_answers(username)
        else:
            # get progress numbers
            current_marks = sum(marks)
            for answer in range(num_questions):
                if (attempts[answer] == 3 and marks[answer] == 0) or marks[answer] > 0:
                    current_finished += 1
    except FileNotFoundError as err:
        # error handling for if the file could not be found
        print("Error: TM database '" + DB_filepath + "' could not be found.")
        raise err     
    
    return current_finished, current_marks, attempts, marks


# checks the given answers to a question and updates progress
def check_answer(username, question_index, student_answer, is_last_attempt):
    # calls the relevant QB and asks to check
    portnum, q_index = q_to_qb(question_index)
    seed = int(hashlib.sha256(username.encode('utf-8')).hexdigest(), 16) % 10**8

    #TBC update answers

    return CheckAnswerRequest(portnum, q_index, seed, is_last_attempt, student_answer)


# creates new student entry in the TM_database
def init_answers(username):
    try:
        TM_database = open(DB_filepath, "a")
        TM_database.write(
            username + ";"
            + ",".join(list(map(str, [0] * num_questions))) 
            + ";" + ",".join(list(map(str, [0] * num_questions))) + "\n"
        )
        TM_database.close()
    except FileNotFoundError as err:
        # error handling for if the file could not be found
        print("Error: TM database '" + DB_filepath + "' could not be found.")
        raise err   

# updates answers for this student in the TM_database
def update_answers(username, question_index, is_correct):
    try:
        # find relevant entry and replace it with current counts
        for line in fileinput.FileInput(DB_filepath, inplace=1):
            line_answers = line.split(";")
            if line_answers[0] == username:
                attempts = list(map(int, line_answers[1].split(",")))
                if is_correct:
                    marks = list(map(int, line_answers[2].split(",")))
                    marks[question_index] += 3 - attempts[question_index]
                attempts[question_index] += 1

                line = line.replace(line, username + ";"
                    + ",".join(list(map(str, attempts))) 
                    + ";" + ",".join(list(map(str, marks))) + "\n")
            sys.stdout.write(line)
    except FileNotFoundError as err:
        # error handling for if the file could not be found
        print("Error: TM database '" + DB_filepath + "' could not be found.")
        raise err   
    

# gets the start and end indexes of questions that were taken from a qb
def indexes_in_qb(i):
    low = i * num_questions // num_qbs
    high = (i + 1) * num_questions // num_qbs
    return range(num_questions)[low:high][0],  range(num_questions)[low:high][-1]

# gets the qb that a given question index was taken from
def q_to_qb(q):
    for i in range(num_qbs):
        start, end = indexes_in_qb(i)
        if q >= start and q <=end:
            return i, q - start
    return