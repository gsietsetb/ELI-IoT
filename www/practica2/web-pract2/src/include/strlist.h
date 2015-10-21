
#ifndef _STRLIST_H_
#define _STRLIST_H_

typedef struct StrList_struct {
	int size, capacity;
	char **items;
} *StrList;


StrList StrList_create(void);
	/**
	 * Crea una llista buida.
	 */

void StrList_destroy(StrList list);
	/**
	 * Allibera la memòria de la llista.
	 * Després de la invocació a aquesta funció, ja no es podrà usar
	 * el valor de 'list'.
	 */


int StrList_size(StrList list);
	/**
	 * Obté el tamany de la llista (quantitat d'elements).
	 */
#define StrList_size(sl)	((sl)->size)

const char *StrList_get(StrList list, int i);
	/**
	 * Obté l'element en la posició 'i'. La posició del primer element és 0.
	 * Precondició: 0 <= i && i < StrList_size(list)
	 */

int StrList_index(StrList list, const char *value);
	/**
	 * Obté la posició del primer element igual a 'value'.
	 * Si no hi ha cap element igual a 'value', aleshores retorna -1.
	 */

void StrList_set(StrList list, int i, const char *value);
	/**
	 * Modifica l'element en la posició 'i' al valor 'value'.
	 * La posició del primer element és 0.
	 * Precondició: 0 <= i && i < StrList_size(list)
	 */

void StrList_add(StrList list, const char *value);
	/**
	 * Afegeix un nou element al final de la llista igual a 'value'.
	 * El tamany de la llista quedarà incrementat una unitat.
	 */

void StrList_clear(StrList list);
	/**
	 * Elimina tots els elements de la llista, fent que el tamany sigui 0.
	 */


#endif
