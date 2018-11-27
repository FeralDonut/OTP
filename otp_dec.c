/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - keygen.c
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
//Cite CS344 Lectures and Notes

int MAX = 1000000, MAX2 =1000020;
int match = 0; 
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
//Cite https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c

char *decryptFile(char *file_name) {
//the allowable keys are A-Z and space, represented as digits 
	int ascii_val[27] = { 32, 65, 66, 67, 68, 69,	
								 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,	
								 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90};
	int i, temp;
//read file that is passed in 

	FILE *file = fopen(file_name, "r");
	char *decrypt;
	size_t n = 0;
	int c;

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
				match = 1;
			}
		}
//if not match is found that means that there are invalid characters not belonging to the key selection array 
			if (!match)
			{
				fprintf(stderr, "ERROR: Invalid characters in file %s\n",file_name);
				exit(1);
			}
		n++; 
	}
	decrypt[n] = '\0';

return decrypt;
}




int main(int argc, char *argv[])
{

	int socketFD, portNumber, charsWritten, charsRead, charsWrittenv2;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX];
	char stop[3];    
	char final[MAX2];
	
	 // Check usage & args
	if (argc < 4) 
	{ 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	}
																					 
	//inspect for bad characters																					 
	char* fileName = decryptFile(argv[1]);
	char* key = decryptFile(argv[2]);

	//if the files make it to this point there are no bad characters

	//check if the key is too short 
	if (strlen(fileName) >  strlen(key))
	{
		fprintf(stderr, "Key is too short\n");
		exit(1);

	}

	strcat(fileName, "%");
	strcat(key, "@");
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	

charsWrittenv2=0; 
	while (charsWrittenv2<strlen(key))
	{

//send key  to the server 	
	charsWrittenv2 = send(socketFD, key, strlen(key),0); // Write to the server

//error message if failure to send to server or data still needs to be written 

		if (charsWrittenv2 < 0) error("CLIENT: ERROR writing to socket");
		if (charsWrittenv2 < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	}

charsWritten=0;

	while (charsWritten< strlen(fileName))
	{

//send file to the server 

		charsWritten = send(socketFD, fileName, strlen(fileName),0); // Write to the server

//error message sent if failure to send to server of if data is not done sending 

		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	
	}

//receive message from server
memset(stop,'\0', sizeof(stop));
charsRead=recv(socketFD,stop,sizeof(stop)-1,0);
	if (charsRead < 0) 
		error("CLIENT: ERROR reading from socket");

if(stop[0] =='!')
{
//if message received contains ! that means the client is trying to connect to wrong server
 fprintf(stderr, "Error: otp_dec cannot use otp_enc_d. Port number: %d\n", portNumber);

exit(2);
}


// Get return message from server

	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

// strcat messages from both buffer stop and buffer to print out the concatenated message
memset(final,'\0', sizeof(final));
strcat(final,stop);
strcat(final,buffer); 

//final message print 
	printf("%s\n", final);

	close(socketFD); // Close the socket
	return 0;
}
