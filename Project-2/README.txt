README - ftclient.py and ftserver.c
Author: Jonathan Jones (jonesjon 932709446)

These programs are meant to be executed on the ENGR FLIP servers.

[1] --- Compiling the programs ---

	Use the makefile to compile ftserver.c:

		make

	This will compile ftserver.c as ftserver
	
	There is no need to compile the python script since its a runtime script.

[2] --- Starting up ftclient.c ---

    Before you start the program, run the command:

        hostname
    
    to determine which ENGR FLIP server you are on so that you know what hostname to communicate with.

    Start the program:

        ftclient <PORT_NUM>

    Make sure to use a valid port number.
    The server is now up and running.

[3] --- Starting up ftclient.py ---

    Open a new terminal on the ENGR FLIP servers.

    Start the program:

        python ftclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND (-g followed by <FILENAME> or -l)> <FILENAME (If -g)>

    Make sure that the python command runs python2.7.5 and that all your command line arguments are valid.
    If you are running this multiple times without restarting the server, please use a different data port number every time. It takes some time for those ports to close on FLIP.

    For my testing, I ran the server on flip1 and the client on flip2.
    I would usually run

        python ftclient.py flip1 <PORT_NUM> -g <FILENAME> <DATA_PORT>
        or
        python ftclient.py flip1 <PORT_NUM> -l <DATA_PORT>

[4] --- Checking for files ---

    New files will be created with the prepend "copy-"

    Check both terminals output messages.
    