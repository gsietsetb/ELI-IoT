
#include "urlcoding.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


char *strdcat(const char *s1, const char *s2)
{
    char *str = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(str,s1);
    strcat(str,s2);
    return str;
}


char *read_std_file(FILE *f, int len)
{
    char *result = strdcat("", "");
    char buffer[1025];
    int nread = 0;
    while (len < 0 || nread < len) {
	char *oldr = result;
	int mx = (len < 0) ? 1024 : (len - nread);
	int i = fread(buffer, 1, mx, f);
    	if (i <= 0) {
	    break;
	}
	buffer[i] = 0;
	result = strdcat(result, buffer);
	free(oldr);
    }
    return result;
}


char *read_input(int fd, unsigned len)
{
    char *str = malloc(len+1);
    unsigned nread = 0;
    while (nread<len) {
	int i = read(fd,str+nread,len-nread);
    	if (i<=0) {
	    break;
	}
	nread = nread + i;
    }
    str[nread] = 0;
    return str;
}

