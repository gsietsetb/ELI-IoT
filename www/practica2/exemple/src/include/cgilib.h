
#ifndef _CGILIB_H_
#define _CGILIB_H_


void cgiMain(void);


#define MIME_HTML	"text/html"
#define MIME_URLFORM	"application/x-www-form-urlencoded"

const char *request_method(void);
const char *request_contentType(void);
int request_contentLength(void);

const char **request_paramValues(const char *name, int *pValueCount);
const char *request_param(const char *name);

int request_cookieCount(void);
const char *request_cookieName(int i);
const char *request_cookieValue(int i);


void session_init(const char *db);
const char *session_getAttribute(const char *name);
void session_setAttribute(const char *name, const char *value);
void session_invalidate(void);


int response_setContentType(const char *mimeType);
int response_redirect(const char *uri);
int response_isCommitted(void);
int response_commitHeaders(void);


#endif


