#include "sym.h"

#include <stdlib.h>
#include "builtin.h"
#include "eval.h"
#include "heap.h"
#include "print.h"

/**************/
/* Hash table */
/**************/
enum
{
    state_unused,
    state_used,
    //state_deleted,
};

typedef struct table_cell_tag
{
    int state;
    wchar_t * key;
    data stack;
} table_cell;

typedef struct hash_table_tag
{
    table_cell * data;
    int len;
    int usedcnt;
} hash_table;

void hash_table_init(hash_table * table, int len);
void hash_table_cleanup(hash_table * table);

hash_table table;

/**********/
/* Rehash */
/**********/
void rehash();
void try_rehash();

/*********************/
/* Internal function */
/*********************/
unsigned int string_hash(const wchar_t * s);
wchar_t * clone_string(const wchar_t * s);
void move_stack(hash_table * table, wchar_t * key, data stack);

/**************/
/* Hash table */
/**************/
void hash_table_init(hash_table * table, int len)
{
    int i;
    table->data = malloc(sizeof(table_cell) * len);
    if (!table->data)
        error(L"bad alloc.\n");
    for (i = 0; i < len; ++i)
    {
        table->data[i].state = state_unused;
        table->data[i].key = NULL;
        table->data[i].stack = nil;
    }
    table->len = len;
    table->usedcnt = 0;
}

void hash_table_cleanup(hash_table * table)
{
    int i;
    for (i = 0; i < table->len; ++i)
        free(table->data[i].key);
    memset(table, 0, sizeof(table));
}

/**********/
/* Rehash */
/**********/
void rehash()
{
    hash_table newtable;
    int newlen, i;

    if (table.len == 0)
        newlen = 0x400;
    else
        newlen = table.len * 2;

    hash_table_init(&newtable, newlen);

    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            move_stack(&newtable, table.data[i].key, table.data[i].stack);
            table.data[i].key = NULL;
        }

    hash_table_cleanup(&table);
    table = newtable;
}

void try_rehash()
{
    if (((double)table.usedcnt / table.len) >= 0.75)
        rehash();
}

/*********************/
/* Internal function */
/*********************/
unsigned int string_hash(const wchar_t * s)
{
    unsigned int i = 0;
    while (*s)
    {
        i = (i * 257) ^ (*s);
        s += 1;
    }
    return(i);
}

void move_stack(hash_table * table, wchar_t * key, data stack)
{
    int i;
    i = (string_hash(key) % table->len);
    table->data[i].state = state_used;
    table->data[i].key = key;
    table->data[i].stack = stack;
}

/***********************/
/* Symbol map operator */
/***********************/
void init_symbol_stack()
{
    rehash();
    init_builtin();
}

void cleanup_symbol_stack()
{
    hash_table_cleanup(&table);
}

void cpush_symbol(const wchar_t * key, data value)
{
    unsigned int hash;
    int i, j;

    try_rehash();

    hash = string_hash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_unused))
        {
            table.data[j].state = state_used;
            table.data[j].key = clone_string(key);
            table.data[j].stack = make_pair(value, table.data[j].stack);
            table.usedcnt += 1;
            return;
        }
        else if(((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0)))
        {
            table.data[j].stack = make_pair(value, table.data[j].stack);
        }
    }
    error(L"pushsymimpl failed.\n");
}

void cpop_symbol(const wchar_t * key)
{
    unsigned int hash;
    int i, j;
    data d;

    hash = string_hash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0))
        {
            if (is_not_nil(table.data[j].stack))
            {
                d = cdr(table.data[j].stack);
                free_data(table.data[j].stack);
                table.data[j].stack = d;
            }
            return;
        }
    }
    error(L"popsym failed.\n");
}

data find_symbol(const wchar_t * key)
{
    unsigned int hash;
    int i, j;
    hash = string_hash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0))
        {
            if (is_not_nil(table.data[j].stack))
                return(car(table.data[j].stack));
            return(NULL);
        }
        if (table.data[j].state == state_unused)
            return(NULL);
    }
    return(NULL);
}

data replace_symbol(const wchar_t * key, data value)
{
    unsigned int hash;
    int i, j;
    hash = string_hash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0))
        {
            if (is_not_nil(table.data[j].stack))
            {
                set_car(table.data[j].stack, value);
                return(value);
            }
            return(NULL);
        }
        if (table.data[j].state == state_unused)
            return(NULL);
    }
    return(NULL);
}

void mark_symbol()
{
    int i;
    data d;
    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            d = table.data[i].stack;
            while(is_not_nil(d))
            {
                mark_data(car(d));
                d = cdr(d);
            }
        }
}

data _dump_symbol(data d)
{
    int i;
    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            wprintf(L"(\"%s\" ", table.data[i].key);
            print(car(table.data[i].stack));
            wprintf(L")\n");
        }
    return(t);
}

data _push_args(data d)
{
    if (is_nil(d))
        return(d);
    if (!is_pair(car(d)))
        error(L"pushargs arguments is must be list.\n");
    if ((!is_symbol(caar(d))) || (!is_pair(cdar(d))))
        error(L"invalid key at pushargs.\n");
    cpush_symbol(raw_string(caar(d)), car(cdar(d)));
    return(_push_args(cdr(d)));
}

data _pop_args(data d)
{
    if (is_nil(d))
        return(d);
    if (!is_pair(car(d)))
        error(L"popargs arguments is must be list.\n");
    if ((!is_symbol(caar(d))))
        error(L"invalid key at popargs.\n");
    cpop_symbol(raw_string(caar(d)));
    return(_pop_args(cdr(d)));
}
