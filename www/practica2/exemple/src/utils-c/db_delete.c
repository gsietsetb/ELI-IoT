
#include "dbc.h"
#include <stdio.h>


int main(int argc, char **argv)
{
    DBConnection dbc;

    /* Arguments:
     *		argv[1]		database's directory
     *		argv[2]		name of table to delete
     *		argv[3]		name of column with values to select
     *		argv[4]		value to select
     */
    if (argc != 3 && argc != 5) {
    	printf("Usage: %s  db_root  table_name  [col_name  value]\n",argv[0]);
    	return 1;
    }

    dbc = DBC_open(argv[1]);
    if (dbc == NULL) {
    	printf("Cannot access database in directory '%s'\n",argv[1]);
    	return 1;
    }
    if (argc == 5) {
	if (DBC_delete(dbc,argv[2],argv[3],argv[4]) == -1) {
	    printf("Cannot delete records from table '%s'\n",argv[2]);
	    return 1;
	}
    } else {
	if (DBC_delete(dbc,argv[2],NULL,NULL) == -1) {
	    printf("Cannot delete records from table '%s'\n",argv[2]);
	    return 1;
	}
    }
    DBC_close(dbc);
    return 0;
}



