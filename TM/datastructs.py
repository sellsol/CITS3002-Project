import fileinput
import sys

# globals (should we have a file for this?)
num_questions = 3
DB_filepath = "test_TM_database_v2.txt"
QB_portnums = [1401,2002] # ignore what this means for now TBC

# class for our database of student answers info
class TM_student:
    def __init__(self, username):
        self.username = username        
        self.current_finished = 0
        self.current_marks = 0
        
        self.questions = []
        self.types = []
        self.choices = []
        self.attempts = []
        self.marks = []
        
    # gets questions info for this student from the QBs
    def get_questions(self):
        seed = self.username
        for QB in range(len(QB_portnums)):
            start, end = indexes_in_qb(QB)
            line_questions = QB_get_questions(QB_portnums[QB], end - start + 1, seed).split("\;")
            self.questions.extend(line_questions[0].split("\,"))
            self.types.extend(line_questions[1].split(","))
            self.choices.extend([choice.split("\:") for choice in line_questions[2].split("\,")])

    # reads current answers info for this student from the TM_database
    def get_answers(self):
        try:
            # gets answers from DM if student already exists there
            found = False
            TM_database = open(DB_filepath, "r")
            for line in TM_database:
                line_answers = line.split(";")
                if line_answers[0] == self.username:
                    self.attempts = list(map(int, line_answers[1].split(",")))
                    self.marks = list(map(int, line_answers[2].split(",")))
                    found = True
                    break
            TM_database.close()
            # if student not yet in DB initialise to new test and record in DB
            if not found:
                self.attempts = [0] * num_questions
                self.marks = [0] * num_questions
                self.init_answers()
                
            # get progress numbers
            self.current_marks = sum(self.marks)
            for answer in range(num_questions):
                if self.is_finished(answer):
                    self.current_finished += 1
        except FileNotFoundError as err:
            # error handling for if the file could not be found
            print("Error: TM database '" + DB_filepath + "' could not be found.")
            raise err        
    
    
    # checks if a question at a certain page index is already finished
    def is_finished(self, answer_index):
        if self.attempts[answer_index] == 3 and self.marks[answer_index] == 0:
            return True
        elif self.marks[answer_index] > 0:
            return True
        else:
            return False
        
            
    # checks the given answers to a question and updates progress
    def check_answer(self, question_index, student_answer):
        print(question_index)
        print(student_answer)
        print(self.attempts)
        # calls the relevant QB and asks to check
        is_last_attempt = self.attempts[question_index] == 2
        portnum, q_index = q_to_qb(question_index)
            
        result = QB_check_question(portnum, q_index, student_answer, is_last_attempt)
 
        # update depending on if correct or not
        if result[0]:
            self.marks[question_index] += 3 - self.attempts[question_index]
            self.attempts[question_index] += 1
            self.current_marks += self.marks[question_index]
            self.current_finished += 1
        else:
            self.attempts[question_index] += 1
            if is_last_attempt:
                self.current_finished += 1
    
    # creates new student entry in the TM_database
    def init_answers(self):
        try:
            TM_database = open(DB_filepath, "a")
            TM_database.write(
                self.username + ";"
                + ",".join(list(map(str, self.attempts))) 
                + ";" + ",".join(list(map(str, self.marks))) + "\n"
            )
            TM_database.close()
        except FileNotFoundError as err:
            # error handling for if the file could not be found
            print("Error: TM database '" + DB_filepath + "' could not be found.")
            raise err   
    
    # updates answers for this student in the TM_database
    def update_answers(self):
        try:
            # find relevant entry and replace it with current counts
            for line in fileinput.FileInput(DB_filepath, inplace=1):
                if line.split(";")[0] == self.username:
                    line = line.replace(line, self.username + ";"
                        + ",".join(list(map(str, self.attempts))) 
                        + ";" + ",".join(list(map(str, self.marks))) + "\n")
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
def QB_check_question(portnum, q_index, is_last_attempt, student_answer):    
    if is_last_attempt:
        # return in form is_correct, student_output, sample_output
        return False, student_answer, "placeholder checked_output"
    else:
        return False

# TEMPORARY test func to print all the info in a TM_student object
def TEST_student_info(student):
    # general info
    print("username: " + student.username)
    print("current finished: " + str(student.current_finished) + "/" + str(num_questions))
    print("current_marks: " + str(student.current_marks) + "/" + str(num_questions * 3))
    print()
    
    # for each page
    for page_index in range(num_questions):        
        print("question " + str(page_index + 1) + ": " + student.questions[page_index])
        if student.types[page_index] == 0:
            print("\t[enter your code here]")
        else:
            i = 1
            for choice in student.choices[page_index]:
                print("\t" + str(i) + ". " + choice)
                i += 1
                
        if student.is_finished(page_index):
            print("\t[question is finished, no more submissions]")
        
        print("attempts: " + str(student.attempts[page_index]))
        print("marks: " + str(student.marks[page_index]))
        
        print()

# TEMPORARY main func for testing
def main(input_username):
    # login sequence
    #normally authentication here first to get username
    student = TM_student(input_username)
    student.get_questions()
    student.get_answers()
    
    # test display
    TEST_student_info(student)
    
    # entering answers
    student.check_answer(2, 3)
    student.update_answers()
    
    # test display again
    TEST_student_info(student)

import sys
if __name__ == "__main__":
    main(sys.argv[1])
    
