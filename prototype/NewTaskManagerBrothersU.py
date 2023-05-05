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
s.setblocking(False)
sel.register(s, selectors.EVENT_READ, data=None)

print(f"Connected by {addr}")

def SendMessage(s, data):
    s.sendall(struct.pack("i", len(data)) + bytes(data, "utf-8")); #Sends length of data then data

def accept_wrapper(s):
    conn, addr = s.accept()
    print(f"Accepted connection from {addr}")
    conn.setblocking(False)
    data = types.SimpleNamespace(addr=addr, inb=b"", outb=b"")
    events = selectors.EVENT_READ | selectors.EVENT_WRITE
    sel.register(conn, events, data=data)
    
def service_connection(key, mask):
    s = key.fileobj
    data = key.data
    if (mask & selectors.EVENT_READ):
        recv_data = s.recv(1024) #Should use recvall
        if recv_data:
            data.outb += recv_data
        else:
            print(f"Closing connection to {data.addr}")
            sel.unregister(s)
            s.close()
    if (mask & selectors.EVENT_WRITE):
        if data.outb:
            print(f"Echoing {data.outb!r} to {data.addr}")
            sent = s.send()
            data.outb = data.outb[sent:]

try:
    while True:
        events = sel.select(timeout=None)
        for key, mask in events:
            if key.data is None:
                accept_wrapper(key.fileObj)
            else:
                service_connection(key, mask)
except KeyboardInterrupt:
    print("Caught ya doing a keyboard interrupt")
finally:
    sel.close()
