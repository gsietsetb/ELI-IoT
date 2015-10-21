/*  urlcoding.c
 *
 * Data: 18-Oct-1999
 *
 * L'estructura Form representa les dades d'un formulari HTML.
 * El constructor te un parametre String que correspon a la codificacio
 * URL-form d'aquestes dades (es la codificacio que fan normalment tots els
 * navegadors WEB abans de pasar les dades del formulari al servidor).
 * Sintaxi d'aquesta codificacio:
 *	form-coding:	form-field *('&' form-field)
 *	form-field:	field-name '=' field-value
 *	field-name:	URLcoded-string
 *	field-value:	URLcoded-string
 *	URLcoded-string:  string codificat de la seg&uuml;ent forma
 *		Les lletres i els digits ('A'..'Z', 'a'..'z', '0'..'9')
 *			es deixen igual.
 *		Els espais (' ') es codifiquen amb '+'.
 *		Els altres caracters es codifiquen amb la sequencia
 *			'%xx' on xx es el codi ASCII hexadecimal del caracter.
 *
 * Conte tambe metodes utilitats per a codificar/decodificar cadenes de 
 * caracters en format URL.
 */

#include "urlcoding.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>


/* Si voleu traces de depuracio descomenteu aquesta linia i comenteu
 * la seguent:
#define TRACE(args)	printf args
*/
#define TRACE(args)


static struct FormElem *Form_search_elem(Form form, const char *name)
{
    int i;
    for (i = 0; i < form->size; i++) {
	if(strcmp(form->elems[i].name, name) == 0) {
	    return & form->elems[i];
	}
    }

    return NULL;
}


Form Form_create(const char *s)
{
	Form form;
	int length;
	const char *first;

	form = (Form) malloc(sizeof(struct Form_struct));
	form->capacity = 10;
	form->size = 0;
	form->elems = (struct FormElem*)malloc(
		form->capacity * sizeof(struct FormElem)
	);

	if (s == NULL || *s == 0) {
	    return form;
	}
	length = strlen(s);
	first = s;			/* Posicio inicial del camp actual */
	while (1) {
	    const char *last, *sep;
	    char *name, *value;
	    last = strchr(first, '&');	/* Es la posicio del seguent separador de camps */
	    if (last == NULL) {
		last = s + length;
	    }
	    sep = strchr(first, '=');	/* Es la posicio del separador entre nom i valor */
	    if (sep == NULL) {
	        Form_destroy(form);	/* Error de sintaxi */
		return NULL;
	    }
TRACE(("DEBUG: first=\"%s\" last=\"%s\" sep=\"%s\"\n", first,last,sep));
	    assert(sep < last);

	    name = URL_decode(first, (unsigned)(sep-first));
	    value = URL_decode(sep+1, (unsigned)(last-sep)-1);
	    Form_add(form, name, value);
	    free(name);
	    free(value);

	    if (*last == 0) {
		break;
	    }
	    first = last + 1;	/* Mou a l'inici del seguent camp */
	}

	return form;
}


static void Form_destroy_elem(struct FormElem *elem)
{
	int j;
	free(elem->name);
	for (j = 0; j < elem->count; j ++) {
	    free(elem->values[j]);
	}
	free(elem->values);
}

void Form_destroy(Form form)
{
    int i;
    for (i=0; i<form->size; i++) {
	Form_destroy_elem(& form->elems[i]);
    }

    free(form->elems);
    free(form);
}


int Form_getCount(Form form)
{
    return form->size;
}


const char *Form_getName(Form form, int i)
{
    if (0 <= i && i < form->size) {
	return (const char *) form->elems->name;
    } else {
	return NULL;
    }
}


const char **Form_getValues(Form form, const char *name, int *pValueCount)
{
    struct FormElem *elem = Form_search_elem(form, name);
    if (elem != NULL) {
	*pValueCount = elem->count;
	return (const char **) elem->values;
    } else {
	*pValueCount = 0;
	return NULL;
    }
}

const char *Form_get(Form form, const char *name)
{
    int valueCount;
    const char **values = Form_getValues(form, name, & valueCount);
    if (valueCount > 0) return values[0];
    else return NULL;
}


/* Deprecated */
int Form_get_count(Form form, const char *name)
{
    int valueCount;
    Form_getValues(form, name, & valueCount);
    return valueCount;
}

/* Deprecated */
const char *Form_get_i(Form form, const char *name, int index)
{
    int valueCount;
    const char **values = Form_getValues(form, name, & valueCount);
    if (index < 0 || valueCount <= index) return NULL;
    else return values[index];
}


void Form_add(Form form, const char *name, const char *value)
{
    struct FormElem *elem = Form_search_elem(form, name);
    if (elem != NULL) {
	elem->count ++;
	elem->values = realloc(elem->values, elem->count * sizeof(char*));
	elem->values[elem->count - 1] = strdcat(value, "");
    } else {
	if (form->size == form->capacity) {
	    form->capacity = 2 * form->capacity;
	    form->elems = (struct FormElem*)realloc(form->elems,
			form->capacity * sizeof(struct FormElem)
	    );
	}
	form->elems[form->size].name = strdcat(name, "");
	form->elems[form->size].count = 1;
	form->elems[form->size].values = malloc(sizeof(char*));
	form->elems[form->size].values[0] = strdcat(value, "");
	form->size ++;
    }
}


void Form_clear(Form form)
{
    int i;
    for (i = 0; i < form->size; i++) {
	Form_destroy_elem(& form->elems[i]);
    }
    form->size = 0;
}


void Form_remove(Form form, const char *name)
{
    int i;
    for (i = 0; i < form->size; i++) {
	if(strcmp(form->elems[i].name, name) == 0) {
	    Form_destroy_elem(& form->elems[i]);
	    form->size --;
	    if (i < form->size) {
		form->elems[i] = form->elems[form->size];
	    }
	    return;
	}
    }
}


char *Form_encode(Form form)
{
    char *result = strdcat("", "");
    int i;
    for (i = 0; i < form->size; i++) {
	struct FormElem *elem = & form->elems[i];
	char *ename = URL_encode(elem->name, strlen(elem->name));
	int j;
	for (j = 0; j < elem->count; j ++) {
	    char *evalue = URL_encode(elem->values[j], strlen(elem->values[j]));
	    char *tmp;
	    if (i > 0 || j > 0) {
		tmp = strdcat(result, "&"); free(result); result = tmp;
	    }
	    tmp = strdcat(result, ename); free(result); result = tmp;
	    tmp = strdcat(result, "="); free(result); result = tmp;
	    tmp = strdcat(result, evalue); free(result); result = tmp;
	    free(evalue);
	}
	free(ename);
    }
    return result;
}


void Form_printEncoded(Form form, FILE *f)
{
    char *encoded = Form_encode(form);
    fprintf(f, "%s", encoded);
    free(encoded);
}


/**************************************************************************/
/* URL encoding/decoding */

char *URL_encode(const char *s, unsigned len)
{
	char *buf = (char*)malloc(3*len+1);
	int last = 0;
	int i;

	for(i=0; i<len; i++) {
	    int c = (unsigned char)s[i];
	    if( (c>='0' && c<='9') 
		|| (c>='A' && c<='Z') || (c>='a' && c<='z') ) {
		buf[last++] = c;
	    } else if (c==' ') {
		buf[last++] = '+';
	    } else {
		char b;
		buf[last++] = '%';
		b = c / 16;
		if (b < 10) {
		    b = b + '0';
		} else {
		    b = b - 10 + 'A';
		}
		buf[last++] = b;
		b = c % 16;
		if (b < 10) {
		    b = b + '0';
		} else {
		    b = b - 10 + 'A';
		}
		buf[last++] = b;
	    }
	}

	buf[last] = 0;
	return buf;
}


static int digit_value(char c)
{
    if ('a'<=c && c<='f') {
	return c-'a'+10;
    } else
    if ('A'<=c && c<='F') {
	return c-'A'+10;
    } else {
	return c-'0';
    }
}

char *URL_decode(const char *s, unsigned len)
{
	char *buf = (char*)malloc(len+1);
	int last = 0;
	int i=0;

	while (i<len) {
	    int c = (unsigned char)s[i];
	    if( c=='%') {
		int b = digit_value(s[i+1]);
		b = b*16 + digit_value(s[i+2]);
		i = i+2;
		buf[last++] = b;
	    } else if (c=='+') {
		buf[last++] = ' ';
	    } else {
		buf[last++] = c;
	    }
	    i++;
	}

	buf[last] = 0;
	return buf;
}



