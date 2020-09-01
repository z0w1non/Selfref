#ifndef UNICODE_H
#define UNICODE_H

#include <wchar.h>

// Call free() specify the value retuened from this function.
char * wstring_to_string(const wchar_t * wcs);

#endif
