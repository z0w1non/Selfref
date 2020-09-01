#include "unicode.h"

#pragma warning(disable:4996)

#include <stdlib.h>

char * wstring_to_string(const wchar_t * wcs)
{
    size_t length, i;
    char * buffer = NULL;
    length = wcslen(wcs);

    buffer = (char *)malloc(sizeof(char) * (length + 1));
    if (!buffer)
        return NULL;

    for (i = 0; i < length; ++i)
    {
        if (wctomb(&buffer[i], wcs[i]) == -1)
        {
            //ERROR
        }
    }
    buffer[length] = '\0';

    return buffer;
}
