/*  urlcoding.h
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


#ifndef _URLCODING_H_
#define _URLCODING_H_

#include <stdio.h>


typedef struct Form_struct {
    int capacity, size;
    struct FormElem {
	char *name;
	char **values;
	int count;
    } *elems;
} *Form;


Form Form_create(const char *encoded);
void Form_destroy(Form form);
int Form_getCount(Form form);
const char *Form_getName(Form form, int i);
const char **Form_getValues(Form form, const char *name, int *pValueCount);
const char *Form_get(Form form, const char *name);
void Form_clear(Form form);
void Form_remove(Form form, const char *name);
void Form_add(Form form, const char *name, const char *value);
char *Form_encode(Form form);
void Form_printEncoded(Form form, FILE *stream);

/* Deprecated */
int Form_get_count(Form form, const char *name);
const char *Form_get_i(Form form, const char *name, int index);
#define Form_print_encoded(form, stream)	Form_printEncoded(form, stream)

    
char *URL_encode(const char *s, unsigned len);
char *URL_decode(const char *s, unsigned len);


char *strdcat(const char *s1, const char *s2);
	/* Obté un nou string (en memòria dinàmica) format per la concatenació
	 * dels 2 paràmetres 's1' i 's2'.
	 */


char *read_std_file(FILE *stream, int len);
	/* Llegeix i construeix una cadena de caracters (en memòria dinàmica).
	 * Parametres:
	 *	stream	stream d'entrada.
	 *	len	nombre màxim de bytes a llegir (pot ser menor si
	 *		 abans detecta fi de stream o error).
	 *		 Si len < 0, aleshores llegeix fins fi de stream o error.
	 */


#endif


