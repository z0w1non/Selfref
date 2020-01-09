#include "data.h"

#include <stdlib.h>
#include "heap.h"
#include "print.h"

enum
{
    id_cons = 0,
    id_func = 1,
    id_macro = 2,
    id_sym = 3,
    id_nil = 4,
    id_t = 5,
    id_lambda = 6,
    id_int = 7,
    id_dbl = 8,
    id_str = 9,
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

data makefunc(func_t f)
{
    data d = alloc();
    d->info |= id_func;
    ((func_t *)getbuf(d))[0] = f;
    return(d);
}

data makemacro(func_t f)
{
    data d = alloc();
    d->info |= id_macro;
    ((func_t *)getbuf(d))[0] = f;
    return(d);
}

data makelambda(data args, data impl)
{
    data d = alloc();
    d->info |= id_lambda;
    (((data *)getbuf(d))[0]) = args;
    (((data *)getbuf(d))[1]) = impl;
    return(d);
}

data makesym(const wchar_t * name)
{
    int buflen;
    data d = alloc();
    d->info |= id_sym;
    buflen = wcslen(name) + 1;
    (((wchar_t **)getbuf(d))[0]) = (wchar_t *)malloc(sizeof(wchar_t) * buflen);
    wcscpy_s((((wchar_t **)getbuf(d))[0]), buflen, name);
    return(d);
}

data makenil()
{
    data d = alloc();
    d->info |= id_nil;
    return(d);
}

data maket()
{
    data d = alloc();
    d->info |= id_t;
    return(d);
}

data quoteimpl(data d)
{
    return(d);
}

data makequote()
{
    return(makemacro(quoteimpl));
}

data makeint(int value)
{
    data d = alloc();
    d->info |= id_int;
    ((int *)getbuf(d))[0] = value;
    return(d);
}

data makedbl(double value)
{
    data d = alloc();
    d->info |= id_dbl;
    ((double *)getbuf(d))[0] = value;
    return(d);
}

data makestr(const wchar_t * name)
{
    data d = alloc();
    d->info |= id_str;
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
    if (!cconsp(d))
        return(nil);
    return(((data *)getbuf(d))[0]);
}

data cdr(data d)
{
    if (!cconsp(d))
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
    if (!clambdap(d))
        error(L"getargs failed.\n");
    return(((data *)getbuf(d))[0]);
}

data getimpl(data d)
{
    if (!clambdap(d))
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
data consp(data d)
{
    return(nilort(tid(car(d)) == id_cons));
}

data funcp(data d)
{
    return(nilort(tid(car(d)) == id_func));
}

data macrop(data d)
{
    return(nilort(tid(car(d)) == id_macro));
}

data lambdap(data d)
{
    return(nilort(tid(car(d)) == id_lambda));
}

data symp(data d)
{
    return(nilort(tid(car(d)) == id_sym));
}

data nilp(data d)
{
    return(nilort(tid(car(d)) == id_nil));
}

data nnilp(data d)
{
    return(nilort(tid(car(d)) != id_nil));
}

data intp(data d)
{
    return(nilort(tid(car(d)) == id_int));
}

data dblp(data d)
{
    return(nilort(tid(car(d)) == id_dbl));
}

data nump(data d)
{
    return(nilort((tid(car(d)) == id_int) || (tid(car(d)) == id_dbl)));
}

data strp(data d)
{
    return(nilort(tid(car(d)) == id_str));
}

data zerop(data d)
{
    if (cintp(car(d)))
        return(nilort(cint(car(d)) == 0));
    else if (cdblp(car(d)))
        return(nilort(cdbl(car(d)) == 0.0));
    return(nil);
}

/****************************/
/* Predicate function for C */
/****************************/
int cconsp(data d)
{
    return(tid(d) == id_cons);
}

int cfuncp(data d)
{
    return(tid(d) == id_func);
}

int cmacrop(data d)
{
    return(tid(d) == id_macro);
}

int clambdap(data d)
{
    return(tid(d) == id_lambda);
}

int csymp(data d)
{
    return(tid(d) == id_sym);
}

int cnilp(data d)
{
    return(tid(d) == id_nil);
}

int ctp(data d)
{
    return(tid(d) == id_t);
}

int cnnilp(data d)
{
    return(tid(d) != id_nil);
}

int cintp(data d)
{
    return(tid(d) == id_int);
}

int cdblp(data d)
{
    return(tid(d) == id_dbl);
}

int cnump(data d)
{
    return((tid(d) == id_int) || (tid(d) == id_dbl));
}

int cstrp(data d)
{
    return(tid(d) == id_str);
}

int czerop(data d)
{
    if (cintp(d))
        return(cint(d) == 0);
    else if (cdblp(d))
        return(cdbl(d) == 0.0);
    return(0);
}

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void linknode(data a, data b)
{
    if (a == b)
        error(L"linknode failed.\n");
    csetcdr(a, b);
    csetcar(b, a);
}

data insertnode(data list, data node)
{
    if (list == node)
        error(L"insertnode failed.\n");
    data prev = car(list);
    linknode(prev, node);
    linknode(node, list);
    return(node);
}

data pullnode(data * list)
{
    data d1, d2, pop;
    if ((car(*list) == *list) || (cdr(*list) == *list))
        error(L"pullnode failed.\n");
    d1 = car(*list);
    d2 = cdr(*list);
    linknode(d1, d2);
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
