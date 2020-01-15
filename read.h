#ifndef READ_H
#define READ_H

#include <wchar.h>
#include <stdio.h>
#include "data.h"

void set_input_stream(FILE * stream);
FILE * get_input_stream();
data read();

#endif
