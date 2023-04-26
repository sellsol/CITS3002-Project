import socket
import struct

HOST = "127.0.0.1"
PORT = 65432

print(f"Hosting on {HOST}, port {PORT}");
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen()
conn, addr = s.accept()

print(f"Connected by {addr}")
data = struct.pack("i", 12) + b"WAAZZZZAAAAP"
conn.sendall(data)
while True:
    data = conn.recv(1024)
    if not data:
        break
    print(data)

conn.close()
s.close()
