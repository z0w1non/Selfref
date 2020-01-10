#ifndef PRINT_H
#define PRINT_H

#include <wchar.h>
#include <stdarg.h>
#include "data.h"

data print(data);
data print_list(data);
void error(const wchar_t * format, ...);

#endif
