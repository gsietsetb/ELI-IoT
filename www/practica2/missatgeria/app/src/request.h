#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "urlcoding.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FORM_MIME	"application/x-www-form-urlencoded"
#define CGI_HEAD	"Content-Type: text/html;charset=utf-8"
#define INVALID_REQUEST	"invalid_request.html"
#define HTML_HEAD	"<html><head></head><body>"
#define HTML_FOOT	"</body></html>\n"

Form check_request(void);
void redirect(const char* target);

#endif
