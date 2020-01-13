#ifndef EVAL_H
#define EVAL_H

#include "data.h"

/********/
/* Eval */
/********/
data eval(data);
data _eval_list(data);
data call_macro(data);
data call_unnamed_macro(data);
data call_unnamed_function(data);

/*****************/
/* Operator list */
/*****************/
void init_operator_list();
data add_builtin_left_associative_operator_function(const wchar_t * name, function_t f);
data add_builtin_right_associative_operator_function(const wchar_t * name, function_t f);
data add_builtin_left_associative_operator_macro(const wchar_t * name, function_t f);
data add_builtin_right_associative_operator_macro(const wchar_t * name, function_t f);
void add_operator(data);
void remove_operator(const wchar_t *);
/* nullable */ data find_operator(const wchar_t *);
int compare_operator_priority(const wchar_t *, const wchar_t *);
data sort_operator(data);

#endif
