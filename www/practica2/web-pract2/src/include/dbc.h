
#ifndef _DBC_H_
#define _DBC_H_

#include "strlist.h"

typedef struct DBC_struct {
	char *db_root;
	StrList result_names;
	StrList *result_row_mem;
	int result_row_capacity;
	int result_row_size;
	int result_row_actual;
} *DBConnection;



/*********************************************************/
/* Database management */

DBConnection DBC_open(const char *db_directory);
/**
 * Crea una connexió amb la base de dades.
 * Retorna NULL si ERROR.
 */

void DBC_close(DBConnection dbc);
/**
 * Allibera la connexió.
 * Després de la invocació a aquesta funció, ja no es podrà usar
 * el valor de 'dbc'.
 */


int DBC_create_table(DBConnection dbc, const char *table, StrList cols);
/**
 * Crea una taula amb el nom indicat en 'table' i els camps indicats
 * en 'cols'.
 * Retorna 0 si OK, -1 si ERROR.
 */

int DBC_drop_table(DBConnection dbc, const char *table);
/**
 * Destrueix la taula indicada
 */


int DBC_select(
	DBConnection dbc, const char *table,
	const char *sname, const char *svalue
);
/**
 * Obté en una llista temporal (interna) el conjunt dels registres de la
 * taula 'table' en els que el valor del camp 'sname' és igual a 'svalue'.
 * En cas de 'sname==NULL' aleshores obté tots els registres de la taula.
 * Després d'aquesta operació, caldrà utilitzar 'DBC_next' per posicionar-se
 * al primer registre del resultat.
 */

int DBC_next(DBConnection dbc);
/**
 * Avança el cursor de registre actual.
 * Retorna 0 si OK, -1 si no pot avançar (final de resultat). 
 */

char *DBC_get(DBConnection dbc, const char *cname);
/**
 * Obté el valor del camp 'cname' del registre actual. 
 */


int DBC_insert(DBConnection dbc, const char *table, StrList cvalues);
/**
 * Afegeix un registre en la taula 'table'.
 * 'cvalues' és la llista de valors dels camps del registre.
 * La quantitat i l'ordre dels valors ha de correspondre a la quantitat i ordre
 * dels camps definits en la taula indicada.
 * Obté 0 si OK, -1 si ERROR.
 */


int DBC_update(
	DBConnection dbc, const char *table,
	const char *uname, const char *uvalue,
	const char *sname, const char *svalue
);
/**
 * Modifica el valor del camp 'uname' al valor 'uvalue'
 * dels registres de la taula 'table' que tenen el camp 'sname' igual a 'svalue'.
 * En cas de 'sname==NULL' aleshores modifica tots els registres de la taula.
 */


int DBC_delete(
	DBConnection dbc, const char *table,
	const char *sname, const char *svalue
);
/**
 * Elimina els registres que tenen el camp 'sname' igual a 'svalue'.
 * En cas de 'sname==NULL' aleshores elimina tots els registres de la taula.
 */


/*********************************************************/
/* Database creation/deletion */

int DB_create_db(const char *db_directory);


#endif
