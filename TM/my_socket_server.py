import socket
import struct
import selectors
import types
from queue import Queue
from config import *

sel = selectors.DefaultSelector()
data_to_send = Queue()
data_received = Queue()
qbs = Queue()
waiting_reconnect = False

# starts the socket server, runs forever
def StartServer(stop):
    #initialise host socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening on host {HOST}, port {PORT}")
    #s.setblocking(False) #needed or not? We have threads
    sel.register(s, selectors.EVENT_READ, data=None)
    
    # continuously check for events with the sockets
    while True:
        events = sel.select(timeout = 0.1) #will we need to deal with timeout later on
        for key, mask in events:
            #print(key.data)
            if key.data is None:
                AcceptConnection(s)
            else:
                ServiceConnection(key, mask)
        if stop():
            break
    
    # close the socket if an exception occurs
    print(f"Closing host {HOST}, port {PORT}")
    s.close()     
   
# accept an incoming connection from a socket, registering into selector
def AcceptConnection(s):
    conn, addr = s.accept()
    
    global waiting_reconnect
    already_connected = False
    if waiting_reconnect:
        for qb in list(qbs.queue):
            if qb == None and not already_connected:
                qbs.get()
                qbs.put(addr)
                print(f"Connected to {addr}")
                if None not in list(qbs.queue):
                    waiting_reconnect = False
                already_connected = True
            else:
                qbs.get()
                qbs.put(qb)
    else:
        qbs.put(addr)
        print(f"Connected to {addr}")

    #conn.setblocking(False)
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

        length_header = int.from_bytes(recv_data[:4], byteorder = "little")
        print(length_header)
        recv_data = recv_data[4:]
        got = len(recv_data)
        while got < length_header:
            d = sock.recv(1024)
            got += len(d)
            recv_data += d
        print(recv_data)
        if recv_data:
            # data received from the socket
            data_received.put((data.addr, recv_data))
        else: 
            print(recv_data)
            # socket has closed the connection/ connection lost
            print(f"Closing connection to {data.addr}")
            sel.unregister(sock)
            sock.close()
            
            global waiting_reconnect
            waiting_reconnect = True
            for qb in list(qbs.queue):
                if qb == data.addr:
                    qbs.get()
                    qbs.put(None)
                else:
                    qbs.get()
                    qbs.put(qb)
    if mask & selectors.EVENT_WRITE:  
        # socket is ready to write to          
        if not data_to_send.empty() and list(data_to_send.queue)[0][0] == key.data.addr:
            # if first message in queue is for this socket, send it
            data.outb = data_to_send.get()[1]
            print(f"Sending data to {data.addr}")
            sock.sendall(data.outb)
           
# serialises and sends a question request to a qb,
# waits for a reply and returns it deserialised
def GenQuestionsRequest(qb_index, numQuestions, seed):
    addr = list(qbs.queue)[qb_index]
    data_to_send.put((addr, struct.pack("i", 10) + b'G' 
        + struct.pack("c", numQuestions.to_bytes(1, 'big')) + struct.pack("q", seed)))

    # wait for a reply to appear in the queue
    while True:
        for recv_data in list(data_received.queue):
            if recv_data[0] == addr:
                print(f"Received data from {addr}")
                # deserialise reply
                rawReceived = data_received.get()[1] 

                received = rawReceived.decode('utf-8').split("\;")
                                
                questions = received[:numQuestions]
                types = received[numQuestions : 2 * numQuestions]
                choices = [choice.split("\,") for choice in received[2 * numQuestions : -1]]
                
                return questions, types, choices
            
# serialises and sends a check answer request to a qb, 
# waits for a reply and returns it deserialised
def CheckAnswerRequest(qb_index, seedIndex, seed, is_last_attempt, student_answer):
    addr = list(qbs.queue)[qb_index]
    
    data_to_send.put((addr, struct.pack("i", 11 + len(student_answer)) + b'C'
        + struct.pack("c", seedIndex.to_bytes(1, 'big')) + struct.pack("q", seed)
        + struct.pack("c", is_last_attempt.to_bytes(1, 'big')) + bytes(student_answer, "utf-8")))    
    
    # wait for a reply to appear in the queue
    while True:
        for recv_data in list(data_received.queue):
            if recv_data[0] == addr:
                print(f"Received data from {addr}")    
                
                # deserialise reply
                rawReceived = data_received.get()[1]
                print(rawReceived)
                correct_flag = chr(rawReceived[0])
                is_correct = correct_flag == 't'
                is_image_output = correct_flag == 'i'

                # if 3rd incorrect attempt return outputs
                if is_last_attempt and not is_correct:
                    rawReceived = rawReceived[1:]
                    print(rawReceived)

                    header = int.from_bytes(rawReceived[:4], byteorder = "little")
                    rawReceived = rawReceived[4:]
                    if is_image_output:
                        sample_output = rawReceived[:header]
                    else:
                        sample_output = rawReceived[:header].decode('utf-8')
                        
                    rawReceived = rawReceived[header:]
                    print(rawReceived)
                    
                    header = int.from_bytes(rawReceived[:4], byteorder = "little")
                    rawReceived = rawReceived[4:]
                    if is_image_output:
                        student_output = rawReceived[:header]
                    else:
                        student_output = rawReceived[:header].decode('utf-8')
                    
                    return is_correct, is_image_output, sample_output, student_output
                else:
                    return is_correct, False, "", ""

def test_ready():
    qbs_ready = 0
    for qb in list(qbs.queue):
        if qb != None:
            qbs_ready += 1
    return qbs_ready == num_qbs