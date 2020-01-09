#ifndef PRINT_H
#define PRINT_H

#include <wchar.h>
#include <stdarg.h>
#include "data.h"

data print(data);
data printlist(data);
void error(const wchar_t * format, ...);

#endif
