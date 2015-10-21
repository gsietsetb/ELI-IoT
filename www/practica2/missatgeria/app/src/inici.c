#include "urlcoding.h"
#include "request.h"
#include "dbc.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h> 

#define LOGIN_ERR  0
#define HOME       1
#define MAIL_LIST  2
#define MAIL       3
//#define COMPOSE    4

#define ENVIAT 1
#define ERROR -1

#define LOGIN_OK   "index.cgi?page=mail_list&idsession="
#define SHOW_MAIL  "index.cgi?page=mail&idmail="

void view(int page, ...);
int  check_login(Form form);
int  check_session(Form form, char* idsession, int* exists);
void gotomails();
void redirect(const char* target);
int  init_database();
unsigned int timerandom();
char* random_idsession(char* string, unsigned short int len);

Form form;
int main(int argc, char **argv){
	form = check_request();
	init_database();
	DBConnection dbc;
	
	if (form == NULL)
		return 0;
	//GET LOGIN OR SHOW HOMEPAGE
	if (Form_get(form, "login") != NULL){	
		//printf("checking login...\n");
		int login_ok = check_login(form);
		int unique_id = 0;
		char* idsession = NULL;
		char* target = NULL;
		//char target[1000]="hola!";
		int user_already_logged;
		
		
		StrList list = StrList_create();
		
		idsession = check_session(form, idsession, &user_already_logged);
		
		if(user_already_logged){
			//printf("already logged in: copying string for redirection...\n");
			target = malloc((sizeof(char))*(strlen(LOGIN_OK)+strlen(idsession)+1));
			//printf("copy!!\n");
			strcpy(target, LOGIN_OK);
			//printf("strcat!! idsession=%s\n",idsession);
			strcat(target, idsession);
			//printf("redirecting...\n");
			redirect(target);
			free(target);
			free(idsession);
		}else if (login_ok){
			do
			{
				//printf("generating new idsession...\n");
				idsession = random_idsession(idsession, 16);
			
				dbc = DBC_open("db");
				DBC_select(dbc, "sessions", "session_id", idsession);
				
				if ((unique_id = DBC_next(dbc)) == 0)
					free(idsession);
					
			}while( unique_id == 0);
	
			//printf("logging session to DB...\n");
			//printf("`--> idsession = %s\n", idsession);	
			StrList_add(list, idsession);
			StrList_add(list, Form_get(form, "user"));
		
			DBC_insert(dbc, "sessions", list);
			StrList_clear(list);

			//printf("copying string for redirection...\n");
			free(target);
			target = malloc(strlen(LOGIN_OK)+16+1);
			strcpy(target, LOGIN_OK);
			strcat(target, idsession);
			
			//printf("redirecting...\n");
			redirect(target);
			
			free(target);
			free(idsession);
		}
		else
			view(LOGIN_ERR);
		
		return 0;
	}
	
	if (Form_get(form, "idsession") == NULL){	
		view(HOME);
		return 0;
	}
	if (Form_get(form, "page") != NULL){
		if(strcmp(Form_get(form, "page"),"mail_list") == 0){
			if(Form_get(form, "sendmail") == NULL)
				view(MAIL_LIST);
			else{

				// check idsession and get user
				DBC_select(dbc, "sessions", "session_id", Form_get(form, "idsession"));
				if(DBC_next(dbc) == -1)
				    return -1;
				char *user = DBC_get(dbc, "user_id");
			    			  
				// send mail and capture result							    
				int result = sendmail(user,
						Form_get(form, "to"),
						Form_get(form, "subject"),
						Form_get(form, "content"));
				view(MAIL_LIST, result);
			}
			return 0;
		}else if(strcmp(Form_get(form, "page"),"mail") == 0){
			view(MAIL);
			return 0;
		}
		/*else if(strcmp(Form_get(form, "page"),"compose") == 0)
		{
			view(COMPOSE);
			return 0;
		}*/
	}
	
	view(HOME);
	return 0;
}
void view(int page, ...){
	va_list arguments;                     
	
	DBConnection dbc;
	printf(CGI_HEAD);
	printf("\n\n");
	printf(HTML_HEAD);
	printf("\n");
	
	switch(page){
		//Combinació errònia
		case LOGIN_ERR:
		    printf("<p style=\"color:red\">Combinació usuari/contrasenya errònies</p>");
		
		//Pagina Principal
		case HOME:
		    printf("<form method=\"POST\">\
		    Usuari: <input type=\"text\" name=\"user\"><br>\
		    Contrasenya: <input type=\"password\" name=\"passwd\">\
		    <input type=\"submit\" value=\"Entra\" name=\"login\">\
		    </form>\n");
		    break;
		
		//Llistat de mails
		case MAIL_LIST:
		    va_start (arguments, 1);
		    int send_ok = va_arg(arguments, int);
		    va_end(arguments);
		    printf("<h1>MAIL LIST</h1>\n");
		    if (send_ok == ENVIAT)		
			printf("<color=\"green\">S'ha enviat el mail amb èxit!<color><br>");
		    else if(send_ok == ERROR)	
			printf("<color=\"red\">El destinatari no existeix<color><br>");
		    printf("<b>Redacta nou mail</b><br>\
			  <form method=\"POST\">\
			  Destí: <input type=\"text\" name=\"to\"><br>\
			  Subject: <input type=\"text\" name=\"subject\"><br>\
			  Mail: <textarea name=\"mail\" wrap=\"virtual\" rows=\"10\" cols=\"200\"></textarea><br>\
			  <input type=\"hidden\" name=\"idsession\" value=\"%s\">\
			  <input type=\"hidden\" name=\"sendmail\" value=\"true\">\
			  <input type=\"submit\" value=\"Envia\" name=\"sendmail\">\
			  </form>", Form_get(form, "idsession"));
		    printf("<table><tr><th>Remitent</th><th>Assumpte</th></tr>");
		    dbc = DBC_open("db");

		    // get user from session
		    DBC_select(dbc, "sessions", "session_id", Form_get(form, "idsession"));
		    if(DBC_next(dbc) == -1)
			break;
		    char *user = DBC_get(dbc, "user_id");
		    
		    // get mails of the user
		    DBC_select(dbc, "mails", "to", user);
		    while (DBC_next(dbc) != -1)
		    printf("<tr><td>%s</td><td><a href=\"%s%s&idsession=%s\">%s</a></td></tr>",
			    DBC_get(dbc, "from"),
			    SHOW_MAIL,
			    DBC_get(dbc, "mail_id"),
			    Form_get(form, "idsession"),
			    DBC_get(dbc, "subject"));
		    DBC_close(dbc);
		    // all mails printed
		    
		    printf("</table>\n");
		    break;
		
		// Mails    
		case MAIL:
		    dbc = DBC_open("db");
		    DBC_select(dbc, "mails", "mail_id", Form_get(form, "idmail"));
		    if(DBC_next(dbc) == -1)
			    break;
		
		    printf("<h1>MAIL</h1>\n");
		    printf("<a href=\"%s%s\">Torna enrere</a><br>",LOGIN_OK, Form_get(form, "idsession"));
		    printf("<dl>\
			    <dt><b>From</b></dt><dd>%s</dd>\
			    <dt><b>Subject</b></dt><dd>%s</dd>\
			    <dt><b>Mail</b></dt><dd>%s</dd>\
			    </dl>",
			    DBC_get(dbc, "from"),
			    DBC_get(dbc, "subject"),
			    DBC_get(dbc, "content")
		    );
		
		    
		// No hauria de passar...
		default: break;
	}
	printf(HTML_FOOT);
}
int check_login(Form form){
	DBConnection dbc = DBC_open("db");
	DBC_select(dbc,"users","user_name", Form_get(form, "user")); //suposem nom d'usuari únic
	if (DBC_next(dbc) == -1)
	{
		DBC_close(dbc);
		return 0;
	}
	if (strcmp(DBC_get(dbc, "passwd"), Form_get(form, "passwd")) != 0)
	{
		DBC_close(dbc);
		return 0;
	}
	
	DBC_close(dbc);

	return 1;
	
	/*if(strcmp(Form_get(form, "user"), "paco")==0 && strcmp(Form_get(form, "passwd"), "patata")==0 )
		return 1;
	else 
		return 0;*/
}
int check_session(Form form, char* idsession, int* exists){
	//printf("opening database\n");
	DBConnection dbc = DBC_open("db");
	DBC_select(dbc,"sessions","user_id",Form_get(form, "user")); //suposem sessió única
	//printf("user checked\n");
	if (DBC_next(dbc) == -1) //no existeix
	{
		DBC_close(dbc);
	//	printf("user wasn't logged in yet\n");
		*exists = 0;
		return idsession;
	}
	//printf("user was already logged in\n");
	if(idsession != NULL) 
	    free(idsession);
	idsession = malloc(strlen(DBC_get(dbc, "session_id")) + sizeof('\0'));
	strcpy(idsession, DBC_get(dbc, "session_id"));
	//printf("current session id is: %s\n",idsession);
	DBC_close(dbc);

	*exists = 1;
	return idsession;
}
int init_database(){
	// Crea la BD	
	if (DB_create_db("db") == -1)
		return 0; //surt si ja existeix
	
	DBConnection dbc = DBC_open("db");
	
	// Crea la taula users
	StrList list = StrList_create();
	StrList_add(list, "user_id");
	StrList_add(list, "user_name");
	StrList_add(list, "passwd");

	DBC_create_table(dbc, "users", list);

	// Afegeix 2 usuaris inicials
	StrList_clear(list); //root
	StrList_add(list, "0");
	StrList_add(list, "root");
	StrList_add(list, "root");

	DBC_insert(dbc, "users", list);
	
	StrList_clear(list); //paco
	StrList_add(list, "1");
	StrList_add(list, "paco");
	StrList_add(list, "patata");
		
	DBC_insert(dbc, "users", list);
	
	// Crea la taula sessions
	StrList_clear(list);
	StrList_add(list, "session_id");
	StrList_add(list, "user_id");
	
	DBC_create_table(dbc, "sessions", list);

	// Crea la taula mails
	StrList_clear(list);
	
	StrList_add(list, "mail_id");
	StrList_add(list, "from");
	StrList_add(list, "to");
	StrList_add(list, "subject");
	StrList_add(list, "content");
	
	DBC_create_table(dbc, "mails", list);
	
	StrList_clear(list);
	StrList_add(list, "0");
	StrList_add(list, "root");
	StrList_add(list, "paco");
	StrList_add(list, "Benvingut");
	StrList_add(list, "Benvingut al nostre sistema cutroide de mails! :D");
	DBC_insert(dbc, "mails", list);
	
	StrList_clear(list);
	StrList_add(list, "1");
	StrList_add(list, "root");
	StrList_add(list, "paco");
	StrList_add(list, "Colegueo");
	StrList_add(list, "<p>Que m'agradaria provar el meu sistema amb tu, m'ajudes una mica?<br>Gràcies!</p>\
	    <p>Et copio un totxo vale?</p>\
	    <p>La iniciativa CDIO és un marc educatiu innovador per a la formació d'enginyeres i enginyers del segle XXI. En el marc CDIO es proporciona als estudiants un ensenyament dels conceptes fonamentals de l'enginyeria en un context de concepció, disseny, implementació i operació de sistemes i productes del món real. Es tracta de generar un entorn pròxim a l’exercici professional de l’enginyeria com a context ideal per a l’aprenentatge de l’enginyeria. Arreu del món, els centres col·laboradors de la iniciativa CDIO l’han adoptat com el marc de referència per a la seva planificació curricular i l’avaluació basada en els resultats de l’aprenentatge.</p>\
	    \
	    <p>CDIO va néixer l’any 2000 al MIT amb l’objectiu de reduir la distància existent entre el perfil de sortida dels seus enginyers i les necessitats de la indústria. Donat que aquest era un problema que afectava a tots els estudis d’enginyeria arreu del món, des del principi es va crear un grup de treball internacional, inicialment amb les universitats sueques de Chalmers i Linköping i el KTH. A partir de qüestionaris i entrevistes amb els agents externs (empreses, ex-estudiants i agències d’acreditació), l’any 2004 defineixen la llista de competències més exhaustiva que es coneix, el CDIO Syllabus i una llista de 12 estàndards. A continuació obren la seva iniciativa a la col·laboració internacional i actualment hi ha més de 50 institucions pertanyents a 25 països que són membres en diferents graus de la iniciativa.</p>\
	    \
	    <p>L’ETSETB-UPC, després de comparar diversos models per a la definició dels plans d’estudis dels nous graus, identifica la iniciativa CDIO com el model més complet i coherent, hi entra en contacte al 4t congrés internacional a Gent l’any 2008 i presenta la seva candidatura com a centre col·laborador al 5è congrés internacional a Singapore l’any 2009, sent acceptada pel CDIO Council el juliol del 2009. El Juny del 2009 se celebra al Campus Nord un workshop amb els membres CDIO del centre i sud d’Europa amb la presència dels líders internacionals de l’iniciativa. Des de llavors ha participat activament a diversos workshops i conferències i a la coordinació del grup d’Europa central i del sud.</p>\
		    ");
	DBC_insert(dbc, "mails", list);
	
	// Crea taula comptadors
	StrList_clear(list);
	StrList_add(list, "users");
	StrList_add(list, "mails");
	StrList_add(list, "sessions");

	DBC_create_table(dbc, "counters", list);

	StrList_clear(list);
	StrList_add(list, "2"); // users
	StrList_add(list, "2"); // mails
	StrList_add(list, "0"); // sessions
	DBC_insert(dbc, "counters", list);
	
	// allibera variables
	StrList_destroy(list);
	DBC_close(dbc);
	return 0;
}
/*
int register_user(const char* name, const char* passwd)
{
	DBConnection dbc = DBC_open("db");
	StrList list = StrList_create();
	
	StrList_add(list, 0);
	StrList_add(list, name);
	StrList_add(list, passwd);
		
	DBC_insert(dbc, "users", list);
}*/
unsigned int timerandom(){
	int seconds = (int) time(NULL);
	int number = rand();
	return abs(seconds*number) % 32767; //màxim positiu d'un signed int de 32 bits.
}
char* random_idsession(char* string, unsigned short int len){
	string = malloc(len*(sizeof (char))+1);
	int i;
	unsigned int type;
	unsigned int digit;
	
	for(i=0; i<len; i++)
	{
		type = timerandom()%3; // number, uppercase or lowercase
		digit = timerandom()%26;
		//printf("*** type=%d\tdigit=%d ***\n", type, digit);
		if (type == 0)
			digit = digit%10 + '0';
		else if (type == 1)
			digit += 'A';
		else
			digit += 'a';
		//printf("*** digit=%c=%d ***\n", digit, digit);
		string[i] = digit;
	}
	string[len] = '\0';
	return string;
}

int sendmail(char* from, char* to, char* subject, char* content){
	//E usuari??
	DBConnection dbc = DBC_open("db");
	DBC_select(dbc,"users", "user_name", from); //suposem nom d'usuari únic
	if (DBC_next(dbc) == -1){
	  DBC_close(dbc);
	  return ERROR;
	}
	// obtenim el valor del comptador de mails
	DBC_select(dbc, "counters", NULL, NULL);
	char* s = DBC_get(dbc, "mails");
	// formatem l'identificador
	int mail_id = atoi(s);
	// enviem (registrem) el mail
	StrList list = StrList_create();
	StrList_add(list, s);
	StrList_add(list, from);
	StrList_add(list, to);
	StrList_add(list, subject);
	StrList_add(list, content);
	DBC_insert(dbc, "mails", list);
	StrList_destroy(list);
	// incrementem el valor del comptador de mails
	mail_id++;
	if(mail_id%10==0)
		s = realloc(s, strlen(s)+1+sizeof('\0'));
	sprintf(s, "%d", mail_id);
	DBC_update(dbc, "counters",
		"mails", &s,
		NULL, NULL
	);
	free(s);
	return ENVIAT;
}







