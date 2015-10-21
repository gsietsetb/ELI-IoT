
#include "dbc.h"
#include "urlcoding.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>	/* chmod() */


#define DIR_MOD		0700
#define FILE_MOD	0600

#define UPDATE_NAME	"dbc:update"
#define DATA_PREFIX	"dbc:data:"


/* Forward declarations */
static void DBC_clear_result(DBConnection dbc);

static int DBC_write_record(FILE *f, StrList list);
static int DBC_read_record_in(FILE *f, StrList list);
	/* Return 0 if OK, -1 if end of file or ERROR */

#define DBC_tmp_file(dbc)	strdcat((dbc)->db_root, UPDATE_NAME)

static char *DBC_table_filename(DBConnection dbc, const char *table)
{
	char *s1, *s2, *s3;

	s1 = URL_encode(table, strlen(table));
	s2 = strdcat(DATA_PREFIX, s1);
	s3 = strdcat(dbc->db_root, s2);
	free(s1);
	free(s2);
    	return s3;
}


/* Database creation/deletion */

int DB_create_db(const char *db_directory)
{
    int err;
    FILE *f;
    char *tmp_file;

    if (db_directory[strlen(db_directory) - 1] == '/') {
	tmp_file = strdcat(db_directory, UPDATE_NAME);
    } else {
	tmp_file = strdcat(db_directory, "/" UPDATE_NAME);
    }
    err = mkdir(db_directory, DIR_MOD);
    if (err != 0) {
	return -1;
    }

    f = fopen(tmp_file, "w");
    if (f == NULL) {
	free(tmp_file);
    	return -1;
    }
    fclose(f);
    chmod(tmp_file, FILE_MOD);
    free(tmp_file);

    return 0;
}


/* Database management */

DBConnection DBC_open(const char *db_directory)
{
    DBConnection dbc;
    FILE *f;
    char *tmp_file;

    dbc = (DBConnection) malloc(sizeof (struct DBC_struct));
    if (db_directory[strlen(db_directory) - 1] == '/') {
    	dbc->db_root = strdcat(db_directory, "");
    } else {
    	dbc->db_root = strdcat(db_directory, "/");
    }
    chmod(db_directory, DIR_MOD);

    tmp_file = DBC_tmp_file(dbc);
    f = fopen(tmp_file, "w");
    chmod(tmp_file, FILE_MOD);
    free(tmp_file);
    if (f == NULL) {
	free(dbc->db_root);
	free(dbc);
    	return NULL;
    }
    fclose(f);

    dbc->result_names = StrList_create();
    dbc->result_row_capacity = 10;
    dbc->result_row_mem = (StrList*) malloc(dbc->result_row_capacity * sizeof(StrList));
    dbc->result_row_size = 0;
    dbc->result_row_actual = -1;

    return dbc;
}


void DBC_close(DBConnection dbc)
{
    if (dbc != NULL) {
	DBC_clear_result(dbc);
	free(dbc->result_row_mem);
	StrList_destroy(dbc->result_names);
	free(dbc->db_root);
	free(dbc);
    }
}


static void DBC_clear_result(DBConnection dbc)
{
	int i;
	for (i=0; i < dbc->result_row_size; i++) {
		StrList_destroy(dbc->result_row_mem[i]);
		dbc->result_row_mem[i] = NULL;
	}
	dbc->result_row_size = 0;
	StrList_clear(dbc->result_names);
	dbc->result_row_actual = -1;
}


int DBC_create_table(DBConnection dbc, const char *table, StrList cols)
{
	FILE *tf;
	char *tab_file;

	tab_file = DBC_table_filename(dbc, table);
	tf = fopen(tab_file,"r");
	if (tf != NULL) {
		/* Table already exist ! */
		fclose(tf);
		free(tab_file);
		return -1;
	}
	tf = fopen(tab_file, "w");
	chmod(tab_file, FILE_MOD);

	DBC_write_record(tf, cols);
	fclose(tf);
	free(tab_file);
	return 0;
}


int DBC_drop_table(DBConnection dbc, const char *table)
{
	int result;
	char *s1;

	s1 = DBC_table_filename(dbc, table);
	result = remove(s1);
	free(s1);
	return result;
}


int DBC_select(
	DBConnection dbc, const char *table,
	const char *sname, const char *svalue
)
{
	FILE *tf;
	int sindex;
	StrList values;
	char *tab_file;

	DBC_clear_result(dbc);

	tab_file = DBC_table_filename(dbc, table);
	tf = fopen(tab_file, "r");
	free(tab_file);
    	if (tf==NULL) {
	    return -1;
    	}
	if (DBC_read_record_in(tf, dbc->result_names) == -1) {
		fclose(tf);
		return -1;
	}
	if (sname == NULL) {
		sindex = -1;
	} else {
		sindex = StrList_index(dbc->result_names, sname);
		if (sindex == -1 || svalue == NULL) {
			StrList_clear(dbc->result_names);
			fclose(tf);
			return -1;
		}
	}

	values = StrList_create();
	while (DBC_read_record_in(tf, values) != -1) {
	    if (sindex == -1 || strcmp(StrList_get(values, sindex), svalue) == 0) {
	    	if (dbc->result_row_size >= dbc->result_row_capacity) {
		    /* Increments result_row_mem capacity */
		    dbc->result_row_capacity = 2 * (dbc->result_row_capacity + 1);
		    dbc->result_row_mem = (StrList*) realloc(dbc->result_row_mem,
			dbc->result_row_capacity * sizeof(StrList)
		    );
	    	}
	    	dbc->result_row_mem[dbc->result_row_size ++] = values;
	    	values = StrList_create();
	    }
	}
	StrList_destroy(values);

	fclose(tf);
	return 0;
}


int DBC_next(DBConnection dbc)
{
	if (StrList_size(dbc->result_names) == 0) {
	    return -1;
	}
	dbc->result_row_actual ++;
	if (dbc->result_row_actual >= dbc->result_row_size) {
	    dbc->result_row_actual = dbc->result_row_size;
	    return -1;
	}
	return 0;
}


char *DBC_get(DBConnection dbc, const char *cname)
{
	int cindex;

	if (dbc->result_row_actual < 0) {
		return NULL;
	}
	if (dbc->result_row_actual >= dbc->result_row_size) {
		return NULL;
	}
	cindex = StrList_index(dbc->result_names, cname);
	if (cindex == -1) {
		return NULL;
	}
	return strdcat("", StrList_get(dbc->result_row_mem[dbc->result_row_actual], cindex));
}


int DBC_insert(DBConnection dbc, const char *table, StrList cvalues)
{
	FILE *tf;
	StrList cnames;
	char *tab_file;

	tab_file = DBC_table_filename(dbc, table);
	tf = fopen(tab_file, "r");
	if (tf == NULL) {
		free(tab_file);
		return -1;
	}
	cnames = StrList_create();
	if (DBC_read_record_in(tf, cnames) == -1) {
		fclose(tf);
		StrList_destroy(cnames);
		free(tab_file);
		return -1;
	}
	fclose(tf);

	if (StrList_size(cnames) != StrList_size(cvalues)) {
		StrList_destroy(cnames);
		free(tab_file);
		return -1;
	}
	StrList_destroy(cnames);

	tf = fopen(tab_file, "a");
	free(tab_file);
	if (tf == NULL) {
		return -1;
	}
	DBC_write_record(tf, cvalues);
	fclose(tf);
	return 0;
}


/**
 * Modifica el valor del camp 'uname' al valor 'uvalue'
 * dels registres de la taula 'table' que tenen el camp 'sname' igual a 'svalue'.
 */
int DBC_update(
	DBConnection dbc, const char *table,
	const char *uname, const char *uvalue,
	const char *sname, const char *svalue
)
{
	FILE *tf, *uf;
	int i, sindex, uindex;
	StrList cnames, cvalues;
	char *tab_file, *tmp_file;

	tab_file = DBC_table_filename(dbc, table);
	tf = fopen(tab_file, "r");
    	if (tf==NULL) {
	    free(tab_file);
	    return -1;
    	}
    	cnames = StrList_create();
	if (DBC_read_record_in(tf, cnames) == -1) {
	    free(tab_file);
	    fclose(tf);
	    StrList_destroy(cnames);
	    return -1;
	}
	if (sname == NULL) {
	    sindex = -1;
	} else {
	    sindex = StrList_index(cnames, sname);
	    if (sindex == -1 || svalue == NULL) {
		StrList_destroy(cnames);
		fclose(tf);
		free(tab_file);
		return -1;
	    }
	}
	uindex = StrList_index(cnames, uname);
	if (uindex == -1 || uvalue == NULL) {
	    StrList_destroy(cnames);
	    fclose(tf);
	    free(tab_file);
	    return -1;
	}

	tmp_file = DBC_tmp_file(dbc);
	uf = fopen(tmp_file, "w");
	if (uf == NULL) {
	    StrList_destroy(cnames);
	    fclose(tf);
	    free(tab_file);
	    free(tmp_file);
	    return -1;
	}
	DBC_write_record(uf, cnames);
	cvalues = StrList_create();
	while (DBC_read_record_in(tf, cvalues) != -1) {
	    if (sindex == -1 || strcmp(StrList_get(cvalues, sindex), svalue) == 0) {
	    	StrList_set(cvalues, uindex, uvalue);
	    }
	    DBC_write_record(uf, cvalues);
	}
	fclose(uf);
	fclose(tf);
	StrList_destroy(cnames);
	StrList_destroy(cvalues);

	i = rename(tmp_file, tab_file);
	chmod(tab_file, FILE_MOD);
	free(tab_file);
	free(tmp_file);
	return i;
}


int DBC_delete(
	DBConnection dbc, const char *table,
	const char *sname, const char *svalue
)
{
	FILE *tf, *uf;
	int sindex, i;
	StrList cnames, cvalues;
	char *tab_file, *tmp_file;

	tab_file = DBC_table_filename(dbc,table);
	tf = fopen(tab_file,"r");
    	if (tf==NULL) {
	    free(tab_file);
	    return -1;
    	}
    	cnames = StrList_create();
	if (DBC_read_record_in(tf,cnames) == -1) {
	    fclose(tf);
	    StrList_destroy(cnames);
	    free(tab_file);
	    return -1;
	}
	if (sname == NULL) {
	    sindex = -1;
	} else {
	    sindex = StrList_index(cnames, sname);
	    if (sindex == -1 || svalue == NULL) {
		StrList_destroy(cnames);
		fclose(tf);
		free(tab_file);
		return -1;
	    }
	}

	tmp_file = DBC_tmp_file(dbc);
	uf = fopen(tmp_file,"w");
	if (uf == NULL) {
	    StrList_destroy(cnames);
	    fclose(tf);
	    free(tab_file);
	    free(tmp_file);
	    return -1;
	}
	DBC_write_record(uf, cnames);
	cvalues = StrList_create();
	while (DBC_read_record_in(tf, cvalues) != -1) {
	    if (sindex != -1 && strcmp(StrList_get(cvalues, sindex), svalue) != 0) {
	    	DBC_write_record(uf,cvalues);
	    }
	}
	fclose(uf);
	fclose(tf);
	StrList_destroy(cnames);
	StrList_destroy(cvalues);

	i = rename(tmp_file, tab_file);
	chmod(tab_file, FILE_MOD);
	free(tab_file);
	free(tmp_file);
	return i;
}



static int DBC_write_record(FILE *f, StrList list)
{
	int i;
	char *s1, *s2;

	fprintf(f,"%d ",StrList_size(list));
	for (i = 0; i < StrList_size(list); i++) {
		s1 = StrList_get(list, i);
		s2 = URL_encode(s1, strlen(s1));
		fprintf(f,"%s ",s2);
		free(s2);
	}
	fprintf(f,"\n");
	return 0;
}


static int DBC_read_record_in(FILE *f, StrList list)
{
	char *s1, *tok, *next_tok;
	int capacity, len, rsize, i;

	len = 0;
	capacity = 100;
	s1 = (char*)malloc(capacity);
	while (fgets(s1+len, capacity-len, f) != NULL) {
		len = strlen(s1);
		if (s1[len-1] == '\n') {
			len --;
			s1[len] = 0;
			break;
		}
		if (capacity-1 == len) {
			capacity *= 2;
			s1 = realloc(s1,capacity);
		}
	}

	if (len == 0) {
		free(s1);
		return -1;
	}

	StrList_clear(list);
	tok = s1; next_tok = strchr(tok,' '); *next_tok = 0;
	rsize = atoi(tok);
	for (i=0; i < rsize; i++) {
		char *decod;
		tok = next_tok+1; next_tok = strchr(tok, ' '); *next_tok = 0;
		decod = URL_decode(tok, (unsigned)(next_tok-tok));
		StrList_add(list, decod);
		free(decod);
	}
	free(s1);
	return rsize;
}


