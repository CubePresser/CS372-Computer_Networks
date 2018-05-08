README - chatclient.c and chatserve.py
Author: Jonathan Jones (jonesjon 932709446)

These programs are meant to be executed on the ENGR FLIP servers.

[1] --- Compiling the programs ---

	Use the makefile to compile chatclient.c:

		make

	This will compile chatclient.c as chatclient
	
	There is no need to compile the python script since its a runtime script.



[2] --- Starting up chatserve.py ---

	Start the program:

		python chatserve.py hostname port

	For the purposes of this assignment, hostname should be localhost and the port number must be a valid port

	When chatserve has started it should prompt you for a user handle. 
	Enter a valid handle (10 characters at most, no whitespace)

	Once the handle has been entered, the program will block while it waits for a connection.



[3] --- Starting up chatclient ---

	Make sure to follow step [1] to compile chatclient.c into the executable chatclient

	Open a new window on the ENGR FLIP servers.

	Start the program:

		chatclient hostname port

	For the purposes of this assignment, hostname should be localhost.
	The port number must be the port that you specified for chatserve.py in [2]

	When chatclient has started it should prompt you for a user handle.
	Enter a valid handle (10 characters at most, no whitespace)

	After the handle has been entered, both your current window and the chatclient.py window should say "Connection made!"


[4] --- Chatting between the two programs ---

	chatclient will always go first when it comes to communicating between the two hosts.

	Messages are sent by pressing ENTER.

	Turns in sending messages will alternate between the two hosts.
	After the host has sent a message, the server can send a message then the host and so on.

	While it is not a host's turn to send a message, they will block until a message is recieved from the other host.

	To end a connection, type "\quit" into either chatclient or chatserve.py and send the message.