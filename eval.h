#ifndef EVAL_H
#define EVAL_H

#include "data.h"

/********/
/* Eval */
/********/
data eval(data);
data _eval_list(data);
data call_macro(data);
data call_function(data);

#endif
