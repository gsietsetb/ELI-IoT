
#include "cgilib.h"

#include "urlcoding.h"
#include "dbc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>	/* getpid() */
#include <time.h>	/* time() */


/****************************************************************
 * Request
 */

const char *request_method(void)
{
    return getenv("REQUEST_METHOD");
}

const char *request_contentType(void)
{
    return getenv("CONTENT_TYPE");
}

static int request_contentLengthMem = -1;

int request_contentLength(void)
{
    if (request_contentLengthMem < 0) {
	request_contentLengthMem = 0;
	if (getenv("CONTENT_LENGTH") != NULL) {
	    request_contentLengthMem = atoi(getenv("CONTENT_LENGTH"));
	}
    }
    return request_contentLengthMem;
}


/****************************************************************
 * Request parameters
 */

static Form qs_form = NULL,
            content_form = NULL;


const char **request_paramValues(const char *name, int *pValueCount)
{
    const char **values = NULL;
    *pValueCount = 0;
    if (strcmp(request_method(), "POST") == 0 && content_form != NULL) {
	values = Form_getValues(content_form, name, pValueCount);
    }
    if (*pValueCount == 0 && qs_form != NULL) {
	values = Form_getValues(qs_form, name, pValueCount);
    }
    return values;
}

const char *request_param(const char *name)
{
    int valueCount;
    const char **values = request_paramValues(name, & valueCount);
    if (valueCount > 0) {
	return values[0];
    }
    return NULL;
}


/****************************************************************
 * Request cookies
 */

typedef struct {
    char *mem;
    int size;
} StrBuffer;

#define StrBuffer_init(sbp)	memset((sbp), 0, sizeof(*(sbp)))
#define StrBuffer_free(sbp)	free((sbp)->mem)
static void StrBuffer_copy(StrBuffer *sbp, const char *text)
{
    if (strlen(text) >= sbp->size) {
	sbp->size = strlen(text) + 1;
	sbp->mem = (char*) realloc(sbp->mem, sbp->size);
    }
    strcpy(sbp->mem, text);
}
#define StrBuffer_clear(sbp)	StrBuffer_copy((sbp), "")


static StrBuffer hscan_buffer = { NULL, 0};
static char *hscan_actual, *hscan_first;
static int hscan_saved = 0;
static int hscan_token;
#define HTOKEN_TOKEN		-1
#define HTOKEN_QUOTSTRING	-2
#define HTOKEN_COMMENT		-3

#define SP	' '
#define HT	'\t'
#define DEL	'\x7f'
#define SPHT		" \t"
#define SEPARATORS	"()<>@,;:\\\"/[]?={} \t"

/* scan a structured field body (see RFCs 2616 and 822) */
static void hscan_next(char *text)
{
    int state = 0;
    int comment = 0;
    if (text != NULL) {
	StrBuffer_copy(& hscan_buffer, text);
	hscan_actual = hscan_buffer.mem;
    }
    if (hscan_saved != 0) {
	*hscan_actual = hscan_saved;
    }
    if (hscan_token == HTOKEN_QUOTSTRING || hscan_token == HTOKEN_COMMENT) {
	if (*hscan_actual != 0) hscan_actual ++;
    }
    hscan_saved = 0;
    hscan_token = 0;
    hscan_first = hscan_actual;
    while (*hscan_actual != 0) {
	int ch = (unsigned char) * (hscan_actual++);
	switch (state) {
	    case 0:	/* in field body */
		if (strchr(SPHT, ch) != NULL) {
		} else if (ch == '\n') {
		} else if (ch == '\r') {
		    state = 1;
		} else if (ch == '\"') {
		    hscan_first = hscan_actual;
		    state = 10;
		} else if (ch == '(') {
		    hscan_first = hscan_actual;
		    state = 20;
		    comment = 1;
		} else if (32 <= ch && ch != DEL && strchr(SEPARATORS, ch) == NULL) {
		    hscan_first = hscan_actual - 1;
		    state = 30;
		} else {
		    hscan_token = ch;
		    return;
		}
		break;
	    case 1:	/* after CR */
		if (ch == '\n') {
		    state = 0;
		} else {
		    hscan_actual --;
		    hscan_token = '\r';
		    return;
		}
		break;
	    case 10:	/* in quoted string */
		if (ch == '\r') {
		    state = 11;
		} else if (ch == '\\') {
		    hscan_actual --;
		    strcpy(hscan_actual, hscan_actual + 1);
		} else if (ch == '\"') {
		    hscan_actual --;
		    hscan_saved = *hscan_actual;
		    *hscan_actual = 0;
		    hscan_token = HTOKEN_QUOTSTRING;
		    return;
		} else {
		}
		break;
	    case 20:	/* in comment */
		if (ch == '\r') {
		    state = 21;
		} else if (ch == '\\') {
		    hscan_actual --;
		    strcpy(hscan_actual, hscan_actual + 1);
		} else if (ch == ')') {
		    if (--comment == 0) {
			hscan_actual --;
			hscan_saved = *hscan_actual;
			*hscan_actual = 0;
			hscan_token = HTOKEN_COMMENT;
			return;
		    }
		} else if (ch == '(') {
		    comment ++;
		} else {
		}
		break;
	    case 11:	/* after CR, in quoted string or comment */
	    case 21:
		if (ch == '\n') {
		    hscan_actual -= 2;
		    strcpy(hscan_actual, hscan_actual + 2);
		}
		state -= 1;
		break;
	    case 30:	/* in token */
		if (32 <= ch && ch != DEL && strchr(SEPARATORS, ch) == NULL) {
		} else {
		    hscan_actual --;
		    hscan_saved = *hscan_actual;
		    *hscan_actual = 0;
		    hscan_token = HTOKEN_TOKEN;
		    return;
		}
		break;
	}
    }
    /* end of string reached */
    switch (state / 10) {
	case 0:	/* in field body */
	    hscan_token = 0;
	    break;
	case 1:	/* in quoted string */
	    hscan_token = HTOKEN_QUOTSTRING;
	    break;
	case 2:	/* in comment */
	    hscan_token = HTOKEN_COMMENT;
	    break;
	case 3:	/* in token */
	    hscan_token = HTOKEN_TOKEN;
	    break;
    }
}


static int req_cookiesCount = 0;
static int req_cookiesCapacity = 0;
static Form *req_cookies = NULL;

/* parse Cookie request header field body (see RFC 2109) */
static int parseAVPair(StrBuffer *nameBuf, StrBuffer *valueBuf)
{
    if (hscan_token != HTOKEN_TOKEN) {
	fprintf(stderr, "Bad Cookie header syntax: token=%d\n", hscan_token);
	return -1;
    }
    if (nameBuf != NULL) StrBuffer_copy(nameBuf, hscan_first);
    hscan_next(NULL);
    if (hscan_token != '=') {
	fprintf(stderr, "Bad Cookie header syntax: token=%d\n", hscan_token);
	return -1;
    }
    hscan_next(NULL);
    if (hscan_token != HTOKEN_TOKEN && hscan_token != HTOKEN_QUOTSTRING) {
	fprintf(stderr, "Bad Cookie header syntax: token=%d\n", hscan_token);
	return -1;
    }
    if (valueBuf != NULL) StrBuffer_copy(valueBuf, hscan_first);
    hscan_next(NULL);
    return 0;
}

static void parseCookies(void)
{
    StrBuffer name, value;
    char *cookie;
    StrBuffer_init(& name); StrBuffer_init(& value);
    if (getenv("HTTP_COOKIE") == NULL) {
	return;
    }
    cookie = getenv("HTTP_COOKIE");
    hscan_next(cookie);
    while (hscan_token == HTOKEN_TOKEN && hscan_first[0] == '$') {
	if (parseAVPair(NULL, NULL) != 0) {
	    fprintf(stderr, "Bad Cookie header syntax: %s\n", cookie);
	    return;
	}
	if (hscan_token != ',' && hscan_token != ';') {
	    fprintf(stderr, "Bad Cookie header syntax: %s\n", cookie);
	    return;
	}
	hscan_next(NULL);
    }
    while (hscan_token == HTOKEN_TOKEN) {
	Form form;
	if (parseAVPair(& name, & value) != 0) {
	    StrBuffer_free(& name); StrBuffer_free(& value);
	    return;
	}
	if (req_cookiesCapacity == req_cookiesCount) {
	    req_cookiesCapacity += 10;
	    req_cookies = (Form*) realloc(req_cookies, req_cookiesCapacity * sizeof(Form));
	}
	form = Form_create("");
	req_cookies[req_cookiesCount ++] = form;
	Form_add(form, "NAME", name.mem);
	Form_add(form, "VALUE", value.mem);
	if (hscan_token != ',' && hscan_token != ';') {
	    break;
	}
	hscan_next(NULL);
	while (hscan_token == HTOKEN_TOKEN && hscan_first[0] == '$') {
	    if (parseAVPair(& name, & value) != 0) {
		StrBuffer_free(& name); StrBuffer_free(& value);
		return;
	    }
	    Form_add(form, name.mem, value.mem);
	    if (hscan_token != ',' && hscan_token != ';') {
		break;
	    }
	    hscan_next(NULL);
	}
    }
    if (hscan_token != 0) {
	fprintf(stderr, "Bad Cookie header syntax: %s\n", cookie);
    }
    StrBuffer_free(& name); StrBuffer_free(& value);
}

int request_cookieCount(void)
{
    if (req_cookies == NULL) {
	parseCookies();
    }
    return req_cookiesCount;
}

static Form getCookie(int i)
{
    if (req_cookies == NULL) {
	parseCookies();
    }
    if (0 <= i && i < req_cookiesCount) {
	return req_cookies[i];
    }
    return NULL;
}

const char *request_cookieName(int i)
{
    Form cookie = getCookie(i);
    if (cookie != NULL) return Form_get(cookie, "NAME");
    else return NULL;
}

const char *request_cookieValue(int i)
{
    Form cookie = getCookie(i);
    if (cookie != NULL) return Form_get(cookie, "VALUE");
    else return NULL;
}


/****************************************************************
 * Random generation
 */

static int random_state = 0;

static int random_getByte(void)
{
    if (random_state == 0) {
	srand((unsigned int) (time(NULL) ^ getpid()));
    }
    do {
	srand((unsigned int) (random_state ^ getpid() ^ time(NULL)));
	random_state = rand();
    } while (random_state == 0);
    return random_state;
}

static void random_getBytes(char *addr, unsigned len)
{
    while (len > 0) {
	addr[--len] = (char) random_getByte();
    }
}


/****************************************************************
 * Session management
 */

static char *session_db = NULL;

static int session_req_cookie = -2;	/* -2=not searched, -1=searched but not found, 0..=found */
static char *session_id = NULL;
static Form session_attrs = NULL;

#define CGI_SESSIONID_NAME	"CGI_SESSION_ID"

#define CGI_SESSION_MAXINACTIVETIME	600	/* 10 minutes */


static const char *session_getId(void)
{
    /* Session-id Cookie processing */
    if (session_id == NULL) {
	if (session_req_cookie <= -2) {
	    int i;
	    session_req_cookie = -1;
	    for (i = 0; i < request_cookieCount(); i ++) {
		if (strcmp(request_cookieName(i), CGI_SESSIONID_NAME) == 0) {
		    session_req_cookie = i;
		    session_id = strdcat(request_cookieValue(i), "");
		    break;
		}
	    }
	}
    }
    return session_id;
}

static void session_create(void)
{
    char buf[64];
    long rands[4];
    long t = time(NULL);
    StrList cvalues;
    DBConnection dbc;
    if (session_id != NULL) {
	free(session_id);
    }
    if (session_attrs != NULL) {
	Form_destroy(session_attrs);
    }
    dbc = DBC_open(session_db);
    while (1) {
	/* Make session id. */
	int isDup = 0;
	random_getBytes((char*) rands, sizeof(rands));
	sprintf(buf, "%08lx%08lx%08lx%08lx", rands[0], rands[1], rands[2], rands[3]);
	/* Search duplicated & remove invalid sessions */
	DBC_select(dbc, "sessions", NULL, NULL);
	while (DBC_next(dbc) == 0) {
	    char *sid = DBC_get(dbc, "id");
	    char *slasttime = DBC_get(dbc, "last_time");
	    if (strcmp(sid, buf) == 0) {
		isDup = 1;
	    }
	    if (t - atoi(slasttime) >= CGI_SESSION_MAXINACTIVETIME) {
		DBC_delete(dbc, "sessions", "id", sid);
	    }
	    free(sid);
	    free(slasttime);
	}
	if (! isDup) break;
    }
    /* Insert... */
    session_id = strdcat(buf, "");
    session_attrs = Form_create("");
    cvalues = StrList_create();
    /* Add id */
	StrList_add(cvalues, buf);
    /* Add creation & last access time */
	sprintf(buf, "%ld", t);
	StrList_add(cvalues, buf);
	StrList_add(cvalues, buf);
    /* Add session attributes */
	StrList_add(cvalues, "");
    DBC_insert(dbc, "sessions", cvalues);
    StrList_destroy(cvalues);
    DBC_close(dbc);
}

static void session_tryload(void)
{
    DBConnection dbc;
    if (session_getId() == NULL) {
	return;
    }
    dbc = DBC_open(session_db);
    DBC_select(dbc, "sessions", "id", session_id);
    if (DBC_next(dbc) == -1) {
    	/* Session not found in database */
    	free(session_id);
    	session_id = NULL;
    } else {
	/* Session is in database (not new) */
	char *encoded;
	if (session_attrs == NULL) {
	    /* First load in this request. Set last accessed time */
	    char *lasttime = DBC_get(dbc, "last_time");
	    long t = time(NULL);
	    char buf[30];
	    if (t - atoi(lasttime) >= CGI_SESSION_MAXINACTIVETIME) {
		/* Session is invalid (too much inactive time) */
		free(lasttime);
		DBC_delete(dbc, "sessions", "id", session_id);
		free(session_id);
		session_id = NULL;
		DBC_close(dbc);
		return;
	    }
	    free(lasttime);
	    sprintf(buf, "%ld", t);
	    DBC_update(dbc, "sessions", "last_time", buf, "id", session_id);
	} else {
	    Form_destroy(session_attrs);
	}
    	encoded = DBC_get(dbc,"attributes");
    	session_attrs = Form_create(encoded);
    	free(encoded);
    }
    DBC_close(dbc);
}


void session_init(const char *db)
{
    session_db = strdcat(db, "");
}


const char *session_getAttribute(const char *name)
{
    session_tryload();
    if (session_id == NULL) {
	return NULL;
    }
    return Form_get(session_attrs, name);
}

void session_setAttribute(const char *name, const char *value)
{
    DBConnection dbc;
    char * encoded;
    session_tryload();
    if (session_id == NULL) {
	session_create();
    }
    Form_remove(session_attrs, name);
    if (value != NULL) {
	Form_add(session_attrs, name, value);
    }
    encoded = Form_encode(session_attrs);
    dbc = DBC_open(session_db);
    DBC_update(dbc, "sessions", "attributes", encoded, "id", session_id);
    free(encoded);
    DBC_close(dbc);
}


void session_invalidate(void)
{
    if (session_getId() != NULL) {
	DBConnection dbc = DBC_open(session_db);
	DBC_delete(dbc, "sessions", "id", session_id);
	free(session_id);
	if (session_attrs != NULL) {
	    Form_destroy(session_attrs);
	}
	session_id = NULL;
	session_attrs = NULL;
	DBC_close(dbc);
    }
}

static void session_setCookie(void)
{
    /* Set Path attribute for PATH_INFO independence */
    if (session_id != NULL) {
	printf("Set-Cookie: %s=%s; Version=1; Path=\"%s\"\n",
		CGI_SESSIONID_NAME, session_id, getenv("SCRIPT_NAME"));
    } else if (session_req_cookie >= 0) {
	printf("Set-Cookie: %s=0; Version=1; Path=\"%s\"; Max-Age=0\n",
		CGI_SESSIONID_NAME, getenv("SCRIPT_NAME"));
    }
}


/****************************************************************
 * Generic CGI processing
 */

static char *response_contentType = "text/html";
static int response_committed = 0;


int response_setContentType(const char *mimeType)
{
    if (response_committed) {
	return -1;
    } else {
	response_contentType = strdcat(mimeType, "");
	return 0;
    }
}

int response_redirect(const char *uri)
{
    if (response_committed) {
	return -1;
    } else {
	printf("Location: %s\n", uri);
	session_setCookie();
	printf("\n");
	response_committed = 1;
	return 0;
    }
}


int response_isCommitted(void)
{
    return response_committed;
}

int response_commitHeaders(void)
{
    if (response_committed) {
	return -1;
    } else {
	printf("Content-Type: %s\n", response_contentType);
	session_setCookie();
	printf("\n");
	response_committed = 1;
	return 0;
    }
}


/****************************************************************
 * Generic CGI processing
 */

#define CGI_DEBUG_NEVER		0
#define CGI_DEBUG_BYQUERY	1
#define CGI_DEBUG_ALWAYS	2

#define CGI_DEBUG	CGI_DEBUG_BYQUERY

extern char **environ;

#if CGI_DEBUG > CGI_DEBUG_NEVER
static void print_esc_env(const char *text)
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
static void prepare_debug(void)
{
    /* Debug preparation */
#if CGI_DEBUG == CGI_DEBUG_BYQUERY
    if (getenv("QUERY_STRING") != NULL && strcmp(getenv("QUERY_STRING"), "CGI_DEBUG=ON") == 0) {
	/* Debug enabling */
	    printf("Content-Type: text/html\n");
	    printf("Set-Cookie: _CGI_DEBUG_ON_=1; Version=1\n\n");
	    printf("<html><head></head><body>\n");
	    printf("<h2 align=\"center\"><font color=\"red\">CGI DEBUG is enabled</font></h2>\n");
	    printf("</body></html>\n");
	    exit(0);
    } else if (getenv("QUERY_STRING") != NULL && strcmp(getenv("QUERY_STRING"), "CGI_DEBUG=OFF") == 0) {
	/* Debug disabling */
	    printf("Content-Type: text/html\n");
	    printf("Set-Cookie: _CGI_DEBUG_ON_=1; Version=1; Max-Age=0\n\n");
	    printf("<html><head></head><body>\n");
	    printf("<h2 align=\"center\"><font color=\"green\">CGI DEBUG is disabled</font></h2>\n");
	    printf("</body></html>\n");
	    exit(0);
    } else if (getenv("HTTP_COOKIE") != NULL && strstr(getenv("HTTP_COOKIE"), "_CGI_DEBUG_ON_") != NULL) {
#else
    {
#endif
	char inpath[64], envpath[64], outpath[64], errpath[64];
	char buf[256];
	char **envp;
	int inTextLength = 0;
	if (getenv("QUERY_STRING") != NULL && strncmp(getenv("QUERY_STRING"), "CGI_OUT_", 8) == 0) {
	    /* Send previous CGI response (if exists) */
	    FILE *in = fopen(getenv("QUERY_STRING"), "r");
	    if (in != NULL) {
		fclose(in);
		sprintf(buf, "cat %s", getenv("QUERY_STRING"));
		system(buf);
		system("rm CGI_*_*.log");
		exit(0);
	    }
	}
	if (getenv("CONTENT_LENGTH") != NULL && getenv("CONTENT_TYPE") != NULL &&
	    (strcmp(getenv("CONTENT_TYPE"), MIME_URLFORM) == 0 || strncmp(getenv("CONTENT_TYPE"), "text/", 5) == 0))
	{
	    inTextLength = atoi(getenv("CONTENT_LENGTH"));
	}
	/* Redirect CGI outputs & sends info page */
	    sprintf(envpath, "CGI_ENV_%d.log", getpid());
	    sprintf(outpath, "CGI_OUT_%d.log", getpid());
	    sprintf(errpath, "CGI_ERR_%d.log", getpid());
	    printf("Content-Type: text/html\n\n");
	    printf("<html><head></head><body>\n");
	    printf("<p><a href='%s'>Environment variables:</a></p>\n<table>", envpath);
	    for (envp = environ; *envp != NULL; envp++) {
		char *eq = strchr(*envp, '=');
		if (eq != NULL) {
		    *eq = 0;
		    printf("<tr><th align=\"right\">"); print_esc_env(*envp);
		    printf("</th><td align=\"left\">"); print_esc_env(eq + 1); printf("</td></tr>\n");
		    *eq = '=';
		}
	    }
	    printf("</table>\n");
	    if (inTextLength > 0) {
		sprintf(inpath, "CGI_IN_%d.log", getpid());
		printf("<p><a href='%s'>CGI input</a></p>\n", inpath);
	    }
	    printf("<p><a href='%s'>CGI output</a></p>\n", outpath);
	    printf("<p><a href='%s'>CGI errors</a></p>\n", errpath);
	    printf("<p><a href='%s?%s'>Send response to browser (and remove logs)</a></p>\n", getenv("SCRIPT_NAME"), outpath);
	    printf("<p><a href='%s?CGI_DEBUG=OFF'><font color=\"green\">Disable debug</font></a></p>\n", getenv("SCRIPT_NAME"));
	    printf("</body></html>\n");
	    fflush(stdout);
	    sprintf(buf, "/usr/bin/env >%s", envpath);
	    system(buf);
	    if (inTextLength > 0) {
		char *input = read_std_file(stdin, inTextLength);
		FILE *finput = fopen(inpath, "w");
		fprintf(finput, "%s", input);
		fclose(finput);
		free(input);
		freopen(inpath, "r", stdin);
	    }
	    //***outcopyfd = dup(1);
	    freopen(outpath, "w", stdout);
	    freopen(errpath, "w", stderr);
	    setbuf(stdout,NULL);
    }
}
#endif


void cgiMain(void);

int main(int argc, char **argv)
{
#if CGI_DEBUG > CGI_DEBUG_NEVER
    prepare_debug();
#endif

    /* Request processing */
    if (getenv("QUERY_STRING") != NULL) {
	qs_form = Form_create(getenv("QUERY_STRING"));
    }
    if (strcmp(request_method(), "POST") == 0 && request_contentType() != NULL &&
	strcmp(request_contentType(), MIME_URLFORM) == 0)
    {
	    char *form_s = read_std_file(stdin, request_contentLength());
	    content_form = Form_create(form_s);
	    free(form_s);
    }

    /* Session-id Cookie processing */
/***
    char *cookie = getenv("HTTP_COOKIE");
    if (cookie == NULL) {
	session_id = NULL;
    } else {
	size_t pos1 = strcspn(cookie, "=");
	if (strncmp(cookie, CGI_SESSIONID_NAME, pos1) == 0) {
	    size_t pos2 = strcspn(cookie+pos1+1, ";");
	    session_id = (char*) malloc(pos2 + 1);
	    strncpy(session_id, cookie+pos1+1, pos2);
	} else {
	    session_id = NULL;
	}
    }
request_cookieCount();
***/
    cgiMain();

    return 0;
}


