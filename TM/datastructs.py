import fileinput
import sys

DB_filepath = "test_TM_database.txt"
num_questions = 3

# class for our database of student answers info
class TM_student:
    def __init__(self, username):
        self.username = username        
        self.current_finished = 0
        self.current_marks = 0
        self.questions = []
        
    # checks if a student is already in the TM_database
    def is_new_student(self):
        TM_database = open(DB_filepath, "r")
        
        for line in TM_database:
            line_username = line.split(",")[0]
            if line_username == self.username:
                return False
            
            for _ in range(num_questions - 1):
                next(TM_database)
                
        TM_database.close()
        return True         
        
    # reads current answers and their question info for this student from the TM_database
    def read_answers(self):
        if self.is_new_student():            
            # get info for random sequence of questions and initialise each question                
            questions_info = QB_get_questions_new()
            for q_info in questions_info:
                question = TM_question(q_info[0])
                question.question = q_info[1]
                question.num_choices = q_info[2]
                question.choices = q_info[3:]
                self.questions.append(question)
            
        else:
            TM_database = open(DB_filepath, "r")
            line_answer = TM_database.readline().strip().split(",", 4)
            
            # skips through the file until a matching username is found
            while line_answer[0] != self.username:
                line_answer = TM_database.readline().strip().split(",", 4)

            # gets the succeeding lines in the file (same student) and read into student answers
            question_ids = []
            for _ in range(num_questions):
                # store data on student answers
                question_ids.append(line_answer[1])
                question = TM_question(line_answer[1])
                question.attempts = int(line_answer[2])
                question.marks = int(line_answer[3])
                
                # if student already has 3 incorrect attempts, read in the student's last output and update progress
                if (question.is_finished_incorrect()):
                    question.student_output = line_answer[4]
                    question.sample_output = "Placeholder sample output" # TO REPLACE - get sample output from QB
                    self.current_finished += 1
                # if student already answered the question correctly, update progress
                if (question.marks > 0):
                    self.current_finished += 1
                    self.current_marks += question.marks
                
                self.questions.append(question)
                line_answer = TM_database.readline().strip().split(",", 4)
            
            TM_database.close()
            
            # get info corresponding to the question ids and store into questions array
            questions_info = QB_get_questions(question_ids)
            for i in range(num_questions):
                q_info = questions_info[i]
                question = self.questions[i]
                question.question = q_info[1]
                question.num_choices = q_info[2]
                question.choices = q_info[3:]
            
    # checks the given answers to a question and updates progress
    def check_answer(self, question_index, student_answer):
        if self.questions[question_index].attempts == 2:
            is_last_attempt = True
        else:
            is_last_attempt = False
            
        result = QB_check_question(self.questions[question_index].question_id, student_answer, is_last_attempt)
 
        if result[0]:
            self.questions[question_index].correct_attempt()
            self.current_finished += 1
            self.current_marks += self.questions[question_index].marks
        else:
            self.questions[question_index].incorrect_attempt(result[1], result[2])
            if is_last_attempt:
                self.current_finished += 1
    
    # writes current answers for this student to the TM_database
    def write_answers(self):
        if self.is_new_student():
            TM_database = open(DB_filepath, "a")
            for i in range(num_questions):
                question = self.questions[i]
                TM_database.write("\n" + self.username + "," + str(question.question_id) 
                    + "," + str(question.attempts) + "," + str(question.marks))
                if question.is_finished_incorrect():
                    TM_database.write(question.student_output)
            TM_database.close()
        else:
            i = 0
            for line in fileinput.input(DB_filepath, inplace=1):
                # skips through the file until a matching username is found
                if line.split(",")[0] == self.username:
                    # if third incorrect attempt, also write student output
                    question = self.questions[i]
                    if question.is_finished_incorrect():
                        line = line.replace(line, self.username + "," + str(question.question_id)
                            + "," + str(question.attempts) + "," + str(question.marks) 
                            + "," + str(question.student_output) + "\n")
                        i += 1
                    else:
                        line = line.replace(line, self.username + "," + str(question.question_id)
                            + "," + str(question.attempts) + "," + str(question.marks) + "\n")
                        i += 1
                sys.stdout.write(line)
        
                
class TM_question:
    def __init__(self, id):
        self.question_id = id
        self.question = None
        self.num_choices = 0
        self.choices = []
        
        self.attempts = 0
        self.marks = 0
        self.student_output = None
        self.sample_output = "placeholder sample output" # placeholder - not dealt with yet
        
    def correct_attempt(self):
        self.marks += 3 - self.attempts
        self.attempts += 1
    
    def incorrect_attempt(self, student_output, sample_output):
        self.attempts += 1
        if self.attempts == 3:
            self.student_output = student_output
            self.sample_output = sample_output  
    
    def is_finished_correct(self):
        return self.marks > 0
    
    def is_finished_incorrect(self):
        return self.marks == 0 and self.attempts == 3
            
            

# PLACEHOLDER func that gets a random sequence of questions and their info for a new student
def QB_get_questions_new():
    # return in 2d lists of id, question string, num choices, <multiple choices if applicable>
    questions = [
        [3, "Write a hello world program in C.", -1],
        [2, "What is Chris' favourite colour?", 4, "red", "green", "blue", "black"],
        [6, "Which is not a network layer routing algorithm?", 4, "flooding", "leaky bucket", "link state", "distance vector"]
    ]
    return questions


# PLACEHOLDER func that gets info for a given sequence of questions 
def QB_get_questions(question_ids):
    # same format as no parameters counterpart -  getting question ids doesn't mean much here though
    questions = [
        [question_ids[0], "What is Chris' favourite colour?", 4, "red", "green", "blue", "black"],
        [question_ids[1], "Write a hello world program in C.", -1],
        [question_ids[2], "Which is not a network layer routing algorithm?", 4, "flooding", "leaky bucket", "link state", "distance vector"]
    ]
    return questions

# PLACEHOLDER func that checks an answer with the QB
def QB_check_question(question_id, student_answer, is_last_attempt):
    if is_last_attempt:
        # return in form is_correct, student_output, sample_output
        return False, student_answer, "placeholder checked_output"
    else:
        return False, None, None

# TEMPORARY test func to print all the info in a TM_student object
def TEST_student_info(student):
    print("username: " + student.username)
    print("current finished: " + str(student.current_finished) + "/" + str(num_questions))
    print("current_marks: " + str(student.current_marks) + "/" + str(num_questions * 3))
    print()
    for page_index in range(num_questions):
        question = student.questions[page_index]
        
        print("question id: " + str(question.question_id))        
        print("question: " + question.question)
        if question.num_choices == -1:
            print("\t[enter your code here]")
        else:
            for i in range (question.num_choices):
                print("\t" + str(i + 1) + ". " + question.choices[i])
        
        print("attempts: " + str(question.attempts))
        print("marks: " + str(question.marks))
        
        if question.is_finished_incorrect():
            print("\t3rd incorrect output: " + str(question.student_output))
            print("\tsample output: " + str(question.sample_output))
        
        print()

# TEMPORARY main func for testing
def main(input_username):
    # normally authentication here first to get username
    student = TM_student(input_username)
    student.read_answers()
    
    # display
    TEST_student_info(student)
    
    # entering answers
    student.check_answer(2, 3)
    
    # display
    TEST_student_info(student)
    
    # logout writing
    student.write_answers()

import sys
if __name__ == "__main__":
    main(sys.argv[1])
    
