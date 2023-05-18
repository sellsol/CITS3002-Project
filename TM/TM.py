import sys
from threading import Thread
from config import *
from my_http_server import *
from my_socket_server import *
from operations import *

if __name__ == "__main__":
    # thread that currently runs the socket server
    stop_server = False
    thread = Thread(target=StartServer, args =(lambda : stop_server, ))
    thread.start()

    # running the httpserver 
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        stop_server = True
        thread.join()
        webServer.server_close()
        print("Interrupted, closing program.\n")
        sys.exit(130)

