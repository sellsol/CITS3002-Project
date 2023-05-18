from http.server import BaseHTTPRequestHandler, HTTPServer
from http.cookies import SimpleCookie
from urllib.parse import unquote
import json
from config import *
from operations import *
import base64

class MyServer(BaseHTTPRequestHandler):            
    # Builds the html page
    def do_GET(self):
        if self.path == "/":
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            cookie = SimpleCookie(self.headers.get("Cookie"))
            username = cookie.get("username")
            
            if not test_ready():
                print("\tStudent connected while test not ready")
                with open("qb_error.html", "r") as f:
                    html = f.read()
                    self.wfile.write(bytes(html, "utf-8")) 
            elif username is None:
                print ("\tStudent connected to login page")
                with open("login.html", "r") as f:
                    html = f.read()
                    self.wfile.write(bytes(html, "utf-8")) 
            else:
                with open("questions.html", "r") as f:
                    html = f.read()
                    self.wfile.write(bytes(html, "utf-8"))
        elif self.path == "/styles.css":
            self.send_response(200)
            self.send_header("Content-type", "text/css")
            self.end_headers()

            with open("styles.css", "r") as f:
                css = f.read()
                self.wfile.write(bytes(css, "utf-8"))

        elif self.path == "/login_script.js":
            self.send_response(200)
            self.send_header("Content-type", "text/javascript")
            self.end_headers()

            with open("login_script.js", "r") as f:
                js = f.read()
                self.wfile.write(bytes(js, "utf-8"))
        elif self.path == "/questions_script.js":
            self.send_response(200)
            self.send_header("Content-type", "text/javascript")
            self.end_headers()

            with open("questions_script.js", "r") as f:
                js = f.read()
                self.wfile.write(bytes(js, "utf-8"))
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
                cookie = SimpleCookie()
                cookie["username"] = username
                cookie["username"]["path"] = "/"
                cookie["username"]["max-age"] = 3600 
                
                self.send_response(200)
                self.send_header("Location", "/")
                self.send_header("Set-Cookie", cookie.output(header=""))
                self.end_headers()
                
                print("\tStudent logged in: " + username)
            else:
                self.send_response(401)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"success": False}).encode("utf-8"))
                print("\tStudent failed to login: " + username)
        elif self.path == "/get-data":
            cookie = SimpleCookie(self.headers.get("Cookie"))
            username = cookie.get("username")
            
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            
            questions, types, choices = get_questions(username.value)
            current_finished, current_marks, attempts, marks = get_answers(username.value)
            response = {"success": True, "username": username.value, "questions": questions, "types": types, 
                        "choices": choices, "current_finished": current_finished, 
                        "current_marks": current_marks, "attempts": attempts, "marks": marks}
            self.wfile.write(json.dumps(response).encode("utf-8"))
            print("\tStudent's questions loaded")
        elif self.path == "/submit-answer":
            if (not test_ready()):
                self.send_response(503)
                self.end_headers()
            else:
                content_length = int(self.headers["Content-Length"])
                body = self.rfile.read(content_length)
                data = body.decode("utf-8")
                username, answer, pos, attempts = data.split(":")
                answer = unquote(answer)
                is_last_attempt = int(attempts) == 2
                
                print("\tSubmitting answer from: " + username, ", Question index: " + pos)
                print("\tAnswer: " + answer)
                self.send_response(200)
                self.end_headers()                
                correct, is_image_output, sample_output, student_output = check_answer(username, int(pos), answer, is_last_attempt)
                if (is_image_output):
                    response = {"success": True, "image": True, "correct": False, 
                                "student_output": base64.b64encode(student_output).decode("utf-8"), 
                                "sample_output": base64.b64encode(sample_output).decode("utf-8")}
                else:
                    response = {"success": True, "image": False, "correct": correct, 
                                "student_output": student_output, 
                                "sample_output": sample_output}
                self.wfile.write(json.dumps(response).encode("utf-8"))
        else:
            self.send_response(404)
            self.end_headers()
            
    def handle(self):
        try:
            BaseHTTPRequestHandler.handle(self)
        except socket.error:
            pass