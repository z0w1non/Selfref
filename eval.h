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
void add_operator(data);
void remove_operator(const wchar_t *);
data find_operator(const wchar_t *);
int compare_operator_priority(const wchar_t *, const wchar_t *);
data sort_operator(data);

#endif
