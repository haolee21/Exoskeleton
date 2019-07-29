import socket
import sys
def main():
    # print("wait for connection\n")
    # sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    # #bind the socket to the port
    # server_address = ('192.168.1.142',5005)
    # sock.bind(server_address)

    # sock.listen()
    # while True:
    #     print(sys.stderr, "wait for a connection")
    #     connection, client_address = sock.accept()


    HOST = '192.168.1.142' # Server IP or Hostname
    PORT = 12345 # Pick an open Port (1000+ recommended), must match the client sport
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('Socket created')
    #managing error exception

    try:
	    s.bind((HOST, PORT))
    except socket.error:
	    print ('Bind failed ')
    s.listen()

    print ('Socket awaiting messages')
    conn, addr = s.accept()
    print ('Connected')

    while True:
        data = (conn.recv(6)).decode('ascii')
        print(data)
        if(data =='end'):
            print(data=='end')
            break
    return 0
if __name__ == '__main__':
    main()
    