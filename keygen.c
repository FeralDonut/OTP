/*
 Name: Jose-Antonio D. Rubio
 OSUID: 932962915
 Class: 372-400
 OTP - keygen.c
 COMMENT:	This program creates a key file of specified length. 
			The characters in the file generated will be any of the 
			27 allowed characters, generated using the standard UNIX 
			randomization methods. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char** argv)
{

	int length= atoi(argv[1]);	
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Invalid number of arguments.\n");
	}

	srand(time(NULL));

	

	for (i = 0; i < length; i++)
	{
		//ASCII values for space and A-Z
		int ascii_val[27] = { 32, 65, 66, 67, 68, 69,	
								 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,	
								 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90};

		int skeleton_key = rand() % 27; 

		int skeleton_value = ascii_val[skeleton_key];
		//convert to char
		fprintf(stdout, "%c", skeleton_value);
	}

	fprintf(stdout, "\n");


    return 0;
}

