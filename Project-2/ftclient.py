from socket import *
from sys import argv, exit, stdout

def main():
    #Get command line arguments
    args_len = len(argv)
    if args_len >= 5: #Minimum number of arguments?
        serverHost = argv[1]
        serverPort = int(argv[2])
        command = argv[3]
        if args_len == 6 and command == '-g': #File request
            command_type = 1
            filename = argv[4]
            dataPort = int(argv[5])
        elif args_len == 5 and command == '-l': #Directory request
            command_type = 0
            filename = ''
            dataPort = int(argv[4])
        else:
           usage_message() #Usage error
    else:
        usage_message() #Usage error

    clientSocket = setup_connection(serverHost, serverPort) #Set up connection with server
    dataSocket = set_data_port(dataPort) #Set up data port for receiving data
    send_command(clientSocket, command_type, filename, dataPort) #Send command to server

    if command_type: #If we're getting a file
        recv_file(serverHost, serverPort, dataPort, dataSocket, clientSocket, filename)
    else:
        recv_directory(serverHost, dataPort, dataSocket)
    
    #Close sockets
    dataSocket.close()
    clientSocket.close()

##################################################
 # Function: recv_file
 # Description: Recieves a file or error message from the server. If a file is recieved, creates a new file in current directory called "copy-filename"
 # Params: Server name, server port number, data port number, data socket, control socket, file name
 # Returns: none
 # Pre-conditions: Control connection is already set up with server. Data port is already listening.
 # Post-conditions: File recieved and saved or error message displayed
##################################################
def recv_file(serverHost, serverPort, dataPort, dataSocket, clientSocket, filename):
    file_found = int(clientSocket.recv(1)) #Get message from server that tells us if file has been found or not

    if not file_found: #Tell user that file has not been found
        print serverHost + ':' + str(serverPort) + ' says FILE NOT FOUND'
        return

    f = open("copy-" + filename, "w+") #Open a new file with the prepend "copy-"

    connectionSocket = data_port_connector(dataSocket) #Wait for a connection on the data port

    print 'Recieving \"' + filename + '\" from ' + serverHost + ':' + str(dataPort)

    #Recieve 256 bytes of data at a time and write them into the file until an EOT character is recieved
    while True:
        file_data = connectionSocket.recv(256)
        if file_data.endswith(chr(4)): #If the file_data ends with the end of transmission character
            f.write(file_data[:-1])
            break
        f.write(file_data) #Write into the file

    print 'File transfer complete.\nFile saved as \"copy-' + filename + '\"'
    f.close() #Close the file

##################################################
 # Function: recv_directory
 # Description: Recieves the list of contents of the server directory
 # Params: Server name, data port number, data socket
 # Returns: None
 # Pre-conditions: Data socket is listening
 # Post-conditions: Directory has been displayed to the user
##################################################
def recv_directory(serverHost, dataPort, dataSocket):
    connectionSocket = data_port_connector(dataSocket) #Wait for a data connection from server
    print 'Recieving directory structure from ' + serverHost + ':' + str(dataPort)

    #Recieve directory in 128 byte segments
    while True:
        dir_data = connectionSocket.recv(128)
        if chr(4) in dir_data: #If end of file character found, display rest of directory and break out of loop
            print dir_data[:-1]
            break
        stdout.write(dir_data)

##################################################
 # Function: data_port_connector
 # Description: Waits for a connection on the data socket and accepts
 # Params: data socket
 # Returns: none
 # Pre-conditions: data socket is listening
 # Post-conditions: connection accepted or error
##################################################
def data_port_connector(dataSocket):
    connectionSocket, addr = dataSocket.accept()
    return connectionSocket

##################################################
 # Function: set_data_port
 # Description: Sets up the data port for listening
 # Params: Data port number
 # Returns: Data socket (listening)
 # Pre-conditions: None
 # Post-conditions: Data socket is listening
##################################################
def set_data_port(dataPort):
    dataSocket = socket(AF_INET, SOCK_STREAM)
    dataSocket.bind((gethostname(), dataPort))
    dataSocket.listen(1)
    return dataSocket

##################################################
 # Function: send_command
 # Description: Sends a command to the server. 
 # Params: Socket for sending command to server with, type of command (0 = -l, 1 = -g), file name (Can be empty if -l), data port number
 # Returns: none
 # Pre-conditions: Control connection already set up with server
 # Post-conditions: Command has been sent.
##################################################
def send_command(clientSocket, command_type, filename, dataPort):
    if command_type: #If -g <FILENAME>
        clientSocket.send(str(command_type)+str(dataPort)+'@'+filename)
    else:
        clientSocket.send(str(command_type)+str(dataPort)+'@')

##################################################
 # Function: setup_connection
 # Description: Connects to the server.
 # Params: host name, port number
 # Returns: Socket for communication with server
 # Pre-conditions: Valid host name and port number
 # Post-conditions: Connection is setup or error
##################################################
def setup_connection(host, port):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((host, port))
    return clientSocket

##################################################
 # Function: usage_message
 # Description: Spits out a usage message when bad arguments are recieved or not enough arguments are recieved
 # Params: None
 # Returns: None
 # Pre-conditions: None
 # Post-conditions: Exits with status 1
##################################################
def usage_message():
    print 'USAGE: python ftclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND (-g followed by <FILENAME> or -l)> <FILENAME (If -g)>'
    exit(1)

main()