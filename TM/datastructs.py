import fileinput
import sys

# globals (should we have a file for this?)
num_questions = 3
DB_filepath = "test_TM_database_v2.txt"
QB_portnums = [1401,2002] # ignore what this means for now TBC


# gets questions info for this student from the QBs
def get_questions(username):
    seed = username
    questions = []
    types = []
    choices = []
    
    for QB in range(len(QB_portnums)):
        start, end = indexes_in_qb(QB)
        line_questions = QB_get_questions(QB_portnums[QB], end - start + 1, seed).split("\;")
        questions.extend(line_questions[0].split("\,"))
        types.extend(line_questions[1].split(","))
        choices.extend([choice.split("\:") for choice in line_questions[2].split("\,")])
        
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
def check_answer(username, question_index, student_answer, attempts):
    # calls the relevant QB and asks to check
    is_last_attempt = attempts == 2
    portnum, q_index = q_to_qb(question_index)

    return QB_check_question(username, portnum, q_index, student_answer, is_last_attempt)


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
    low = i * num_questions // len(QB_portnums)
    high = (i + 1) * num_questions // len(QB_portnums)
    return range(num_questions)[low:high][0],  range(num_questions)[low:high][-1]

# gets the qb that a given question index was taken from
def q_to_qb(q):
    for i in range(len(QB_portnums)):
        start, end = indexes_in_qb(i)
        if q >= start and q <=end:
            return i, q - start
    return
            

# PLACEHOLDER func that gets a random sequence of questions and their info
def QB_get_questions(portnum, num_qs, seed):
    # return as a string of lists
    # of questions, types, choices
    if (portnum == 1401):
        questions = (
            "Write a hello world program in C.\;"
            "c\;"
            ""
        )
    else: #portnum is just the other one, 2002 (for now)
        questions = (
            "What is Chris' favourite colour?\,Which is not a network layer routing algorithm?\;"
            "m,m\;"
            "red\:green\:blue\:black\:yellow\,flooding\:leaky bucket\:link state\:distance vector"
        )
    return questions

# PLACEHOLDER func that checks an answer with a QB
def QB_check_question(username, portnum, q_index, is_last_attempt, student_answer):    
    if is_last_attempt:
        # return in form is_correct, student_output, sample_output
        return False#, student_answer, "placeholder checked_output"
    else:
        return False
