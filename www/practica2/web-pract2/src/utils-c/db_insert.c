
#include "dbc.h"

#include <stdio.h>


int main(int argc, char **argv)
{
    DBConnection dbc;
    StrList cvalues;
    int i;

    /* Arguments:
     *		argv[1]		database's directory
     *		argv[2]		name of table to insert
     *		argv[3]		name of first value
     *		...		...
     *		argv[argc-1]	name of last value
     */
    if (argc < 4) {
    	printf("Usage: %s  db_root  table_name  value1  value2  ...\n", argv[0]);
    	return 1;
    }

    dbc = DBC_open(argv[1]);
    if (dbc == NULL) {
    	printf("Cannot access database in directory '%s'\n", argv[1]);
    	return 1;
    }
    cvalues = StrList_create();

    for (i = 3; i < argc; i++) {
    	StrList_add(cvalues, argv[i]);
    }
    if (DBC_insert(dbc, argv[2], cvalues) == -1) {
    	printf(
		"Cannot insert (%d values) in table '%s' in directory '%s'\n",
		argc-3, argv[2], argv[1]
    );
    	return 1;
    }

    DBC_close(dbc);
    StrList_destroy(cvalues);
    return 0;
}
