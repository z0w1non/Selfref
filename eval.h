#ifndef EVAL_H
#define EVAL_H

#include "data.h"

/********/
/* Eval */
/********/
data eval(data);
data eval_list(data);
data expand_macro(data);
data call_function(data);

#endif
