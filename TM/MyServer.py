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
            
            #TODO: Call function to get questions
            questions = [
                "What is your name?",
                "What is your favorite color?",
                "What is your favorite food?",
                "What is your favorite hobby?"
            ]
            questions_json = json.dumps(questions)
                        
            with open("index.html", "r") as f:
                html = f.read()
                html = html.replace("{{questions}}", questions_json)
                self.wfile.write(bytes(html, "utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

    # Accepts data from page
    def do_POST(self):
        if self.path == "/submit-answer":
            content_length = int(self.headers["Content-Length"])
            body = self.rfile.read(content_length)
            data = body.decode("utf-8")
            answer, pos = data.split(":")
            
            #TODO: Do something with this data
            print("Answer: " + answer)
            print("Question Number: " + pos)
            self.send_response(200)
            self.end_headers()
            self.wfile.write(bytes("Answer received: " + answer, "utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

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
