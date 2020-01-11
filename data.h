#ifndef DATA_H
#define DATA_H

#include <wchar.h>

#define debug(expr) do { wprintf(L"%s: ", L#expr); print(expr); wprintf(L"\n"); } while(0);

struct dataimpl_tag;
typedef struct dataimpl_tag * data;

typedef data(*func_t)(data);

typedef struct dataimpl_tag
{
    int info;
    union
    {
        func_t function;
        int _int;
        double _double;
        wchar_t * _string;
        struct
        {
            data first;
            data rest;
        };
    } buffer;
} dataimpl;

/****************************/
/* Data infomation accesser */
/****************************/
void init_data(data);
int  used(data);
void set_used(data, int);
int  marked(data);
void set_marked(data, int);
int  type_id(data);

/*****************/
/* Make function */
/*****************/
data make_pair(data, data);
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
func_t          raw_function(data);
func_t          raw_macro(data);
data            get_args(data);
data            get_impl(data);
int             raw_int(data);
double          raw_double(data);
const wchar_t * raw_string(data);
void            set_car(data, data);
void            set_cdr(data, data);

/*******************/
/* Constant symbol */
/*******************/
data nil;
data t;
data quote;

/**********************/
/* Predicate function */
/**********************/
data _is_pair(data);
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
data _is_operator(data);
data _is_binary_operator(data);
data _is_prefix_operator(data);
data _is_suffix_operator(data);

/****************************/
/* Predicate function for C */
/****************************/
int is_pair(data);
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
int is_operator(data);
int is_binary_operator(data);
int is_prefix_operator(data);
int is_suffix_operator(data);

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
