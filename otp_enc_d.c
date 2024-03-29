/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - otp_enc_d.c
 COMMENT:
	this program connects with otp_enc.c
	encrypt plaintext it is given and returns ciphertext
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 100000


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
void error(const char*msg) {perror(msg); exit(1);}

/*
 NAME
    encryptMessage
 SYNOPSIS
    Pointer to two strings and a buffer to hold the decryption
 DESCRIPTION
    Uses key to decrypt file message with ascii manipulation
 RESOURCE
 	Lecture videos
 	Assignment specs
 	Ascii table
*/
void encryptMessage(char *file_msg, char *key, char *encrypt)
{
	int i, temp; 
	//start i at one to ignore the sentinel
	for (i = 0;i < strlen(file_msg)-1; i++)
	{
		//kept getting errors with the space char
		//shifting spaces to open brack to shift back later
		if(file_msg[i] ==32)
			file_msg[i] =91;
		if (key[i]== 32)
			key[i]= 91;

		//convert to ascii values of 0-26 by subtracting by 65
		temp= (file_msg[i]-65) +(key[i]-65);
		// modulus encryption and shift letterse back to their normal values by adding 65
		encrypt[i] = 65 +(temp % 27);

		//shift open bracket back to space
		if (encrypt[i] == 91)
			encrypt[i] = 32;
	}
	encrypt[i]='\0';

}


/*******************************
			MAIN
*******************************/
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, chars_read, r,spawnpid, status;
	char key[MAX], total_msg[MAX], read_buffer[MAX], file_msg[MAX], encrypt[MAX];

	memset(key, '\0', sizeof(key));
	memset(total_msg, '\0', sizeof(total_msg));

	socklen_t client_addr_size;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections



		// Accept a connection, blocking if one is not available until one connects

	while(1)
	{
		client_addr_size = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &client_addr_size); // Accept

		if (establishedConnectionFD < 0) error("ERROR on accept");

		//Fork to create child process 
		spawnpid = fork();

		switch (spawnpid)
		{
		case -1: 
			perror("Failed to create process");
			exit(1);
			break; 

		case 0: //child
			memset(total_msg, '\0', sizeof(total_msg)); // Clear the buffer
			int i=0;
			while (strstr(total_msg, "%") == NULL) // As long as we haven't found the terminal...
			{

				memset(read_buffer, '\0', sizeof(read_buffer)); // Clear the buffer
				r = recv(establishedConnectionFD, read_buffer, sizeof(read_buffer) - 1, 0); // Get the next chunk
				strcat(total_msg, read_buffer); // Add that chunk to what we have so far
				if (r == -1) 
				{ 
					printf("r == -1\n"); 
					break; 
				} // Check for errors
				if (r == 0) 
				{ 
					printf("r == 0\n");
					break; 
				}
			i++;
			}


			//otp_enc messages are marked with first character being ^ 
			if(total_msg[0]!= '^')
			{
				//if not send message back to trigger not allowed message
				send(establishedConnectionFD, "&", 1, 0);
			}

			//start the  i at 1 to ignore the ~
			int j=0;
			i=1;
			//traverse until you hit the first delimiter that ends the key 
			while(total_msg[i]!='@')
			{
				key[j]= total_msg[i];
				i++;j++;//increment both indice trackers 
			}


			//move our index one to ignore the first char used to check if message was sent by otp_enc
			i++; 
			int k=0;

			while (total_msg[i]!= '%')
			{
				file_msg[k]=total_msg[i];
				i++; 
				k++; 
			}
			
			memset(encrypt, '\0', sizeof(encrypt)); 
			encryptMessage(file_msg, key, encrypt);
			//send encrypted text
			chars_read= send(establishedConnectionFD, encrypt, strlen(encrypt), 0);

			if (chars_read< 0) 
				error("error sending to socket");
			//close child socket
			close(establishedConnectionFD);
			exit(0);
			break; 
		default: 
			waitpid(spawnpid, &status, 0); 
		}
	}

	close(listenSocketFD); 
return 0; 
}
