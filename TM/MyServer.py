from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import unquote
import json
from config import *
from authentication import *
from datastructs import *

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
            
            if validate_student(loginFile ,username, password):
                questions, types, choices = get_questions(username)
                current_finished, current_marks, attempts, marks = get_answers(username)
                
                print("\tLogged in: " + username)
                
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                #questions_json = json.dumps(questions)
                
                # Send success and questions data
                response = {"success": True, "questions": questions, "types": types, 
                            "choices": choices, "current_finished": current_finished, 
                            "current_marks": current_marks, "attempts": attempts, "marks": marks}
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
            username, answer, pos, attempts = data.split(":")
            answer = unquote(answer)
            
            # TODO: Do something with this data
            print("\tUsername: " + username)
            print("\tQuestion Index: " + pos)
            print("\tAnswer: " + answer)
            
            self.send_response(200)
            self.end_headers()
            
            response = {"success": True, "correct": check_answer(username, int(pos), answer, int(attempts))} 
            self.wfile.write(json.dumps(response).encode("utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

    # handles when user browser unexpectedly closes
    def handle(self):
        try:
            BaseHTTPRequestHandler.handle(self)
        except socket.error:
            pass