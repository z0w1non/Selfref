#ifndef EVAL_H
#define EVAL_H

#include "data.h"

/********/
/* Eval */
/********/
data eval(data);
data evallist(data);
data expandmacro(data);
data calllambda(data);

#endif
