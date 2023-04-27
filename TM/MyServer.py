from http.server import BaseHTTPRequestHandler, HTTPServer
import json

hostName = "localhost"
serverPort = 8080

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
            
            # TODO: Check if login information is valid
            # Placeholder function for now            
            # Check credentials
            if check_credentials(username, password):
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()

                # Get questions
                questions = get_questions(username)
                print(questions)
                #questions_json = json.dumps(questions)
                
                # Send success and questions data
                response = {"success": True, "questions": questions}
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
            answer, pos = data.split(":")
            
            # TODO: Do something with this data
            print("Answer: " + answer)
            print("Question Number: " + pos)
            
            self.send_response(200)
            self.end_headers()
            self.wfile.write(bytes("Answer received: " + answer, "utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

def check_credentials(username, password):
    # TODO: Implement a function to check if username and password are valid
    # For now, we'll just return True
    return True

def get_questions(username):
    if (username == "username1"):
        questions = [
            "What is the capital of France?",
            "What is the largest country in the world?",
            "What is the highest mountain in the world?"
        ]
    elif (username == "username2"):
        questions = [
            "What is the currency of Japan?",
            "What is the largest desert in the world?",
            "What is the deepest ocean in the world?"
        ]
    else:
        questions = [
            "What is the capital of Canada?",
            "What is the smallest country in the world?",
            "What is the fastest land animal in the world?"
        ]
    return questions, answers


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
