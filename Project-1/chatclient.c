////////////////////////////////////////////////////////////////////
// Jonathan Jones (jonesjon 932709446)
// Project 1
// CS372 Spring2018
/////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Prototypes
void fill_addr_struct(struct sockaddr_in*, struct hostent*, int, char*);
void set_socket(int*, struct sockaddr_in*);
void connect_server(struct sockaddr_in*, int*);
void send_message(int*, char*);
int recv_message(int*);
char* get_message(char*);
char* get_handle();
void error(const char *msg, int);

int main(int argc, char** argv)
{
	//Check usage
    if(argc < 3)
    {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    //Initialize variables
	int socketFD;
	struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;

    //Get the user handle
	char* handle = get_handle();
	char* message = NULL;

	//Set up the server address struct
    fill_addr_struct(&serverAddress, serverHostInfo, atoi(argv[2]), argv[1]);

    //Set up socket and attempt to connect to the server specified in server address
	set_socket(&socketFD, &serverAddress);
    connect_server(&serverAddress, &socketFD);

    //While we haven't recieved or sent any messages with "\quit", continue sending and receiving messages with the server
    while(1)
    {
    	message = get_message(handle); //Get the message
    	if(strstr(message, "\\quit") != NULL) //Check if the message has "\quit" inside of it
    	{
    		printf("You have terminated the connection\n");
    		send_message(&socketFD, message);
    		break;
    	}
    	send_message(&socketFD, message);
    	if(recv_message(&socketFD) == 0) //If the recieved message contains "\quit" then exit and close the socket
    		break;
	}
	close(socketFD);

	return 0;
}

/*************************************************
 * Function: fillAddrStruct
 * Description: Sets up the server address struct and fills other information like the port number
 * Params: address of sockaddr_in struct, hostent struct
 * Returns: none
 * Pre-conditions: proper addresses and arguments are passed in
 * Post-conditions: Server address struct is filled. Exit if errors.
 * **********************************************/
void fill_addr_struct(struct sockaddr_in* serverAddress, struct hostent* serverHostInfo, int port_number, char* hostname)
{
    //Clear out address struct and obtain port number from command line
    memset((char*)serverAddress, '\0', sizeof(serverAddress));

    //Create network capable socket
    serverAddress->sin_family = AF_INET;
    //Store port number and convert from LSB to MSB form
    serverAddress->sin_port = htons(port_number);
    //Convert the machine name into a special form of address
    serverHostInfo = gethostbyname(hostname);

    //Copy in the address
    if(serverHostInfo == NULL)
    {
        fprintf(stderr, "chatclient error: no such host\n");
        exit(0);
    }
    //Save data
    memcpy((char*)&serverAddress->sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
}

/*************************************************
 * Function: setSocket
 * Description: sets up socket for communication with server
 * Params: address of socket file descriptor var, address of server address struct
 * Returns: none
 * Pre-conditions: correct arguments passed in
 * Post-conditions: socket file descriptor is set and exits on error
 * **********************************************/
void set_socket(int* socketFD, struct sockaddr_in* serverAddress)
{
    //Fill socketFD
    *socketFD = socket(AF_INET, SOCK_STREAM, 0);
    //If a bad file descriptor is given, exit with an error
    if(*socketFD < 0)
    {
        close(*socketFD);
        error("chatclient error: opening socket", 1);
    }
}

/*************************************************
 * Function: connectServer
 * Description: Establishes a connection to a server
 * Params: address of serverAddress struct, address of socket file descriptor
 * Returns: none
 * Pre-conditions: server address and socket file descriptor are correctly filled
 * Post-conditions: Client either establishes connection with server or gives an error
 * **********************************************/
void connect_server(struct sockaddr_in* serverAddress, int* socketFD)
{
    //Establish connection, exit with status 2 if there was an error connecting
    if(connect(*socketFD, (struct sockaddr*)serverAddress, sizeof(*serverAddress)) < 0)
    {
        error("chatclient error: connecting", 2);
    }
    printf("Connection made!\n");
}

/*************************************************
 * Function: send_message 
 * Description: sends content of message given via the socket to the server
 * Params: address of socket file descriptor, message to be sent
 * Returns: none
 * Pre-conditions: socket file descriptor is valid
 * Post-conditions: Contents of message are sent or program exits with error message
 * **********************************************/
void send_message(int* socketFD, char* message)
{
    int charsWritten, message_length;
    message_length = strlen(message);

    //Send message contents
    charsWritten = send(*socketFD, message, message_length, 0);
    if(charsWritten < 0)
    {
        error("chatclient error: writing to socket", 1);
    }
    if(charsWritten < message_length)
    {
        error("chatclient warning: Not all data written to socket!\n", 1);
    }
}

/*************************************************
 * Function: recv_message
 * Description: Recieves text from the server and outputs it to stdout
 * Params: address of socket file descriptor
 * Returns: 1 for no quit message and 0 for quit message
 * Pre-conditions: socket file descriptor is open and correct
 * Post-conditions: message has been fully recieved and sent to stdout
 * **********************************************/
int recv_message(int *socketFD)
{
    //Initialize buffer large enough to hold 1024 bytes if needed
    char message[1024];
    int charsRead;

    //Clean file message
    memset(message, '\0', 1024);

    charsRead = recv(*socketFD, message, sizeof(message)-1, 0);

    //Check if the message we recieved contains "\quit" and if so return 0
    if(strstr(message, "\\quit") != NULL)
    {
    	printf("Connection terminated by peer\n");
    	return 0;
    }

    printf("%s\n", message);
    return 1;
}

/*************************************************
 * Function: get_message
 * Description: Gets a message from standard input, appends the handle to it then returns the combined string
 * Params: User handle string
 * Returns: Message with handle at beginning
 * Pre-conditions: User handle has been set and is valid (At most 10 characters with no spaces plus the "> ")
 * Post-conditions: Message has memory allocated for it and is returned successfully with the handle attached to the beginning
 * **********************************************/
char* get_message(char* handle)
{
	//Initialize variables
	char* message = NULL;
	char buffer[1024] = {'\0'};
	int buffer_len = 0;

	printf(handle);//Prints the handle as a sort of prompt before the message is sent
	strcpy(buffer, handle); //Add the handle to the beginning of the buffer
	buffer_len = strlen(buffer);
	fgets(&buffer[buffer_len], 1024, stdin); //Get the message from stdin up to a maximum of 1024 bytes

	buffer_len = strlen(buffer);
	buffer[buffer_len-1] = '\0'; //Replace the newline at the end of buffer with a null terminator

	//Allocate memory for the message and copy buffer to message
	message = (char*)malloc(sizeof(handle) + (strlen(buffer)*sizeof(char)));
	memcpy(message, buffer, sizeof(char)*buffer_len);

	return message;
}

/*************************************************
 * Function: get_handle
 * Description: Gets the handle from the user via stdin
 * Params: None
 * Returns: User handle string
 * Pre-conditions: None
 * Post-conditions: User handle string is at most 10 characters with no whitespace and has "> " at the end
 * **********************************************/
char* get_handle()
{
	//Initialize variables
	char* handle = NULL;
	char name[13] = {'\0'};

	//While the handle is valid
	while(1)
	{
		printf("Enter a user handle: ");
		fgets(name, 12, stdin);
		if(strstr(name, " ") != NULL)
			printf("Invalid user handle: No whitespace\n");
		else
			break;
	}
 
 	//Add a "> " to the end of the handle
	int name_len = 0;
	name_len = strlen(name);
	name[name_len-1] = '>';
	name[name_len] = ' ';
	name[name_len+1] = '\0';

	//Allocate memory, copy name to handle and return
	handle = (char*)malloc(sizeof(char)*(name_len + 2));
	memcpy(handle, name, (name_len+1)*sizeof(char));

	return handle;
}

/*************************************************
 * Function: error
 * Description: prints specified error message and exits with specified code
 * Params: error message, exit code
 * Returns: none
 * Pre-conditions: valid exit code given
 * Post-conditions: exits process and prints error message to stderr
 * **********************************************/
void error(const char *msg, int n)
{
    perror(msg);
    exit(n);
}