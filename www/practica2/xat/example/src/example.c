
#include "urlcoding.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define FORM_MIME	"application/x-www-form-urlencoded"

Form form;

#define MESSAGE_FILE	"example-message.txt"


void process(void);
void change(void);
void page(void);
char *get_message(void);
void print_escaped(const char *text);


int main(int argc, char **argv)
{
    char *request_method = getenv("REQUEST_METHOD");
    char *content_type = getenv("CONTENT_TYPE");

    if (request_method != NULL && strcmp(request_method, "POST") == 0
	&& content_type != NULL && strcmp(content_type, FORM_MIME) == 0)
    {
	char *content_length = getenv("CONTENT_LENGTH");
	form = Form_create(read_std_file(stdin, atoi(content_length)));
	process();
    } else if (request_method != NULL && strcmp(request_method, "GET") == 0) {
	char *qs = getenv("QUERY_STRING");
	form = Form_create((qs == NULL) ? "" : qs);
	process();
    } else {
	printf("Location: invalid_request.html\n\n");
    }
    return 0;
}


void process(void)
{
    if (Form_get(form, "change") != NULL) {
	/* change action from form */
	change();
    } else {
	page();
    }
}


void change(void)
{
    const char *message = Form_get(form, "message");
    FILE *fmessages = fopen(MESSAGE_FILE, "w");
    if (fmessages==NULL) {
	printf("Location: save_error.html\n\n");
	return;
    }
    fprintf(fmessages, "%s", message);
    fclose(fmessages);

    page();
}


void print_escaped(const char *text)
{
    while (*text != 0) {
	char c = * (text++);
	switch (c) {
	    case '<': printf("&lt;"); break;
	    case '&': printf("&amp;"); break;
	    default: putchar(c); break;
	}
    }
}


void page(void)
{
    char *mess = get_message();
    printf("Content-Type: text/html\n\n");
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
//***    printf("<FORM METHOD=\"POST\" ACTION=\"%s\"><CENTER>\n", getenv("SCRIPT_NAME"));
    printf("<FORM METHOD=\"POST\" ACTION=\"#\"><CENTER>\n");
    printf("<TEXTAREA NAME=\"message\" ROWS=\"15\" COLS=\"100\">"); print_escaped(mess); printf("</TEXTAREA>\n");
    printf("<BR><INPUT TYPE=\"submit\" NAME=\"change\" VALUE=\"Canvia\">\n");
    printf("</CENTER></FORM><HR>\n");
    printf("</BODY></HTML>\n");
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
