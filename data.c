#include "data.h"

#include <stdlib.h>
#include "heap.h"
#include "print.h"

enum
{
    id_pair,
    id_builtin_macro,
    id_builtin_function,
    id_unnamed_macro,
    id_unnamed_function,
    id_symbol,
    id_nil,
    id_t,
    id_int,
    id_double,
    id_string,
    mask_used = 16,
    mask_marked = 32,
};

/****************************/
/* Data infomation accesser */
/****************************/
void init_data(data d)
{
    memset(d, 0, sizeof(dataimpl));
}

int used(data d)
{
    return((d->info & mask_used) != 0);
}

void set_used(data d, int used)
{
    if (used)
        d->info |= mask_used;
    else
        d->info &= ~((int)mask_used);
}

int marked(data d)
{
    return((d->info & mask_marked) != 0);
}

void set_marked(data d, int marked)
{
    if (marked)
        d->info |= mask_marked;
    else
        d->info &= ~((int)mask_marked);
}

int type_id(data d)
{
    return(d->info & 15);
}

/*****************/
/* Make function */
/*****************/
data make_pair(data first, data rest)
{
    data d = alloc();
    d->info |= id_pair;
    d->buffer.first = first;
    d->buffer.rest = rest;
    return(d);
}

data make_builtin_macro(func_t f)
{
    data d = alloc();
    d->info |= id_builtin_macro;
    d->buffer.function = f;
    return(d);
}

data make_builtin_function(func_t f)
{
    data d = alloc();
    d->info |= id_builtin_function;
    d->buffer.function = f;
    return(d);
}

data make_macro(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_macro;
    d->buffer.first = args;
    d->buffer.rest = impl;
    return(d);
}

data make_function(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_function;
    d->buffer.first = args;
    d->buffer.rest = impl;
    return(d);
}

data make_symbol(const wchar_t * name)
{
    wchar_t * newname;
    data d;
    d = alloc();
    d->info |= id_symbol;
    int buflen = wcslen(name) + 1;
    newname = (wchar_t *)malloc(sizeof(wchar_t) * buflen);
    if (!newname)
        error(L"bad alloc.\n");
    wcscpy_s(newname, buflen, name);
    d->buffer._string = newname;
    return(d);
}

data make_nil()
{
    data d = alloc();
    d->info |= id_nil;
    return(d);
}

data make_t
()
{
    data d = alloc();
    d->info |= id_t;
    return(d);
}

data quote_impl(data d)
{
    return(car(d));
}

data make_quote()
{
    return(make_builtin_macro(quote_impl));
}

data make_int(int value)
{
    data d = alloc();
    d->info |= id_int;
    d->buffer._int = value;
    return(d);
}

data make_double(double value)
{
    data d = alloc();
    d->info |= id_double;
    d->buffer._double = value;
    return(d);
}

data make_string(const wchar_t * name)
{
    wchar_t * newname;
    data d;
    d = alloc();
    d->info |= id_string;
    int buflen = wcslen(name) + 1;
    newname = (wchar_t *)malloc(sizeof(wchar_t) * buflen);
    if (!newname)
        error(L"bad alloc.\n");
    wcscpy_s(newname, buflen, name);
    d->buffer._string = newname;
    return(d);
}

/**********************/
/* Cons-cell accesser */
/**********************/
data car(data d)
{
    if (!is_pair(d))
        return(nil);
    return(d->buffer.first);
}

data cdr(data d)
{
    if (!is_pair(d))
        return(nil);
    return(d->buffer.rest);
}

data cadr(data d)
{
    return(car(cdr(d)));
}

data caddr(data d)
{
    return(car(cdr(cdr(d))));
}

data cadddr(data d)
{
    return(car(cdr(cdr(cdr(d)))));
}

data cddr(data d)
{
    return(cdr(cdr(d)));
}

data cdddr(data d)
{
    return(cdr(cdr(cdr(d))));
}

data cddddr(data d)
{
    return(cdr(cdr(cdr(cdr(d)))));
}

data caar(data d)
{
    return(car(car(d)));
}

data cdar(data d)
{
    return(cdr(car(d)));
}

/*********************/
/* Raw data accesser */
/*********************/
func_t raw_function(data d)
{
    return(d->buffer.function);
}

func_t raw_macro(data d)
{
    return(d->buffer.function);
}

data get_args(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed.\n");
    return(d->buffer.first);
}

data get_impl(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed.\n");
    return(d->buffer.rest);
}

int raw_int(data d)
{
    return(d->buffer._int);
}

double raw_double(data d)
{
    return(d->buffer._double);
}

const wchar_t * raw_string(data d)
{
    return(d->buffer._string);
}

void set_car(data d, data car)
{
    d->buffer.first = car;
}

void set_cdr(data d, data cdr)
{
    d->buffer.rest = cdr;
}

/**********************/
/* Predicate function */
/**********************/
data _is_pair(data d)
{
    return(nilort(type_id(car(d)) == id_pair));
}

data _is_builtin_macro(data d)
{
    return(nilort(type_id(car(d)) == id_builtin_macro));
}

data _is_builtin_function(data d)
{
    return(nilort(type_id(car(d)) == id_builtin_function));
}

data _is_unnamed_macro(data d)
{
    return(nilort(type_id(car(d)) == id_unnamed_macro));
}

data _is_unnamed_function(data d)
{
    return(nilort(type_id(car(d)) == id_unnamed_function));
}

data _is_symbol(data d)
{
    return(nilort(type_id(car(d)) == id_symbol));
}

data _is_nil(data d)
{
    return(nilort(type_id(car(d)) == id_nil));
}

data _is_not_nil(data d)
{
    return(nilort(type_id(car(d)) != id_nil));
}

data _is_int(data d)
{
    return(nilort(type_id(car(d)) == id_int));
}

data _is_double(data d)
{
    return(nilort(type_id(car(d)) == id_double));
}

data _is_number(data d)
{
    return(nilort((type_id(car(d)) == id_int) || (type_id(car(d)) == id_double)));
}

data _is_string(data d)
{
    return(nilort(type_id(car(d)) == id_string));
}

data _is_zero(data d)
{
    if (is_int(car(d)))
        return(nilort(raw_int(car(d)) == 0));
    else if (is_double(car(d)))
        return(nilort(raw_double(car(d)) == 0.0));
    return(nil);
}

/****************************/
/* Predicate function for C */
/****************************/
int is_pair(data d)
{
    return(type_id(d) == id_pair);
}

int is_builtin_macro(data d)
{
    return(type_id(d) == id_builtin_macro);
}

int is_builtin_function(data d)
{
    return(type_id(d) == id_builtin_function);
}

int is_unnamed_macro(data d)
{
    return(type_id(d) == id_unnamed_macro);
}

int is_unnamed_function(data d)
{
    return(type_id(d) == id_unnamed_function);
}

int is_symbol(data d)
{
    return(type_id(d) == id_symbol);
}

int is_nil(data d)
{
    return(type_id(d) == id_nil);
}

int is_not_nil(data d)
{
    return(type_id(d) != id_nil);
}

int is_t(data d)
{
    return(type_id(d) == id_t);
}

int is_int(data d)
{
    return(type_id(d) == id_int);
}

int is_double(data d)
{
    return(type_id(d) == id_double);
}

int is_number(data d)
{
    return((type_id(d) == id_int) || (type_id(d) == id_double));
}

int is_string(data d)
{
    return(type_id(d) == id_string);
}

int is_zero(data d)
{
    if (is_int(d))
        return(raw_int(d) == 0);
    else if (is_double(d))
        return(raw_double(d) == 0.0);
    return(0);
}

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void link_node(data a, data b)
{
    if (a == b)
        error(L"linknode failed.\n");
    set_cdr(a, b);
    set_car(b, a);
}

data insert_node(data list, data node)
{
    if (list == node)
        error(L"insertnode failed.\n");
    data prev = car(list);
    link_node(prev, node);
    link_node(node, list);
    return(node);
}

data pull_node(data * list)
{
    data d1, d2, pop;
    if ((car(*list) == *list) || (cdr(*list) == *list))
        error(L"pullnode failed.\n");
    d1 = car(*list);
    d2 = cdr(*list);
    link_node(d1, d2);
    pop = *list;
    *list = d2;
    return(pop);
}

/***********/
/* Utility */
/***********/
data nilort(int b)
{
    return(b ? t : nil);
}
