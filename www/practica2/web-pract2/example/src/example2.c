
#include "cgilib.h"
#include "urlcoding.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define MESSAGE_FILE	"example-message.txt"


void change(void);
void page(void);


void cgiMain(void)
{
    if (strcmp(request_method(), "GET") == 0 || strcmp(request_method(), "POST") == 0) {
	if (request_param("change") != NULL) {
	    /* change action from form */
	    change();
	} else {
	    page();
	}
    } else {
	response_redirect("invalid_request.html\n\n");
    }
}


void change(void)
{
    const char *message = request_param("message");
    FILE *fmessages = fopen(MESSAGE_FILE, "w");
    if (fmessages==NULL) {
	response_redirect("save_error.html\n\n");
    } else {
	fprintf(fmessages, "%s", message);
	fclose(fmessages);

	page();
    }
}


char *get_message(void);
void print_escaped(const char *text);

void page(void)
{
    char *mess = get_message();
    response_setContentType(MIME_HTML);
    response_commitHeaders();

    printf("<HTML><HEAD>\n");
    printf("<TITLE>Disseny d'aplicacions WEB: Exemple CGI interactiu</TITLE></TITLE>\n");
    printf("</HEAD><BODY BGCOLOR=\"lightblue\">\n");
    printf("<center><H1>Exemple de CGI interactiu</H1></center><HR>\n");
    printf("<H2>&Uacute;ltim missatge:</H2>\n");
    if (strlen(mess) == 0) {
      printf("<EM>(no hi missatge)</EM>\n");
    } else {
      printf("<PRE>\n"); print_escaped(mess); printf("</PRE>\n");
    }
    printf("<H2>Missatge:</H2>\n");
    printf("<FORM METHOD=\"POST\" ACTION=\"%s\"><CENTER>\n", getenv("SCRIPT_NAME"));
//***    printf("<FORM METHOD=\"POST\" ACTION=\"#\"><CENTER>\n");
    printf("<TEXTAREA NAME=\"message\" ROWS=\"15\" COLS=\"100\">"); print_escaped(mess); printf("</TEXTAREA>\n");
    printf("<BR><INPUT TYPE=\"submit\" NAME=\"change\" VALUE=\"Canvia\">\n");
    printf("</CENTER></FORM><HR>\n");
    printf("</BODY></HTML>\n");
}

void print_escaped(const char *text)
{
    while (*text != 0) {
	char c = * (text++);
	switch (c) {
	    case '&': printf("&amp;"); break;
	    case '<': printf("&lt;"); break;
	    case '>': printf("&gt;"); break;
	    case '\"': printf("&quot;"); break;
	    case '\'': printf("&apos;"); break;
	    default: putchar(c); break;
	}
    }
}

char *get_message(void)
{
    FILE *fmessages;
    fmessages = fopen(MESSAGE_FILE, "r");
    if (fmessages==NULL) {
	return strdcat("", "");
    } else {
	char *mess;
	mess = read_std_file(fmessages, -1);
	fclose(fmessages);
	return mess;
    }
}


