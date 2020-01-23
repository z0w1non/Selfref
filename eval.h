#ifndef EVAL_H
#define EVAL_H

#include "data.h"

/********/
/* Eval */
/********/
data eval(data);
data _eval_list(data);
data call_builtin_macro(data);
data call_builtin_function(data);
data call_unnamed_macro(data);
data call_unnamed_function(data);

#endif
