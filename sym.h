#ifndef SYM_H
#define SYM_H

#include <wchar.h>
#include "data.h"

/***********************/
/* Symbol map operator */
/***********************/
void initsym();
void freesym();
void pushsym(const wchar_t * key, data value);
void popsym(const wchar_t * key);
data findsym(const wchar_t * key);
void marksym();
data dumpsym(data);

data pushargs(data);
data popargs(data);

#endif
