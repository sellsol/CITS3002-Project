# checks inputted student username and password against file
def validate_student(login_file, input_username, input_password):
    try:
        with open(login_file, "r") as infile:
            for line in infile: 
                login_info = line.strip().split(",")
                if login_info[0] == input_username and login_info[1] == input_password:
                    return True
            return False
    except FileNotFoundError as err:
        # error handling for if the file could not be found    
        print("Error: password file " + login_file + " could not be found")
        raise err
    
            

# TEMPORARY main func for testing - currently testing validate_student
def main(input_username, input_password):
    print(validate_student("test_login_file.txt", input_username, input_password))
    return

import sys
if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])