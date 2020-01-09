#ifndef DATA_H
#define DATA_H

#include <wchar.h>

struct dataimpl_tag;
typedef struct dataimpl_tag * data;

typedef union databuf_tag
{
    void * ptr_[2];
    int int_;
    double dbl_;
    wchar_t * str_[2];
    struct cons_{
        data car_;
        data cdr_;
    };
} databuf;

typedef struct dataimpl_tag
{
    int info;
    databuf buf;
} dataimpl;

typedef data(*func_t)(data);

/****************************/
/* Data infomation accesser */
/****************************/
void initdata(data);
int used(data);
void setused(data, int);
int marked(data);
void setmarked(data, int);
int tid(data);
const wchar_t * tstr(data);
void * getbuf(data);

/*****************/
/* Make function */
/*****************/
data makecons(data, data);
data makefunc(func_t);
data makemacro(func_t);
data makelambda(data, data);
data makesym(const wchar_t *);
data makenil();
data maket();
data makequote();
data makeint(int);
data makedbl(double);
data makestr(const wchar_t *);

/**********************/
/* Cons-cell accesser */
/**********************/
data car(data);
data cdr(data);
data cadr(data);
data caddr(data);
data cadddr(data);
data cddr(data);
data cdddr(data);
data cddddr(data);
data caar(data);
data cdar(data);

/*********************/
/* Raw data accesser */
/*********************/
func_t          cfunc(data);
func_t          cmacro(data);
data getargs(data);
data getimpl(data);
const wchar_t * csym(data);
int             cint(data);
double          cdbl(data);
const wchar_t * cstr(data);
void csetcar(data, data);
void csetcdr(data, data);

/*******************/
/* Constant symbol */
/*******************/
data nil;
data t;
data quote;

/**********************/
/* Predicate function */
/**********************/
data consp(data);
data funcp(data);
data macrop(data);
data lambdap(data);
data symp(data);
data nilp(data);
data nnilp(data);
data intp(data);
data dblp(data);
data nump(data);
data strp(data);
data ufuncp(data);
data zerop(data);

/****************************/
/* Predicate function for C */
/****************************/
int cconsp(data);
int cfuncp(data);
int cmacrop(data);
int clambdap(data);
int csymp(data);
int cnilp(data);
int ctp(data);
int cnnilp(data);
int cintp(data);
int cdblp(data);
int cnump(data);
int cstrp(data);
int cufuncp(data);
int czerop(data);

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void linknode(data a, data b);
data insertnode(data list, data node);
data pullnode(data * list);

/***********/
/* Utility */
/***********/
data nilort(int);

#endif
