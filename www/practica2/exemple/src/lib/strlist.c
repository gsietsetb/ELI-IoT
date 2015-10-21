
#include "strlist.h"
#include "urlcoding.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


StrList StrList_create(void)
{
    StrList list = (StrList) malloc(sizeof(struct StrList_struct));
    list->capacity = 10;
    list->size = 0;
    list->items = (char**) malloc(list->capacity * sizeof(const char*));
    return list;
}


void StrList_destroy(StrList list)
{
    int i;
    for (i = 0; i < list->size; i++) {
	free(list->items[i]);
    }

    free(list->items);
    free(list);
}


const char *StrList_get(StrList list, int i)
{
    if (0 <= i && i < list->size) return list->items[i];
    else return NULL;
}


int StrList_index(StrList list, const char *value)
{
    int i;
    for (i = 0; i < list->size; i++) {
	if (strcmp(value, list->items[i]) == 0) {
		return i;
	}
    }
    return -1;
}


void StrList_clear(StrList list)
{
    int i;
    for (i = 0; i < list->size; i++) {
	free(list->items[i]);
    }
    list->size = 0;
}


void StrList_set(StrList list, int i, const char *value)
{
    assert(0 <= i && i < list->size);
    free(list->items[i]);
    list->items[i] = strdcat("", value);
}


void StrList_add(StrList list, const char *value)
{
    if (list->size == list->capacity) {
	list->capacity = 2 * list->capacity;
	list->items = (char**) realloc(list->items, list->capacity * sizeof(const char*));
    }
    list->items[list->size] = strdcat("", value);
    list->size ++;
}

