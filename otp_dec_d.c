/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - otp_dec_d.c
 COMMENT:
	this program connects with otp_dec.c
	decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext again
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
    decryptMessage
 SYNOPSIS
    Pointer to two strings and a buffer to hold the decryption
 DESCRIPTION
    Uses key to decrypt file message with ascii manipulation
 RESOURCE
 	Lecture videos
 	Assignment specs
 	Ascii table
*/
void decryptMessage(char *file, char *key, char *decrypt)
{
	int i, temp;
	for (i = 0;i < strlen(file)-1; i++)
		{
			temp=0;
			
			//kept getting open brackets showing up when running
			//shifting spaces to open brack to shift back later
			if(file[i] == 32)
				file[i]= 91;
			if (key[i] == 32)
				key[i]=91;


			//decrypt by reversing the encryption method and setting it to a temp place holder
			temp= (file[i]-65)-(key[i]-65);

			//if temp is a negative number off set by 27
			if (temp<0)
				temp+=27;
			
			//shift letters back by adding 65. 
			decrypt[i]=65+temp;

			//shift open bracket back to space 
			if (decrypt[i] == 91)
				decrypt[i]= 32;
		}

		//add \0 character to string
		decrypt[i]= '\0';
}


/*******************************
			MAIN
*******************************/
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, r,spawnpid, status;
	char key[MAX], file[MAX], total_msg[MAX], read_buffer[MAX], buffer[MAX], decrypt[MAX];
	
	socklen_t client_addr_size;
	struct sockaddr_in serverAddress, clientAddress;

	memset(key, '\0', sizeof(key));
	memset(file, '\0', sizeof(file));

	if (argc < 2) 
	{
		fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); 
	} 

	//Create address struct
	//Reference: Lecture 4.3 and CS 372 Project 1&2
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[1]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

	//Socket creation, listen, and bind
	//Reference Lecture 4.3
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) 
		error("ERROR opening socket");
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
		error("ERROR binding socket");
	//receive up to 5 socket connections
	listen(listenSocketFD, 5); 

	while(1)
	{
		client_addr_size = sizeof(clientAddress); 
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &client_addr_size); 

		if (establishedConnectionFD < 0) 
			error("ERROR on accept\n");

		spawnpid = fork();
		switch (spawnpid)
		{
			case -1: 
				perror("Hull Breach!\n");
				exit(1);
				break; 
			case 0: //child
				memset(total_msg, '\0', sizeof(total_msg)); 

				//parse through message until you hit NULL
				while (strstr(total_msg, "%") == NULL) 
				{

					memset(read_buffer, '\0', sizeof(read_buffer)); 
					r = recv(establishedConnectionFD, read_buffer, sizeof(read_buffer) - 1, 0); 
				
					//build the complete message up chunk by chunk
					strcat(total_msg, read_buffer);
					
					//check for errors				
					if (r == -1) 
					{
						printf("r == -1\n"); 
						break; 
					}
					if (r == 0) 
					{ 
						printf("r == 0\n"); 
						break; 
					}
				}
				//check if the file is coming from enc by checking for ~ in first slot
				//and send ! for not allowed
				if(total_msg[0]== '~')
					send(establishedConnectionFD, "!", 1, 0);

				int i=0;
				//parse through until we hit the key deliminater
				while(total_msg[i]!='@') 
				{
					key[i]= total_msg[i];
					i++;
				}
				//move it forward one 
				i++; 
				int k=0;
				//parse through until we hit the message delimiter 
				while (total_msg[i]!= '%')
				{
					file[k]=total_msg[i];
					i++; 
					k++;
				}


				memset(decrypt, '\0', sizeof(decrypt)); 
					
				decryptMessage(file, key, decrypt);
				
				//send decrypted message back through socket
				charsRead= send(establishedConnectionFD, decrypt, strlen(decrypt), 0);

				if (charsRead< 0) 
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
