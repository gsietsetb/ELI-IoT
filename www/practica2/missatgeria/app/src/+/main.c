#include <stdio.h>
#include "urlcoding.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "request.h"

#define LOGIN_OK	"login_ok.html"

Form form;

int main(int argc, char **argv)
{
	form = check_request();
	if (form == NULL)
		return 0;
	
	redirect(LOGIN_OK);
	return 0;
}

