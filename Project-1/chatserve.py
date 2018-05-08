#####################################################################
## Jonathan Jones (jonesjon 932709446)
## Project 1
## CS372 Spring2018
#####################################################################

from socket import *
from sys import argv, exit

def main():
	#Check usage
	if argv < 3:
		print 'USAGE: python chatserve.py hostname port'
		exit(1);

	#Get the server name and port number from command line arguments
	serverName = argv[1]
	serverPort = int(argv[2])

	serverSocket = set_socket(serverName, serverPort) #Set up the socket for TCP usage

	chat(serverSocket) #Establsh a connection with clients and communicate with them


##################################################
 # Function: get_handle
 # Description: Gets a user handle from stdin
 # Params: none
 # Returns: User handle string
 # Pre-conditions: none
 # Post-conditions: Valid user handle is returned where the handle is at most 10 characters, has no whitespace and includes "> " at the end
##################################################
def get_handle():
	loop = True

	while loop:
		handle = raw_input('Enter a user handle: ')
		loop = (handle.find(' ') != -1) or (len(handle) > 10)

	return handle + '> '

##################################################
 # Function: set_socket
 # Description: Sets up the socket with the server name and server port number specified
 # Params: Host name and port number
 # Returns: Socket to be used
 # Pre-conditions: Arguments passed in are properly defined
 # Post-conditions: Server port is properly set up, listening and returned
##################################################
def set_socket(serverName, serverPort):
	serverSocket = socket(AF_INET, SOCK_STREAM)
	serverSocket.bind((serverName, serverPort))
	serverSocket.listen(1)
	return serverSocket

##################################################
 # Function: chat
 # Description: Establishes a connection to other hosts and allows the user to communicate with them via stdin entries. Runs until SIGINT is recieved.
 # Params: Server socket for connections
 # Returns: None
 # Pre-conditions: Server socket is properly set up and listening
 # Post-conditions: None
##################################################
def chat(serverSocket):
	handle = get_handle() #Get the user handle
	while 1:
		print 'Waiting for connection...'
		connectionSocket, addr = serverSocket.accept() #Accept connections that come in
		print 'Connection made!'

		while 1:
			message = connectionSocket.recv(1024) #Recieve up to 1024 bytes from other hosts and store them in message
			if message.find('\quit') != -1: #If message contains "\quit" then go back to waiting for connections to arrive
				print 'Connection terminated by peer'
				break
			print message
			sentence = handle + raw_input(handle) #Get message we're sending and append the user handle to the beginning of it
			connectionSocket.send(sentence)
			if sentence.find('\quit') != -1: #If message we're sending contains "\quit" then go back to waiting for connections to arrive
				print 'You have terminated the connection'
				break
			
		connectionSocket.close() #Close socket connection

main()