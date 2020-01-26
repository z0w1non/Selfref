#include "symbol.h"

#include <stdlib.h>
#include "builtin.h"
#include "hash_table.h"
#include "eval.h"
#include "heap.h"
#include "print.h"

/***********************/
/* Symbol map operator */
/***********************/
hash_table symbol_table;

void init_symbol_stack()
{
    hash_table_init(&symbol_table, 0x4000);
    init_builtin();
}

void cleanup_symbol_stack()
{
    hash_table_cleanup(&symbol_table);
}

void push_symbol(const wchar_t * key, data value)
{
    hash_table_push(&symbol_table, key, value);
}

void cpop_symbol(const wchar_t * key)
{
    hash_table_pop(&symbol_table, key);
}

data override_symbol(const wchar_t * key, data value)
{
    return(hash_table_override(&symbol_table, key, value));
}

data find_symbol(const wchar_t * key)
{
    return(hash_table_find(&symbol_table, key));
}

void mark_symbol()
{
    hash_table_mark(&symbol_table);
}

int dump_symbol_callback(const wchar_t * key, data value)
{
    wprintf(L"(\"%s\" ", key);
    print(value);
    wprintf(L")\n");
    return(0);
}

data _dump_symbol(data d)
{
    hash_table_iterate(&symbol_table, dump_symbol_callback);
    return(t);
}

data _push_args(data d)
{
    if (is_nil(d))
        return(d);
    if (!is_pair(car(d)))
        error(L"pushargs arguments is must be list");
    if ((!is_symbol(caar(d))) || (!is_pair(cdar(d))))
        error(L"invalid key at pushargs");
    push_symbol(raw_string(caar(d)), car(cdar(d)));
    return(_push_args(cdr(d)));
}

data _pop_args(data d)
{
    if (is_nil(d))
        return(d);
    if (!is_pair(car(d)))
        error(L"popargs arguments is must be list");
    if ((!is_symbol(caar(d))))
        error(L"invalid key at popargs");
    cpop_symbol(raw_string(caar(d)));
    return(_pop_args(cdr(d)));
}
