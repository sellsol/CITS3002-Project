import socket
import struct
import selectors

sel = selectors.DefaultSelector()

HOST = "127.0.0.1"
PORT = 65432

print(f"Hosting on {HOST}, port {PORT}")
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen()
conn, addr = s.accept()

print(f"Connected by {addr}")

def CheckAnswerRequest(s, seedIndex, seed, last_attempt, student_answer):
    s.sendall((struct.pack("i", 11 + len(student_answer)) + b'C'
        + struct.pack("c", seedIndex.to_bytes(1, 'big')) + struct.pack("q", seed)
        + struct.pack("c", last_attempt.to_bytes(1, 'big')) + bytes(student_answer, "utf-8")))

def GenQuestionsRequest(s, seedIndex, seed):
    s.sendall(struct.pack("i", 10) + b'G'
        + struct.pack("c", seedIndex.to_bytes(1, 'big')) + struct.pack("q", seed))

f = open("../QB/sample_answers/1/sample.py")
text = f.read()
f.close()
#GenQuestionsRequest(conn, 3, 1231)
#CheckAnswerRequest(conn, 3, 1231, 1, text)
i = 1400
while True:
    if (i < 1410):
        GenQuestionsRequest(conn, 3, i)
        i += 1
    data = conn.recv(1024)
    if not data:
        break
    print(data)
    print(" ")

conn.close()
s.close()
