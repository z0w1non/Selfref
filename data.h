#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <wchar.h>

#define debug(expr) do { printf("%s(%d) %s: ", __FILE__, __LINE__,  #expr); print(expr); wprintf(L"\n"); } while(0);

struct dataimpl_tag;
typedef struct dataimpl_tag * data;

typedef data(*function_t)(data);

typedef struct dataimpl_tag
{
    int info;
    union
    {
        function_t function;
        int _int;
        double _double;
        struct named
        {
            wchar_t * _string;
            data impl;
        };
        struct pair
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
void mark_data(data d);

/*****************/
/* Make function */
/*****************/
data make_pair(data, data);
data make_builtin_macro(function_t);
data make_builtin_function(function_t);
data make_macro(data, data);
data make_function(data, data);
data make_symbol(const wchar_t *);
data make_nil();
data make_t();
data _quote(data);
data make_quote();
data make_int(int);
data make_double(double);
data make_string(const wchar_t *);
data make_left_associative_operator(const wchar_t *, data);
data make_right_associative_operator(const wchar_t *, data);
data make_prefix_operator(const wchar_t *, data);
data make_suffix_operator(const wchar_t *, data);

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
function_t          raw_function(data);
function_t          raw_macro(data);
data            get_args(data);
data            get_impl(data);
int             raw_int(data);
double          raw_double(data);
const wchar_t * raw_string(data);
void            set_car(data, data);
void            set_cdr(data, data);
data            get_operator_impl(data);

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
data _is_left_associative_operator(data);
data _is_right_associative_operator(data);
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
int is_left_associative_operator(data);
int is_right_associative_operator(data);
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
wchar_t * clone_string(const wchar_t * s);

/*********/
/* Queue */
/*********/
struct queue_tag;
typedef struct queue_tag * queue;
queue queue_create(int);
void queue_cleanup(queue);
int queue_enqueue(queue, const void *);
int queue_dequeue(queue, void *);
int queue_front(queue, void *);
int queue_is_empty(queue);
int queue_print_as_data(stack);

/*********/
/* Stack */
/*********/
struct stack_tag;
typedef struct stack_tag * stack;
stack stack_create(int);
void stack_cleanup(stack);
int stack_push(stack, const void *);
int stack_pop(stack, void *);
int stack_front(stack, void *);
int stack_is_empty(stack);
int stack_print_as_data(stack);

/****************/
/* Forward list */
/****************/
typedef int (*predicate_t) (data d);
data forward_list_create();
data forward_list_push_front(data * list, data element);
data forward_list_pop_front(data * list);
data forward_list_find(data list, predicate_t);
data forward_list_remove(data * list, predicate_t);
data forward_list_mark(data list);

#endif
