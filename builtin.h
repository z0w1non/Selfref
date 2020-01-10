#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

void init_builtin();

data inc(data);
data dec(data);
data _add(data);
data _sub(data);
data _mul(data);
data _div(data);
data _mod(data);

data _if(data);
data _and(data);
data _or(data);
data _not(data);

/**************/
/* Functional */
/**************/
data _call(data);
data _bindsym(data);
data _unnamed_macro(data);
data _unnamed_function(data);
data _macro(data);
data _function(data);

/**********************/
/* Cons-cell function */
/**********************/
data _cons(data);
data _setcar(data);
data _setcdr(data);

/*****************/
/* List function */
/*****************/
data _list(data);
data _length(data);

/******************/
/* Zip algorithm */
/******************/
data _zipfirst(data);
data _ziprest(data);
data _zip(data);

/*************************/
/* Symbol stack function */
/*************************/
data _pushsym(data);
data _popsym(data);

/****************************/
/* String integer interface */
/****************************/
data _to_char_code(data);
data _from_char_code(data);

#endif
