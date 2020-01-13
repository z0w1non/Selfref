#ifndef SYM_H
#define SYM_H

#include <wchar.h>
#include "data.h"

/***********************/
/* Symbol map operator */
/***********************/
void init_symbol_stack();
void cleanup_symbol_stack();
void cpush_symbol(const wchar_t * key, data value);
void cpop_symbol(const wchar_t * key);
data find_symbol(const wchar_t * key);
data replace_symbol(const wchar_t * key, data value);
void mark_symbol();
data _dump_symbol(data);

data _push_args(data);
data _pop_args(data);

#endif
