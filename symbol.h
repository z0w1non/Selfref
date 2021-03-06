#ifndef SYM_H
#define SYM_H

#include <wchar.h>
#include "data.h"

/***********************/
/* Symbol map operator */
/***********************/
void init_symbol_stack();
void cleanup_symbol_stack();
void push_symbol(const wchar_t * key, data value);
void cpop_symbol(const wchar_t * key);
data override_symbol(const wchar_t * key, data value);
data find_symbol(const wchar_t * key);
void mark_symbol();
data _dump_symbol(data);

data _push_args(data);
data _pop_args(data);

#endif
