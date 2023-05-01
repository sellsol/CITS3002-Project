# class for our database of student answers info
class TM_student:
    def __init__(self, username, DB_filepath):
        self.username = username
        self.DB_filepath = DB_filepath
        self.num_questions = 3 #TO REPLACE according to how to get number of questions in the test
        
        self.current_finished = 0
        self.current_marks = 0
        self.answers = []
        
    # checks if a student is already in the TM_database
    def is_new_student(self):
        TM_database = open(self.DB_filepath, "r")
        
        for line in TM_database:
            line_username = line.strip().split(",")[0]
            if line_username == self.username:
                return False
            
            for _ in range(self.num_questions):
                next(TM_database)
                
        TM_database.close()
        return True
        
    # reads current answers for this student from the TM_databse
    def read_answers(self):
        if self.is_new_student():
            # NOT FILLED OUT - IN TESTING
            pass
        else:
            TM_database = open(self.DB_filepath, "r")
            line_answer = TM_database.readline().strip().split(",", 5)
            
            # skips through the file until a matching username is found
            while line_answer[0] != self.username:
                line_answer = TM_database.readline().strip().split(",", 4)

            # gets the succeeding lines in the file (same student) and read into student answers
            for _ in range(self.num_questions):
                answer = TM_answer(int(line_answer[1]), "Placeholder q string") # TO REPLACE - get question strings from QB
                answer.attempts = int(line_answer[2])
                answer.marks = int(line_answer[3])
                
                # if student already has 3 incorrect attempts, read in the student's last output and update progress
                if (answer.marks == 0 and answer.attempts == 3):
                    answer.student_output = line_answer[4]
                    answer.sample_output = "Placeholder sample output" # TO REPLACE - get sample output from QB
                    self.current_finished += 1
                # if student already answered the question correctly, update progress
                if (answer.marks > 0):
                    self.current_finished += 1
                    self.current_marks += answer.marks
                
                self.answers.append(answer)
                line_answer = TM_database.readline().strip().split(",", 4)
            
            # no need to get lines after that, just close database
            TM_database.close()
                
class TM_answer:
    def __init__(self, question_id, question):
        self.question_id = question_id
        self.question = question
        self.attempts = 0
        self.marks = 0
        self.student_output = ""
        self.sample_output = ""
        
        

# TEMPORARY test func to print all the info in a TM_student object
def TEST_student_info(student):
    print("username: " + student.username)
    print("current finished: " + str(student.current_finished) + "/" + str(student.num_questions))
    print("current_marks: " + str(student.current_marks) + "/" + str(student.num_questions * 3))
    print()
    for page_index in range(student.num_questions):
        answer = student.answers[page_index]
        print("question id: " + str(answer.question_id))
        print("\tquestion: " + answer.question)
        print("\tattempts: " + str(answer.attempts))
        print("\tmarks: " + str(answer.marks))
        
        if (answer.marks == 0 and answer.attempts == 3):
            print("\t3rd incorrect output: " + answer.student_output)
            print("\tsample output: " + answer.sample_output)

# TEMPORARY main func for testing - currently testing a info initialising sequence for students already in TM_database
def main(input_username):
    # normally authentication here first to get username
    student = TM_student(input_username, "test_TM_database.txt")
    student.read_answers()
    
    TEST_student_info(student)

import sys
if __name__ == "__main__":
    main(sys.argv[1])
    
