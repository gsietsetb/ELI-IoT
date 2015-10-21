#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define LOGIN_OK "index.cgi?page=mails&idsession="

unsigned int timerandom()
{
	int seconds = (int) time(NULL);
	int number = rand();
	return abs(seconds*number) % 32767; //màxim positiu d'un signed int de 32 bits.
}

int main(int argc, char** argv)
{
	char idsession[17];
	char* target = malloc(strlen(LOGIN_OK)+16+1);
	
	for(int times=0; times<50; times++)
	{	
		printf("Random 16 char alphanumeric string:\t");
		for(int i=0; i<16; i++)
		{
			unsigned int type = timerandom()%3; // number, uppercase or lowercase
			unsigned int digit = timerandom()%26;
			if (type == 0)
				digit = digit%10 + '0';
			else if (type == 1)
				digit += 'A';
			else
				digit += 'a';
			
			idsession[i] = digit;
			printf("%c", digit);
		}
		idsession[16] = '\0';
		printf("\t");
		
		strcpy(target, LOGIN_OK);
		strcat(target, idsession);
		printf("%s", target);
		
		printf("\n");
	}
	
	free(target);
}
