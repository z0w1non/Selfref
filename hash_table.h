#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <wchar.h>
#include "data.h"

/**************/
/* Hash table */
/**************/
typedef struct table_cell_tag
{
    int state;
    wchar_t * key;
    data stack;
} table_cell;

typedef struct hash_table_tag
{
    table_cell * data;
    int max_size;
    int used_count;
} hash_table;

void hash_table_init(hash_table * table, int length);
void hash_table_cleanup(hash_table * table);
void hash_table_push(hash_table * table, const wchar_t * key, data value);
void hash_table_pop(hash_table * table, const wchar_t * key);
data hash_table_override(hash_table * table, const wchar_t * key, data value);
data hash_table_find(hash_table * table, const wchar_t * key);
void hash_table_mark(hash_table * table);
void hash_table_iterate(hash_table * table, int (*callback)(const wchar_t *, data));

#endif
