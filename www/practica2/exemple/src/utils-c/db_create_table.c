
#include "dbc.h"

#include <stdio.h>


int main(int argc, char **argv)
{
    DBConnection dbc;
    StrList cnames;
    int i;

    /* Arguments:
     *		argv[1]		database's directory
     *		argv[2]		name of table to create
     *		argv[3]		name of first column
     *		...		...
     *		argv[argc-1]	name of last column
     */
    if (argc < 4) {
    	printf("Usage: %s  db_root  table_name  col1_name  col2_name  ...\n", argv[0]);
    	return 1;
    }

    dbc = DBC_open(argv[1]);
    if (dbc == NULL) {
    	printf("Cannot access database in directory '%s'\n", argv[1]);
    	return 1;
    }
    cnames = StrList_create();

    for (i = 3; i < argc; i++) {
    	StrList_add(cnames, argv[i]);
    }
    if (DBC_create_table(dbc,argv[2],cnames) == -1) {
    	printf("Cannot create table '%s' in directory '%s'\n", argv[2], argv[1]);
    	return 1;
    }

    DBC_close(dbc);
    StrList_destroy(cnames);
    return 0;
}
