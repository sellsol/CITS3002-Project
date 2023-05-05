import socket
import struct
import selectors

sel = selectors.DefaultSelector()

HOST = "127.0.0.1"
PORT = 65432

print(f"Hosting on {HOST}, port {PORT}");
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);
s.bind((HOST, PORT))
s.listen()
conn, addr = s.accept()

print(f"Connected by {addr}")

def SendMessage(s, data):
    s.sendall(struct.pack("i", len(data)) + bytes(data, "utf-8")); #Sends length of data then data

SendMessage(conn, "WAAZZZZAAAAP")

while True:
    data = conn.recv(1024)
    if not data:
        break
    print(data)

conn.close()
s.close()
