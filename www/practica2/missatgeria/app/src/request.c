#include "request.h"

//form = check_request(request_method, content_type);
Form check_request(void)
{
	Form form;
	
	char *request_method = getenv("REQUEST_METHOD");
	char *content_type = getenv("CONTENT_TYPE");
	
	//METHOD POST
	if (request_method != NULL && strcmp(request_method, "POST") == 0
		&& content_type != NULL && strcmp(content_type, FORM_MIME) == 0)
	{
		char *content_length = getenv("CONTENT_LENGTH");
		form = Form_create(read_std_file(stdin, atoi(content_length)));
	}
	//METHOD GET
	else if (request_method != NULL && strcmp(request_method, "GET") == 0)
	{	
		char *qs = getenv("QUERY_STRING");
		form = Form_create((qs == NULL) ? "" : qs);
	} else
	{
		redirect(INVALID_REQUEST);
		return NULL;
	}
	
	return form;
}

void redirect(const char* target)
{
	printf("%s\nLocation: %s\n\n",CGI_HEAD,target);
}
