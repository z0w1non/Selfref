#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

data inc(data);
data dec(data);
data add(data);
data sub(data);
data mul(data);
data div_(data);
data mod(data);

data if_(data);
data and_(data);
data or_(data);
data not_(data);

/**************/
/* Functional */
/**************/
data call(data);
data bindsym(data);
data lambda(data);
data function(data);

/**********************/
/* Cons-cell function */
/**********************/
data cons(data);
data setcar(data);
data setcdr(data);

/*****************/
/* List function */
/*****************/
data list(data);
data length(data);

/******************/
/* Zip algorithm */
/******************/
data zipfirst(data);
data ziprest(data);
data zip(data);

/*************************/
/* Symbol stack function */
/*************************/
data pushsym(data);
data popsym(data);

/****************************/
/* String integer interface */
/****************************/
data tocharcode(data);
data fromcharcode(data);

#endif
