
#include "dbc.h"
#include <stdio.h>


int main(int argc, char **argv)
{
    DBConnection dbc;
    int s;

    /* Arguments:
     *		argv[1]		database's directory
     *		argv[2]		name of table
     *		argv[3]		name of column to show
     *		argv[4]		name of column with values to select
     *		argv[5]		value to select
     */
    if (argc != 4 && argc != 6) {
    	printf("Usage: %s  db_root  table_name  show_col  [select_col  select_value]\n",argv[0]);
    	return 1;
    }

    dbc = DBC_open(argv[1]);
    if (dbc == NULL) {
    	printf("Cannot access database in directory '%s'\n",argv[1]);
    	return 1;
    }
    if (argc == 6) {
	s = DBC_select(dbc,argv[2],argv[4],argv[5]);
    } else {
	s = DBC_select(dbc,argv[2],NULL,NULL);
    }
    if (s == -1) {
    	printf("Error in select (table=%s, column=%s)\n",argv[2],argv[3]);
    	return 1;
    }
    while (DBC_next(dbc) != -1) {
	printf("%s\n",DBC_get(dbc,argv[3]));
    }
    DBC_close(dbc);
    return 0;
}

