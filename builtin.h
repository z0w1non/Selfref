#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

void init_builtin();

/**************/
/* Arithmetic */
/**************/
data _inc(data);
data _dec(data);

/*********************************/
/* Variacdic arithmetic function */
/*********************************/
data _add(data);
data _sub(data);
data _mul(data);
data _div(data);
data _mod(data);

/*******************/
/* Binary operator */
/*******************/
data _add_2op(data);
data _sub_2op(data);
data _mul_2op(data);
data _div_2op(data);
data _mod_2op(data);

/**************/
/* Comparator */
/**************/
data _less_2op(data);
data _less_equal_2op(data);
data _greater_2op(data);
data _greater_equal_2op(data);
data _equal_2op(data);
data _not_equal_2op(data);

data _assign(data);

/***************/
/* Conditional */
/***************/
data _if(data);
data _and(data);
data _or(data);
data _not(data);

/**************/
/* Functional */
/**************/
data _eval(data);
data _print(data);
data _call(data);
data _apply(data);
data _bind_symbol(data);
data _unnamed_macro(data);
data _unnamed_function(data);
data _macro(data);
data _function(data);

/************************/
/* Operator declaration */
/************************/
data _left_associative_operator(data);
data _right_associative_operator(data);

/**********************/
/* Cons-cell function */
/**********************/
data _pair(data);
data _set_first(data);
data _set_rest(data);

/*****************/
/* List function */
/*****************/
data _list(data);
data _length(data);

/******************/
/* Zip algorithm */
/******************/
data _zip_first(data);
data _zip_rest(data);
data _zip(data);

/*************************/
/* Symbol stack function */
/*************************/
data _push_symbol(data);
data _pop_symbol(data);

/****************************/
/* String integer interface */
/****************************/
data _to_char_code(data);
data _from_char_code(data);

data _progn(data);
data _let(data);
data _mapcar(data);
data _strcat(data);

#endif
