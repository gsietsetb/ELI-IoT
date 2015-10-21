
#include "dbc.h"

#include <stdio.h>


int main(int argc, char **argv)
{
    /* Arguments:
     *		argv[1]		database's directory
     */
    if (argc != 2) {
    	printf("Usage: %s  db_root\n", argv[0]);
    	return 1;
    }

    if (DB_create_db(argv[1]) != 0) {
    	printf("Cannot create database '%s'\n", argv[1]);
    	return 1;
    }

    return 0;
}
