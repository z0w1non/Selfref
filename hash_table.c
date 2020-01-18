#include "hash_table.h"
#include <stdlib.h>
#include "print.h"

/*********************/
/* internal function */
/*********************/
size_t string_hash(const wchar_t * s);
void table_cell_init(table_cell * cell);
void hash_table_rehash(hash_table * table);
void hash_table_try_rehash(hash_table * table);
void hash_table_move_stack(hash_table * table, wchar_t * key, data stack);

/**************/
/* Hash table */
/**************/
enum
{
    state_unused,
    state_used,
};

void hash_table_init(hash_table * table, int max_size)
{
    size_t i;
    table->data = malloc(sizeof(table_cell) * max_size);
    if (!table->data)
        goto error;
    for (i = 0; i < max_size; ++i)
        table_cell_init(&table->data[i]);
    table->max_size = max_size;
    table->used_count = 0;
    return;

error:
    error(L"hash table init failed");
    return;
}

void hash_table_cleanup(hash_table * table)
{
    size_t i;
    for (i = 0; i < table->max_size; ++i)
        free(table->data[i].key);
    memset(table, 0, sizeof(table));
}

void hash_table_push(hash_table * table, const wchar_t * key, data value)
{
    size_t hash, i, j;
    hash_table_try_rehash(&table);
    hash = string_hash(key);
    for (i = 0; i < table->max_size; ++i)
    {
        j = ((hash + i) % table->max_size);
        if ((table->data[j].state == state_unused))
        {
            table->data[j].state = state_used;
            table->data[j].key = clone_string(key);
            table->data[j].stack = make_pair(value, table->data[j].stack);
            table->used_count += 1;
            return;
        }
        else if (((table->data[j].state == state_used) && (wcscmp(table->data[j].key, key) == 0)))
        {
            table->data[j].stack = make_pair(value, table->data[j].stack);
        }
    }
    error(L"hash table push failed");
}

void hash_table_pop(hash_table * table, const wchar_t * key)
{
    size_t hash, i, j;
    data d;
    hash = string_hash(key);
    for (i = 0; i < table->max_size; ++i)
    {
        j = ((hash + i) % table->max_size);
        if ((table->data[j].state == state_used) && (wcscmp(table->data[j].key, key) == 0))
        {
            if (is_not_nil(table->data[j].stack))
            {
                d = cdr(table->data[j].stack);
                free_data(table->data[j].stack);
                table->data[j].stack = d;
            }
            return;
        }
    }
    error(L"hash table pop failed");
}

data hash_table_override(hash_table * table, const wchar_t * key, data value)
{
    size_t hash, i, j;
    hash = string_hash(key);
    for (i = 0; i < table->max_size; ++i)
    {
        j = ((hash + i) % table->max_size);
        if ((table->data[j].state == state_used) && (wcscmp(table->data[j].key, key) == 0))
        {
            if (is_not_nil(table->data[j].stack))
            {
                set_car(table->data[j].stack, value);
                return(value);
            }
            return(NULL);
        }
        else if (table->data[j].state == state_unused)
        {
            table->data[j].state = state_used;
            table->data[j].key = clone_string(key);
            table->data[j].stack = make_pair(value, table->data[j].stack);
            table->used_count += 1;
            return(value);
        }
    }
    return(NULL);
}

data hash_table_find(hash_table * table, const wchar_t * key)
{
    size_t hash;
    int i, j;
    hash = string_hash(key);
    for (i = 0; i < table->max_size; ++i)
    {
        j = ((hash + i) % table->max_size);
        if ((table->data[j].state == state_used) && (wcscmp(table->data[j].key, key) == 0))
        {
            if (is_not_nil(table->data[j].stack))
                return(car(table->data[j].stack));
            return(NULL);
        }
        if (table->data[j].state == state_unused)
            return(NULL);
    }
    return(NULL);
}

void hash_table_mark(hash_table * table)
{
    size_t hash, i;
    data d;
    for (i = 0; i < table->max_size; ++i)
    {
        if (table->data[i].state == state_used)
        {
            d = table->data[i].stack;
            while (is_not_nil(d))
            {
                mark_data(d);
                mark_data(car(d));
                d = cdr(d);
            }
        }
    }
    return(NULL);
}

void hash_table_iterate(hash_table * table, int (*callback)(const wchar_t *, data))
{
    size_t i;
    data d;
    for (i = 0; i < table->max_size; ++i)
        if (table->data[i].state == state_used)
        {
            d = table->data[i].stack;
            while (is_not_nil(d))
            {
                if (callback(table->data[i].key, (car(d))))
                    return;
                d = cdr(d);
            }
        }
}

/*********************/
/* Internal function */
/*********************/
size_t string_hash(const wchar_t * s)
{
    size_t hash_value = 0;
    while (*s)
    {
        hash_value = (hash_value * 257) ^ (*s);
        s += 1;
    }
    return(hash_value);
}

void table_cell_init(table_cell * cell)
{
    cell->state = state_unused;
    cell->key = NULL;
    cell->stack = nil;
}

void hash_table_rehash(hash_table * table)
{
    size_t i;
    hash_table new_table;
    hash_table_init(&new_table, table->max_size * 2);
    for (i = 0; i < table->max_size; ++i)
        if (table->data[i].state == state_used)
        {
            hash_table_move_stack(&new_table, table->data[i].key, table->data[i].stack);
            table->data[i].key = NULL;
        }

    hash_table_cleanup(table);
    *table = new_table;
}

void hash_table_try_rehash(hash_table * table)
{
    if (((double)table->used_count / table->max_size) >= 0.75)
        hash_table_rehash(table);
}

void hash_table_move_stack(hash_table * table, wchar_t * key, data stack)
{
    size_t i;
    i = (string_hash(key) % table->max_size);
    table->data[i].state = state_used;
    table->data[i].key = key;
    table->data[i].stack = stack;
}
