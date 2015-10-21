
#include "dbc.h"

#include <stdio.h>


int main(int argc, char **argv)
{
    DBConnection dbc;

    /* Arguments:
     *		argv[1]		database's directory
     *		argv[2]		name of table to create
     */
    if (argc != 3) {
    	printf("Usage: %s  db_root  table_name\n",argv[0]);
    	return 1;
    }

    dbc = DBC_open(argv[1]);
    if (dbc == NULL) {
    	printf("Cannot access database in directory '%s'\n",argv[1]);
    	return 1;
    }
    if (DBC_drop_table(dbc,argv[2]) == -1) {
    	printf("Cannot drop table '%s' from directory '%s'\n",argv[2],argv[1]);
    	return 1;
    }
    DBC_close(dbc);
    return 0;
}
