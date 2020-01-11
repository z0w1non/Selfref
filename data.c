#include "data.h"

#include <stdlib.h>
#include "heap.h"
#include "print.h"

enum
{
    id_cons,
    id_builtin_macro,
    id_builtin_function,
    id_unnamed_macro,
    id_unnamed_function,
    id_sym,
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
void initdata(data d)
{
    memset(d, 0, sizeof(dataimpl));
}

int used(data d)
{
    return((d->info & mask_used) != 0);
}

void setused(data d, int used)
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

void setmarked(data d, int marked)
{
    if (marked)
        d->info |= mask_marked;
    else
        d->info &= ~((int)mask_marked);
}

int tid(data d)
{
    return(d->info & 15);
}

const wchar_t * tstr(data d)
{
    static const wchar_t * str[] =
    {
        L"cons",
        L"function",
        L"macro",
        L"symbol",
        L"nil",
        L"t",
        L"quote",
        L"int",
        L"double",
        L"string",
    };
    return(str[tid(d)]);
}

void * getbuf(data d)
{
    return((void **)(&d->buf));
}

/*****************/
/* Make function */
/*****************/
data makecons(data car, data cdr)
{
    data d = alloc();
    d->info |= id_cons;
    ((data *)getbuf(d))[0] = car;
    ((data *)getbuf(d))[1] = cdr;
    return(d);
}

data make_builtin_macro(func_t f)
{
    data d = alloc();
    d->info |= id_builtin_macro;
    ((func_t *)getbuf(d))[0] = f;
    return(d);
}

data make_builtin_function(func_t f)
{
    data d = alloc();
    d->info |= id_builtin_function;
    ((func_t *)getbuf(d))[0] = f;
    return(d);
}

data make_macro(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_macro;
    (((data *)getbuf(d))[0]) = args;
    (((data *)getbuf(d))[1]) = impl;
    return(d);
}

data make_function(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_function;
    (((data *)getbuf(d))[0]) = args;
    (((data *)getbuf(d))[1]) = impl;
    return(d);
}

data make_symbol(const wchar_t * name)
{
    int buflen;
    data d = alloc();
    d->info |= id_sym;
    buflen = wcslen(name) + 1;
    (((wchar_t **)getbuf(d))[0]) = (wchar_t *)malloc(sizeof(wchar_t) * buflen);
    wcscpy_s((((wchar_t **)getbuf(d))[0]), buflen, name);
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
    ((int *)getbuf(d))[0] = value;
    return(d);
}

data make_double(double value)
{
    data d = alloc();
    d->info |= id_double;
    ((double *)getbuf(d))[0] = value;
    return(d);
}

data make_string(const wchar_t * name)
{
    data d = alloc();
    d->info |= id_string;
    int buflen = wcslen(name) + 1;
    (((wchar_t **)getbuf(d))[0]) = (wchar_t *)malloc(sizeof(wchar_t) * buflen);
    wcscpy_s((((wchar_t **)getbuf(d))[0]), buflen, name);
    return(d);
}

/**********************/
/* Cons-cell accesser */
/**********************/
data car(data d)
{
    if (!is_cons(d))
        return(nil);
    return(((data *)getbuf(d))[0]);
}

data cdr(data d)
{
    if (!is_cons(d))
        return(nil);
    return(((data *)getbuf(d))[1]);
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
func_t cfunc(data d)
{
    return(*((func_t *)getbuf(d)));
}

func_t cmacro(data d)
{
    return(*((func_t *)getbuf(d)));
}

data getargs(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed.\n");
    return(((data *)getbuf(d))[0]);
}

data getimpl(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed.\n");
    return(((data *)getbuf(d))[1]);
}

const wchar_t * csym(data d)
{
    return((const wchar_t **)getbuf(d))[0];
}

int cint(data d)
{
    return(*(int *)getbuf(d));
}

double cdbl(data d)
{
    return(*(double *)getbuf(d));
}

const wchar_t * cstr(data d)
{
    return((const wchar_t **)getbuf(d))[0];
}

void csetcar(data d, data car)
{
    ((data *)getbuf(d))[0] = car;
}

void csetcdr(data d, data cdr)
{
    ((data *)getbuf(d))[1] = cdr;
}

/**********************/
/* Predicate function */
/**********************/
data _is_cons(data d)
{
    return(nilort(tid(car(d)) == id_cons));
}

data _is_builtin_macro(data d)
{
    return(nilort(tid(car(d)) == id_builtin_macro));
}

data _is_builtin_function(data d)
{
    return(nilort(tid(car(d)) == id_builtin_function));
}

data _is_unnamed_macro(data d)
{
    return(nilort(tid(car(d)) == id_unnamed_macro));
}

data _is_unnamed_function(data d)
{
    return(nilort(tid(car(d)) == id_unnamed_function));
}

data _is_symbol(data d)
{
    return(nilort(tid(car(d)) == id_sym));
}

data _is_nil(data d)
{
    return(nilort(tid(car(d)) == id_nil));
}

data _is_not_nil(data d)
{
    return(nilort(tid(car(d)) != id_nil));
}

data _is_int(data d)
{
    return(nilort(tid(car(d)) == id_int));
}

data _is_double(data d)
{
    return(nilort(tid(car(d)) == id_double));
}

data _is_number(data d)
{
    return(nilort((tid(car(d)) == id_int) || (tid(car(d)) == id_double)));
}

data _is_string(data d)
{
    return(nilort(tid(car(d)) == id_string));
}

data _is_zero(data d)
{
    if (is_int(car(d)))
        return(nilort(cint(car(d)) == 0));
    else if (is_double(car(d)))
        return(nilort(cdbl(car(d)) == 0.0));
    return(nil);
}

/****************************/
/* Predicate function for C */
/****************************/
int is_cons(data d)
{
    return(tid(d) == id_cons);
}

int is_builtin_macro(data d)
{
    return(tid(d) == id_builtin_macro);
}

int is_builtin_function(data d)
{
    return(tid(d) == id_builtin_function);
}

int is_unnamed_macro(data d)
{
    return(tid(d) == id_unnamed_macro);
}

int is_unnamed_function(data d)
{
    return(tid(d) == id_unnamed_function);
}

int is_symbol(data d)
{
    return(tid(d) == id_sym);
}

int is_nil(data d)
{
    return(tid(d) == id_nil);
}

int is_not_nil(data d)
{
    return(tid(d) != id_nil);
}

int is_t(data d)
{
    return(tid(d) == id_t);
}

int is_int(data d)
{
    return(tid(d) == id_int);
}

int is_double(data d)
{
    return(tid(d) == id_double);
}

int is_number(data d)
{
    return((tid(d) == id_int) || (tid(d) == id_double));
}

int is_string(data d)
{
    return(tid(d) == id_string);
}

int is_zero(data d)
{
    if (is_int(d))
        return(cint(d) == 0);
    else if (is_double(d))
        return(cdbl(d) == 0.0);
    return(0);
}

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void link_node(data a, data b)
{
    if (a == b)
        error(L"linknode failed.\n");
    csetcdr(a, b);
    csetcar(b, a);
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
