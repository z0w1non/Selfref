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

typedef struct tablecell_tag
{
    int state;
    wchar_t * key;
    data stack;
} tablecell;

typedef struct hashtable_tag
{
    tablecell * data;
    int len;
    int usedcnt;
} hashtable;

void inithashtable(hashtable * table, int len);
void freehashtable(hashtable * table);

hashtable table;

/**********/
/* Rehash */
/**********/
void rehash();
void tryrehash();

/*********************/
/* Internal function */
/*********************/
int strhash(const wchar_t * s);
wchar_t * clone_string(const wchar_t * s);
void movestack(hashtable * table, const wchar_t * key, data stack);

/**************/
/* Hash table */
/**************/
void inithashtable(hashtable * table, int len)
{
    int i;
    table->data = malloc(sizeof(tablecell) * len);
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

void freehashtable(hashtable * table)
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
    hashtable newtable;
    int newlen, i;

    if (table.len == 0)
        newlen = 0x400;
    else
        newlen = table.len * 2;

    inithashtable(&newtable, newlen);

    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            movestack(&newtable, table.data[i].key, table.data[i].stack);
            table.data[i].key = NULL;
        }

    freehashtable(&table);
    table = newtable;
}

void tryrehash()
{
    if (((double)table.usedcnt / table.len) >= 0.75)
        rehash();
}

/*********************/
/* Internal function */
/*********************/
unsigned int strhash(const wchar_t * s)
{
    unsigned int i = 0;
    while (*s)
    {
        i = (i * 257) ^ (*s);
        s += 1;
    }
    return(i);
}

void movestack(hashtable * table, const wchar_t * key, data stack)
{
    int i;
    data d;
    i = (strhash(key) % table->len);
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
    freehashtable(&table);
}

void cpush_symbol(const wchar_t * key, data value)
{
    unsigned int hash;
    int i, j;
    data d;

    tryrehash();

    hash = strhash(key);
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

    hash = strhash(key);
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
    hash = strhash(key);
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
