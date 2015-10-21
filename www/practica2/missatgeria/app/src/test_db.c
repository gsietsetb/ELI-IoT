#include <stdio.h>
#include "strlist.h"
#include "dbc.h"

int main()
{	
	printf("Creant base de dades...\n");
	// Crea la BD	
	if (DB_create_db("db") == -1)
		return 0; //surt si ja existeix
	
	printf("No existia. ");
	DBConnection dbc = DBC_open("db");
	printf("Creada\n");
	
	// Crea la taula users
	printf("Creant taula users...\n");
	StrList list = StrList_create();
	printf("Creada\n");
	StrList_add(list, "user_id");
	StrList_add(list, "user_name");
	StrList_add(list, "passwd");

	printf("Creant taula...\n");	
	DBC_create_table(dbc, "users", list);
	printf("Creada\n");

	// Afegeix 2 usuaris inicials
	printf("Usuaris inicials...\n");
	StrList_clear(list); //root
	StrList_add(list, "0");
	StrList_add(list, "root");
	StrList_add(list, "root");

	DBC_insert(dbc, "users", list);
	printf("Afegit\n");
	
	StrList_clear(list); //paco
	StrList_add(list, "1");
	StrList_add(list, "paco");
	StrList_add(list, "patata");
		
	DBC_insert(dbc, "users", list);
	printf("Afegit\n");
	
	// Crea la taula sessions
	printf("Creant taula users...\n");
	StrList_clear(list);
	StrList_add(list, "session_id");
	StrList_add(list, "user_id");
	
	DBC_create_table(dbc, "sessions", list);
	printf("Creada\n");

	// Crea la taula mails
	printf("Creant taula mails...\n");
	StrList_clear(list);
	
	StrList_add(list, "mail_id");
	StrList_add(list, "from");
	StrList_add(list, "to");
	StrList_add(list, "content");
	
	DBC_create_table(dbc, "mails", list);
	printf("Creada\n");
	
	printf("Afegint mail de prova...\n");
	StrList_clear(list);
	StrList_add(list, "0");
	StrList_add(list, "root");
	StrList_add(list, "paco");
	StrList_add(list, "Benvingut al sistema cutroide de mails! :D");
	DBC_insert(dbc, "mails", list);
	
	printf("Afegit\n");
	
	// Crea taula comptadors
	printf("Creant taula comptadors...\n");
	StrList_clear(list);
	StrList_add(list, "users");
	StrList_add(list, "mails");
	StrList_add(list, "sessions");

	DBC_create_table(dbc, "counters", list);
	printf("Creada\n");
	
	printf("Afegint valors...\n");
	StrList_clear(list);
	StrList_add(list, "2"); // users
	StrList_add(list, "1"); // mails
	StrList_add(list, "0"); // sessions
	DBC_insert(dbc, "counters", list);
	
	printf("Afegits\n");
	
	// allibera variables
	StrList_destroy(list);
	DBC_close(dbc);
	
	
	printf("\n--------------------\n");
	 
	 
	// Primpxthadyor_69
	
	DBConnection dbc2 = DBC_open("db");
	
	//Usuaris		
	printf("\n\n> Usuaris:\n");
	printf("\nuser_id\tuser_name\tpasswd\n");
	DBC_select(dbc2,"users", NULL, NULL);
	while(DBC_next(dbc2) == 0)
		printf("%s\t%s\t%s\n",DBC_get(dbc2, "user_id"),DBC_get(dbc2, "user_name"),DBC_get(dbc2, "passwd"));
	 
	//Sessions
	printf("\n\n> Sessions: ¿Buida?\n");
	printf("session_id\tuser_id\n");
	DBC_select(dbc2,"sessions", NULL, NULL);
	while(DBC_next(dbc2) == 0)
		printf("%s\t%s\n",DBC_get(dbc2, "session_id"),DBC_get(dbc2, "user_id"));

	//Mails
	printf("\n\n> Mails\n");
	printf("mail_id\tfrom\tto\tcontent\n");
	DBC_select(dbc2,"mails", NULL, NULL);
	while(DBC_next(dbc2) == 0)
		printf("%s\t%s\t%s\t%s\n",DBC_get(dbc2, "mail_id"),DBC_get(dbc2, "from"),DBC_get(dbc2, "to"),DBC_get(dbc2, "content"));
	
	//Comptadors
	printf("\n\n> Comptadors:\n"); 
	DBC_select(dbc2,"counters", NULL, NULL);
	printf("users\tmails\tsessions\n");
	while(DBC_next(dbc2) == 0)
		printf("%s\t%s\t%s\n",DBC_get(dbc2, "users"),DBC_get(dbc2, "mails"),DBC_get(dbc2, "sessions"));
	
	DBC_close(dbc2);
	
	return 0;
}















