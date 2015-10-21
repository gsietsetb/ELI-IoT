/*#include "urlcoding.h"
#include <stdlib.h>
#include <errno.h>*/
#include <stdio.h>
#include <string.h>
//#include <unistd.h>

typedef struct{ 
	char nick[20];
	char passwd[20];
}user;

int main(){ 
	user dildo;
	
	printf("Nom d'usuari: ");
	scanf("%s",dildo.nick);
	
	printf("Contrasenya: ");
	scanf("%s",dildo.passwd);

	//strcpy(dildo.nick, "joan");
	//strcpy(dildo.passwd, "latincpetita");
	
	FILE *fp;
	fp = fopen("BD", "a"); //r-w-a
	fprintf(fp, "%s:%s\n", dildo.nick, dildo.passwd);
	fclose(fp);

	return(0);
}
