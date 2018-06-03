////////////////////////////////////////////////////////////////////
// Jonathan Jones (jonesjon 932709446)
// Project 2
// CS372 Spring2018
/////////////////////////////////////////////////////////////////////

//Using modified sections of my code from CS344 Operating Systems I

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER_SIZE 256

void fill_control_addr_struct(struct sockaddr_in*, int*);
void fill_data_addr_struct(struct sockaddr_in*, struct hostent*, int, char*);
void set_control_socket(int*, struct sockaddr_in*);
void set_data_socket(int*);
void connect_data(struct sockaddr_in*, int*);
void accept_connection(socklen_t*, struct sockaddr_in*, int*, int*);
void file_transfer_protocol(int*, struct sockaddr_in, char*, int);
void send_message(int*, const char*);
void send_directory(struct sockaddr_in, int, int, char*);
void send_file(int*, struct sockaddr_in, int, int, char*, char*);
void error(const char*, int);

int main(int argc, char** argv)
{
    //Check usage
    if(argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    //Initialize variables
    int listen_socketFD, established_connectionFD, port_number;
    socklen_t size_of_client_info;
    struct sockaddr_in server_address, client_address;
    
    port_number = atoi(argv[1]);

    //Set up server info
    fill_control_addr_struct(&server_address, &port_number);

    //Set up socket for listening from
    set_control_socket(&listen_socketFD, &server_address);

     printf("Server open on %d\n", port_number);

    //Wait for a client to connect
    while(1)
    {
        //Accept a connection, blocking if one is not available until one connects
        accept_connection(&size_of_client_info, &client_address, &listen_socketFD, &established_connectionFD);

        //Get name of host that connected
        char host_name[256] = {0};
        gethostname(host_name, 255);

        printf("Connection from %s\n", host_name);

        file_transfer_protocol(&established_connectionFD, client_address, host_name, port_number);

        close(established_connectionFD); //Close socket
    }
    return 0;
}

/*************************************************
 * Function: fill_control_addr_struct
 * Description: Sets up the control address struct and fills other information like the port number
 * Params: address of sockaddr_in struct, address of port number integer
 * Returns: none
 * Pre-conditions: proper addresses and arguments are passed in
 * Post-conditions: Control address struct is filled. Exit if errors.
 * **********************************************/
void fill_control_addr_struct(struct sockaddr_in* server_address, int* port_number)
{
    //Clear out address struct
    memset((char*)server_address, '\0', sizeof(server_address));

    //Create network capable socket
    server_address->sin_family = AF_INET;
    //Store port number and convert from LSB to MSB form
    server_address->sin_port = htons(*port_number);
    //Allow any address for connection
    server_address->sin_addr.s_addr = INADDR_ANY;
}

/*************************************************
 * Function: fill_data_addr_struct
 * Description: Sets up the data address struct and fills other information like the port number
 * Params: address of sockaddr_in struct, hostent struct, portnumber, hostname
 * Returns: none
 * Pre-conditions: proper addresses and arguments are passed in
 * Post-conditions: Data address struct is filled. Exit if errors.
 * **********************************************/
void fill_data_addr_struct(struct sockaddr_in* server_address, struct hostent* data_host_info, int port_number, char* host_name)
{
    //Clear out address struct and obtain port number from command line
    memset((char*)server_address, '\0', sizeof(server_address));

    //Create network capable socket
    server_address->sin_family = AF_INET;
    //Store port number and convert from LSB to MSB form
    server_address->sin_port = htons(port_number);
    //Convert the machine name into a special form of address
    data_host_info = gethostbyname(host_name);

    //Copy in the address
    if(data_host_info == NULL)
    {
        fprintf(stderr, "ftserver error: no such host\n");
        exit(0);
    }
    //Save data
    memcpy((char*)&server_address->sin_addr.s_addr, (char*)data_host_info->h_addr, data_host_info->h_length);
}

/*************************************************
 * Function: set_control_socket
 * Description: sets up control socket for communication with client
 * Params: address of listening socket file descriptor var, address of server address struct
 * Returns: none
 * Pre-conditions: correct arguments passed in
 * Post-conditions: listening socket file descriptor is set and exits on error
 * **********************************************/
void set_control_socket(int* listen_socketFD, struct sockaddr_in* server_address)
{
    //Fill listening file descriptor
    *listen_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(*listen_socketFD < 0)
    {
        error("ftserver error: opening socket", 1);
    }
    if(bind(*listen_socketFD, (struct sockaddr *)server_address, sizeof(*server_address)) < 0)
    {
        error("ftserver error: on binding", 1);
    }

    //Listen for a connection
    listen(*listen_socketFD, 5);
}

/*************************************************
 * Function: setSocket
 * Description: sets up socket for data transfer with client
 * Params: address of socket file descriptor var
 * Returns: none
 * Pre-conditions: correct arguments passed in
 * Post-conditions: socket file descriptor is set and exits on error
 * **********************************************/
void set_data_socket(int* data_socketFD)
{
    //Fill socketFD
    *data_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    //If a bad file descriptor is given, exit with an error
    if(*data_socketFD < 0)
    {
        close(*data_socketFD);
        error("ftserver error: opening socket", 1);
    }
}

/*************************************************
 * Function: connect_data
 * Description: Establishes a connection to a client on a data socket
 * Params: address of cliient address struct, address of data socket file descriptor
 * Returns: none
 * Pre-conditions: client address and data socket file descriptor are correctly filled
 * Post-conditions: Server either establishes connection with client or gives an error
 * **********************************************/
void connect_data(struct sockaddr_in* client_address, int* data_socketFD)
{
    //Establish connection, exit with status 2 if there was an error connecting
    if(connect(*data_socketFD, (struct sockaddr*)client_address, sizeof(*client_address)) < 0)
    {
        error("ftserver error: connecting", 2);
    }
}

/*************************************************
 * Function: accept_connection
 * Description: Waits for and accepts any connection on the listening socket provided
 * Params: address of struct that holds size of client info, address for client address struct, 
 * address to listening socket, address to established connection socket
 * Returns: none
 * Pre-conditions: Server has a listening socket
 * Post-conditions: Valid connection has been made and the file descriptors and structs passed in have been changed accordingly
 * **********************************************/
void accept_connection(socklen_t* size_of_client_info, struct sockaddr_in* client_address, int* listen_socketFD, int* established_connectionFD)
{
    //Get size of client info
    *size_of_client_info = sizeof(*client_address);
    //Accept a connection and fill the established connection file descriptor
    *established_connectionFD = accept(*listen_socketFD, (struct sockaddr *)client_address, size_of_client_info);
    //Check for basic errors on accept
    if(*established_connectionFD < 0)
    {
        close(*established_connectionFD);
        fprintf(stderr, "ftserver error: on accept\n");
        exit(1);
    }       
}

/*************************************************
 * Function: file_transfer_protocol
 * Description: Gets a command from the client and either sends a file, sends the directory or gives an error
 * Params: Address of socket, client address sockaddr_in, hostname string, control port number
 * Returns: none
 * Pre-conditions: Control connection with client is already set up.
 * Post-conditions: File or directory has been sent or error has been given
 * **********************************************/
void file_transfer_protocol(int* established_connectionFD, struct sockaddr_in client_address, char* hostname, int control_port)
{
    int charsRead, command_type, data_port, dp_digits;
    char buffer[BUFFER_SIZE] = {0}; //Gets BUFFER_SIZE bytes message size maximum
    memset(buffer, 0, BUFFER_SIZE); //Make sure that buffer is all zeroes

    //Get command from client
    charsRead = recv(*established_connectionFD, buffer, BUFFER_SIZE, 0);
    if(charsRead == 0)
        error("ftserver error: Empty command recieved\n", 1);
    
    //Split command into its components
    command_type = buffer[0]-48; //The first character is a bit indicating whether the flag is -g or -l (Subtract 48 to convert ASCII to number)
    dp_digits = strcspn(buffer, "@") - 1; //Look for the control character in the command and determine how many digits the data port number is

    char* substring = (char*)malloc(dp_digits); //Allocate memory for the data port string that will be converted to a number
    sprintf(substring, "%.*s", dp_digits, buffer + 1); //Extract from the buffer starting at the second character the data port number
    data_port = atoi(substring); //Convert data port number to an integer
    memset(substring, '\0', dp_digits); //Clean up data inside substring
    free(substring); //Free substring memory

    //Check command type
    if(command_type) //If command is -g then we'll need the file name
    {
        //Get the filename
        int len = strlen(&(buffer[dp_digits+2])); //Filename starts after the control character which is at position dp_digits + 2
        char* filename = (char*)malloc(len);
        memcpy(filename, &(buffer[dp_digits+2]), len);

        //Send the file to the client (This isn't "sendfile", its my own function)
        send_file(established_connectionFD, client_address, control_port, data_port, hostname, filename);

        //Make sure to free and clean up the memory that the filename variable used
        memset(filename, '\0', len);
        free(filename);
    }
    else //If the command was for -l
    {
        //Send directory to the client
        send_directory(client_address, control_port, data_port, hostname);
    }
}

/*************************************************
 * Function: send_directory
 * Description: Sends the contents of the current working directory to the client
 * Params: Client address sockaddr_in, control port number, data port number, host name
 * Returns: none
 * Pre-conditions: Connection with client is setup already
 * Post-conditions: Directory has been sent or error has given
 * **********************************************/
void send_directory(struct sockaddr_in client_address, int control_port, int data_port, char* hostname)
{
    printf("List directory requested on port %d\n", data_port);

    //Initialize variables
    int dataSocketFD;
    struct sockaddr_in dataAddress;
    struct hostent* dataHostInfo;
    DIR* directory;
    struct dirent* dir_file;

    //Set up connection to the data port on the client
    fill_data_addr_struct(&dataAddress, dataHostInfo, data_port, inet_ntoa(client_address.sin_addr));
    set_data_socket(&dataSocketFD);
    connect_data(&dataAddress, &dataSocketFD);

    directory = opendir("./"); //Open the cwd for reading

    if(directory == NULL) //Failure to open directory
    {
        fprintf(stderr, "ftserver: error opening directory\n");
        return;
    }

    printf("Sending directory contents to %s:%d\n", hostname, data_port);

    //Iterate through the contents of the directory and send it to the client one name at a time
    while((dir_file = readdir(directory)) != NULL)
    {
        send_message(&dataSocketFD, dir_file->d_name); //Send each directory item
        send_message(&dataSocketFD, "\n");
    }

    //Send an EOT character to tell the client to stop recieving
    char end_transmission  = 4;
    send_message(&dataSocketFD, &end_transmission);
    closedir(directory); //Close the directory
}

/*************************************************
 * Function: send_file
 * Description: Sends a file to the client.
 * Params: Control connection socket, client address information, control port number, data port number, host name, file name
 * Returns: none
 * Pre-conditions: Control connection already set up with client.
 * Post-conditions: File has been sent or file not found sent
 * **********************************************/
void send_file(int* established_connectionFD, struct sockaddr_in client_address, int control_port, int data_port, char* hostname, char* filename)
{
    char buffer[BUFFER_SIZE] = {0}; //Holds the contents of the file as we send it
    memset(buffer, 0, BUFFER_SIZE); //Initialize contents of buffer to zeroes
    
    printf("File \"%s\" requested on port %d\n", filename, data_port);

    FILE *f = fopen(filename, "r"); //Open up filename
    if(f == NULL) //File has not been found
    {
        printf("File not found. Sending error message to %s:%d\n", hostname, control_port);
        send_message(established_connectionFD, "0"); //Tell client that the file has not been found
    }
    else
    {
        send_message(established_connectionFD, "1"); //Tell client that the file has been found

        //Initialize variables
        int dataSocketFD;
        struct sockaddr_in dataAddress;
        struct hostent* dataHostInfo;

        //Set up connection with data port on the client
        fill_data_addr_struct(&dataAddress, dataHostInfo, data_port, inet_ntoa(client_address.sin_addr));
        set_data_socket(&dataSocketFD);
        connect_data(&dataAddress, &dataSocketFD);

        printf("Sending \"%s\" to %s:%d\n", filename, hostname, data_port);

        memset(buffer, 0, BUFFER_SIZE);

        //Grab one character at a time from the file and place it into the buffer
        //Once the buffer is full, send it, flush the buffer and repeat until EOF character is reached in the file
        int i = 0;
        char c = fgetc(f);
        while (c != EOF) //While the character is not an end of file character
        {
            if(i == BUFFER_SIZE - 1) //If the buffer is full
            {
                buffer[i + 1] = '\0'; //Add a null terminator the buffer
                send_message(&dataSocketFD, buffer); //Send the buffer
                memset(buffer, 0, BUFFER_SIZE); //Clean the buffer
                i = 0; //Reset the index of the buffer
            }
            buffer[i] = c; //File the buffer with captured character
            c = fgetc(f); //Capture another character
            i++; //Increment buffer index
        }
        if(i != 0) //If the buffer is not empty but we've reached an EOF
        {
            //Append an end of transmission character to the buffer and send one final message
            buffer[i] = 4;
            buffer[i+1] = '\0';
            send_message(&dataSocketFD, buffer);
        }
        fclose(f); //Close the file
        close(dataSocketFD); //Close the data socket
    }
}

/*************************************************
 * Function: send_message 
 * Description: sends content of message given via the socket to the server
 * Params: address of socket file descriptor, message to be sent
 * Returns: none
 * Pre-conditions: socket file descriptor is valid
 * Post-conditions: Contents of message are sent or program exits with error message
 * **********************************************/
void send_message(int* socketFD, const char* message)
{
    int charsWritten, message_length;
    message_length = strlen(message);

    //Send message contents
    charsWritten = send(*socketFD, message, message_length, 0);
    if(charsWritten < 0)
    {
        error("chatclient error: writing to socket\n", 1);
    }
    if(charsWritten < message_length)
    {
        error("chatclient warning: Not all data written to socket!\n", 1);
    }
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