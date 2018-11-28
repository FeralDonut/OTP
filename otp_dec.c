/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - otp_dec.c
 COMMENT:
	this program will connect to otp_dec_d and will ask it to decrypt ciphertext using a passed-in ciphertext and key
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


#define MAX  1000000

/*
 NAME
    error
 SYNOPSIS
    Pointer to a string 
 DESCRIPTION
    Takes in an error message and sends it to stderr and then poltiely exits
 RESOURCE
 	Lecture videos
*/
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

/*
 NAME
    checkFile
 SYNOPSIS
    takes in a pointer to a file name 
 DESCRIPTION
    Opens the file for reading and ensures the file contains only valid characters
    Here those are space and A-Z
 RESOURCE
 	https://www.tutorialspoint.com/c_standard_library/c_function_ftell.htm
 	https://www.geeksforgeeks.org/fseek-in-c-with-example/
*/
char *checkFile(char *file_name) 
{
	//ascii numbers for space and A-Z
	int ascii_val[27] = { 32, 65, 66, 67, 68, 69,	
								 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,	
								 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90};
	int i, temp;
	

	FILE *file = fopen(file_name, "r");
	char *decrypt;
	size_t n = 0;
	int c, checks_out=0;

	if (file == NULL) 
		return NULL; 

	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	decrypt = malloc(f_size);

	while ((c = fgetc(file)) != EOF) 
	{
		decrypt[n] = (char)c;
		temp = decrypt[n];

		//make sure character are in our decryption key or newline
		for (i = 0; i < 27; i++)
		{
			if (temp == ascii_val[i]|| temp =='\n')
			{
				checks_out = 1;
			}
		}

			if (!checks_out)
			{
				fprintf(stderr, "ERROR: Invalid characters in file %s\n",file_name);
				exit(1);
			}
		n++; 
	}
	decrypt[n] = '\0';

return decrypt;
}


/*
 NAME
    sendThroughSocket
 SYNOPSIS
    takes in a socketFD, a pointer to a file name, and pointer to a buffer 
 DESCRIPTION
    sends file over the socket and repeats until entire file is sent
 RESOURCE
 	Lecture videos
 	CS 372 Project 1 & 2
*/
void sendThroughSocket(int socketFD, char *file, char *buffer)
{
	int chars_written = 0;
	while (chars_written< strlen(file))
	{
		chars_written = send(socketFD, file, strlen(file),0);

		if (chars_written < 0) 
			error("ERROR writing to socket");
		if (chars_written < strlen(buffer)) 
			printf("Not all data written to socket!\n");
	}
}



/*******************************
			MAIN
*******************************/
int main(int argc, char *argv[])
{

	int socketFD, portNumber, chars_written, chars_read, chars_writtenv2;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX];
	char checker[3];    
	char decrypt_msg[MAX];
	memset(decrypt_msg,'\0', sizeof(decrypt_msg));
	 // Check usage & args
	if (argc < 4) 
	{ 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	}
																					 
	//inspect for bad characters																					 
	char* file_name = checkFile(argv[1]);
	char* key = checkFile(argv[2]);

	//check if the key length
	if (strlen(file_name) >  strlen(key))
	{
		fprintf(stderr, "Key is too short\n");
		exit(1);
	}

	//add in deliminators
	strcat(file_name, "%");
	strcat(key, "@");

	//Server address struct
	//Reference: Lecture 4.3 and CS 372 Project 1&2
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[3]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverHostInfo = gethostbyname("localhost"); 

	if (serverHostInfo == NULL) 
	{ 
		fprintf(stderr, "ERROR, no such host\n"); 
		exit(0); 
	}

	// Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 


	//Socket creation and connection
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
		error("ERROR opening socket");
	
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		error("ERROR connecting socket");
	
	sendThroughSocket(socketFD, key, buffer);
	sendThroughSocket(socketFD, file_name, buffer);

	//receive message back
	memset(checker,'\0', sizeof(checker));
	chars_read=recv(socketFD,checker,sizeof(checker)-1,0);
		if (chars_read < 0) error("ERROR reading from socket");

	if(checker[0] =='!')
	{
		//!denotes wrong server was attempted
		fprintf(stderr, "Error: otp_dec cannot use otp_enc_d.\n");
		exit(2);
	}

	//response from server
	memset(buffer, '\0', sizeof(buffer));
	chars_read = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	if (chars_read < 0) error("ERROR reading from socket");

	
	memset(decrypt_msg,'\0', sizeof(decrypt_msg));
	strcat(decrypt_msg,checker);
	strcat(decrypt_msg,buffer); 
 
	printf("%s\n", decrypt_msg);

	close(socketFD); 

	free(file_name);
	free(key);
	return 0;
}
