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
data make_builtin_macro(func_t);
data make_builtin_function(func_t);
data make_macro(data, data);
data make_function(data, data);
data make_symbol(const wchar_t *);
data make_nil();
data make_t();
data make_quote();
data make_int(int);
data make_double(double);
data make_string(const wchar_t *);

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
data _is_cons(data);
data _is_builtin_macro(data);
data _is_builtin_function(data);
data _is_unnamed_macro(data);
data _is_unnamed_function(data);
data _is_symbol(data);
data _is_nil(data);
data _is_not_nil(data);
data _is_int(data);
data _is_double(data);
data _is_number(data);
data _is_string(data);
data _is_zero(data);

/****************************/
/* Predicate function for C */
/****************************/
int is_cons(data);
int is_builtin_macro(data);
int is_builtin_function(data);
int is_unnamed_function(data);
int is_unnamed_macro(data);
int is_symbol(data);
int is_nil(data);
int is_not_nil(data);
int is_t(data);
int is_int(data);
int is_double(data);
int is_number(data);
int is_string(data);
int is_zero(data);

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void link_node(data a, data b);
data insert_node(data list, data node);
data pull_node(data * list);

/***********/
/* Utility */
/***********/
data nilort(int);

#endif
