/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - otp_enc_d.c
 COMMENT:
	
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 100000
int status; 
int filenameFlag=0;
int messageFlag=0;


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
void encryptMessage(char *file, char *key, char *encrypt)
{

}

/*******************************
			MAIN
*******************************/
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, r,spawnpid;
	char key[MAX], file_name[MAX], total_msg[MAX], read_buffer[MAX], buffer[MAX];
	char total_msgv2[MAX], read_bufferv2[MAX];
	char keyv3[MAX], filev3[MAX];
	char encrypt[MAX];

	//clear buffers of garbage data
	memset(encrypt, '\0', sizeof(encrypt));
	memset(key, '\0', sizeof(key));
	memset(file_name, '\0', sizeof(file_name));

	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) 
	{ 
		fprintf(stderr,"USAGE: %s port\n", argv[0]);
		exit(1); 
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
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

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

			//Cite OSU CS344 Lectures 

			memset(total_msg, '\0', sizeof(total_msg)); // Clear the buffer

			int index=0;
			while (strstr(total_msg, "%") == NULL) // As long as we haven't found the terminal...

		{

				memset(read_buffer, '\0', sizeof(read_buffer)); // Clear the buffer

				r = recv(establishedConnectionFD, read_buffer, sizeof(read_buffer) - 1, 0); // Get the next chunk
			
strcat(total_msg, read_buffer); // Add that chunk to what we have so far
//				printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", read_buffer, total_msg);
				if (r == -1) { printf("r == -1\n"); break; } // Check for errors
				if (r == 0) { printf("r == 0\n"); break; }
	index++;
			}


//check if the file is coming from enc. enc will always send a ~ first
if(total_msg[0]!= '~')
{
//if the message doest not begin with ~ then it is NOT from enc, so reject it 

				send(establishedConnectionFD, "!", 1, 0);
//send ! to dec which will make dec print an error message and then exit 

}

//start the  index at 1 to ignore the ~
int j=0;
 index=1;
//traverse until you hit the first delimiter that ends the key 
while(total_msg[index]!='@')
{

keyv3[j]= total_msg[index];
//printf("%c %c", keyv3[index], total_msg[index]);

index++;j++;//increment both indice trackers 
}


//printf("key  is %s\n",keyv3);

//move index forward one to ignore very next character not part of message

index++; 
int k=0;

//keep going until you hit the next delimiter that ends the message 

while (total_msg[index]!= '%')

{

filev3[k]=total_msg[index];

//printf("%c %c", filev3[k], total_msg[index]);

index++; k++; //increment boith indice trackers
}


//printf("filename  is %s\n",filev3);

filenameFlag=1;
messageFlag=1; 


//char* usableChars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	memset(encrypt, '\0', sizeof(encrypt)); // Clear the buffer


//Cite:Encrypt idea from piazza post https://piazza.com/class/jf1c8stzym5f8?cid=213

if (filenameFlag && messageFlag)
{
			int i, temp; 
			//start i at one to ignore the sentinel
	for (i = 0;i < strlen(filev3)-1; i++)

	{
//remap space to left bracket value 
	if(filev3[i] ==32)
{

filev3[i] =91;

}
//remap space to left brack value 
if (keyv3[i]== 32)
{

keyv3[i]= 91;
}			


//subtract 65 from the key and file so they  have ascii value of 0-26 
temp= (filev3[i]-65) +(keyv3[i]-65);


// modulus encryption and  shift letterse back to their normal values by adding 65
encrypt[i] = 65 +(temp % 27);

//remap brackets back to spaces 
if (encrypt[i] == 91)
{
encrypt[i] = 32;

}


	}
encrypt[i]='\0';
//printf("encrypted text %s\n",encrypt);

//send encrypted text to client 
charsRead= send(establishedConnectionFD, encrypt, strlen(encrypt), 0);


			if (charsRead< 0) error("error sending to socket");
//close child socket
			close(establishedConnectionFD);
			exit(0);
}

			break; 



		default: 
			waitpid(spawnpid, &status, 0); //parent will clean up child process
			//close(newSocket);

		}

	}


	close(listenSocketFD); // Close the listening socket
	return 0; 
}
