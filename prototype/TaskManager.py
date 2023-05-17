import socket
import struct
import selectors
import types
from threading import Thread
from queue import Queue

# selector and queues to maintain communication between threads
sel = selectors.DefaultSelector()
data_to_send = Queue()
data_received = Queue()
qbs = Queue()

# globals to send
HOST = "127.0.0.1"
PORT = 65432

# starts the socket server, runs forever
def StartServer():
    #initialise host socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening on host {HOST}, port {PORT}")
    s.setblocking(False) #needed or not? We have threads
    sel.register(s, selectors.EVENT_READ, data=None)
    
    # continuously check for events with the sockets
    while True:
        events = sel.select(timeout=None) #will we need to deal with timeout later on
        for key, mask in events:
            if key.data is None:
                AcceptConnection(s)
            else:
                ServiceConnection(key, mask)        
   
# accept an incoming connection from a socket, registering into selector
def AcceptConnection(s):
    conn, addr = s.accept()
    print(f"Connected to {addr}, id {len(list(qbs.queue))}")
    qbs.put(addr)
    conn.setblocking(False)
    data = types.SimpleNamespace(addr=addr, inb=b"", outb=b"")
    events = selectors.EVENT_READ | selectors.EVENT_WRITE
    sel.register(conn, events, data)
    
# checks for any events for a particular socket in selector, 
# sending any queued up messages and and receiving messages into queue
def ServiceConnection(key, mask):
    sock = key.fileobj
    data = key.data
    if mask & selectors.EVENT_READ:
        # something came in from socket
        recv_data = sock.recv(1024)
        if recv_data:
            # data received from the socket
            data_received.put((data.addr, recv_data))
        else: 
            # socket has closed the connection/ connection lost
            print(f"Closing connection to {data.addr}")
            sel.unregister(sock)
            sock.close()
            
            new_qbs = Queue()
            for qb in list(qbs.queue):
                if qb[0] == data.addr:
                    continue
                else:
                    new_qbs.put(qb)
            qb = new_qbs   
    if mask & selectors.EVENT_WRITE:  
        # socket is ready to write to          
        if not data_to_send.empty() and list(data_to_send.queue)[0][0] == key.data.addr:
            # if first message in queue is for this socket, send it
            data.outb = data_to_send.get()[1]
            print(f"Sending data to {data.addr}")
            sock.sendall(data.outb)
           
# FOR TESTING PURPOSES 
# sends a text message to a qb, waits for a reply and returns it
def SendMessage(qb_index, data):
    addr = list(qbs.queue)[qb_index]
    data_to_send.put((addr, struct.pack("i", len(data)) + bytes(data, "utf-8"))) #Just sends length of data then data    
    
    # wait for a reply to appear in the queue
    while True:
        for recv_data in list(data_received.queue):
            if recv_data[0] == addr:
                print(f"Received data from {addr}")
                return data_received.get()[1].decode('utf-8')
  

# serialises and sends a question request to a qb, waits for a reply and returns it deserialised
def GenQuestionRequest(qb_index, numQuestions, seed):
    addr = list(qbs.queue)[qb_index]
    data_to_send.put((addr, struct.pack("i", 10) + b'G' 
        + struct.pack("c", numQuestions.to_bytes(1, 'big')) + struct.pack("q", seed)))

    # wait for a reply to appear in the queue
    while True:
        for recv_data in list(data_received.queue):
            if recv_data[0] == addr:
                print(f"Received data from {addr}")
                
                # deserialise reply TBC
                return data_received.get()[1].decode('utf-8')

import sys
if __name__ == "__main__":
    # starts a thread to run the socket server forever
    thread = Thread(target=StartServer)
    thread.daemon = True
    thread.start()
    
    # keeps looking for inputs from another part of the program
    # FOR TESTING PURPOSES - implemented here as taking input from cmdline
    while True:
        if not qbs.empty():
            try:
                my_qb = input()
                username = input()
                num_questions = input()  
                result = GenQuestionRequest(int(my_qb), int(num_questions), hash(username))     
                print(f"reply: {result}")
            except KeyboardInterrupt:
                print("Interrupted, closing program\n")
                sys.exit(130)
            
            
