import struct
import sys

numQuestions = 4
seed = hash("ASD") # 10 ::: 1 + 1 + 
v = struct.pack("i", 10) + b'G' + struct.pack("c", numQuestions.to_bytes(1, 'big')) + struct.pack("q", seed)
print(len(v))
#print(struct.pack("q", hash("ASD")))


#hostname = socket.gethostname()
#print(hostname)

#HOST = socket.gethostbyname(hostname)
#print(HOST)
