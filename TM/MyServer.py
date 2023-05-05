from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import unquote
import json
from authentication import *
from datastructs import *

hostName = "localhost"
serverPort = 8000

class MyServer(BaseHTTPRequestHandler):            
    # Builds the html page
    def do_GET(self):
        if self.path == "/":
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
                        
            with open("index.html", "r") as f:
                html = f.read()
                self.wfile.write(bytes(html, "utf-8"))
        #elif self.path == "/answers":
            #TODO: send response from tm
        else:
            self.send_response(404)
            self.end_headers()

    # Accepts data from page
    def do_POST(self):
        if self.path == "/login":
            content_length = int(self.headers["Content-Length"])
            body = self.rfile.read(content_length)
            data = json.loads(body)
            username = data["username"]
            password = data["password"]
            
            if validate_student("test_login_file.txt" ,username, password):
                student = TM_student(username)
                student.get_questions()
                student.get_answers()
                
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                #questions_json = json.dumps(questions)
                
                # Send success and questions data
                response = {"success": True, "questions": student.questions, "types": student.types, 
                            "choices": student.choices, "current_finished": student.current_finished, 
                            "current_marks": student.current_marks, "attempts": student.attempts, "marks": student.marks}
                self.wfile.write(json.dumps(response).encode("utf-8"))
            else:
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"success": False}).encode("utf-8"))
                
        elif self.path == "/submit-answer":
            content_length = int(self.headers["Content-Length"])
            body = self.rfile.read(content_length)
            data = body.decode("utf-8")
            answer, pos, attempts = data.split(":")
            answer = unquote(answer)
            
            # TODO: Do something with this data
            print("Answer: " + answer)
            print("Question Number: " + pos)
            
            self.send_response(200)
            self.end_headers()
            
            response = {"success": True, "correct": check_answer(pos, answer, attempts)} 
            self.wfile.write(json.dumps(response).encode("utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

def check_answer(pos, answer, attempts):
    return True

if __name__ == "__main__":
    # Makes a server object
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")
