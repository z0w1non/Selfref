#include "fileio.h"

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "unicode.h"
#include "print.h"

data _read_file(data d)
{
    FILE * file = NULL;
    char * file_name = NULL;
    fpos_t pos;
    size_t size;
    wchar_t * buffer = NULL;
    wchar_t * ptr;
    wint_t wc;
    data result = nil;
    wchar_t * error_message = NULL;

    if (is_nil(car(d)))
    {
        error_message = L"read file failed";
        goto cleanup;
    }
    if (!is_string(car(d)))
    {
        error_message = L"The first argment of read_file must be string";
        goto cleanup;
    }

    file_name = wstring_to_string(raw_string(car(d)));
    if (!file_name)
        goto cleanup;

    file = fopen(file_name, "r");
    if (!file)
    {
        error_message = L"failed to open file at read_file";
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_END))
    {
        error_message = L"failed to acquire file size at read_file";
        goto cleanup;
    }

    if (fgetpos(file, &pos))
    {
        error_message = L"failed to acquire file size at read_file";
        goto cleanup;
    }
    size = (size_t)pos;

    if (fseek(file, 0, SEEK_SET))
    {
        error_message = L"failed to seek at read_file";
        goto cleanup;
    }

    buffer = (wchar_t *)malloc((size + 1) * sizeof(wchar_t));
    if (!buffer)
    {
        error_message = L"heap allocation failed at file_read";
        goto cleanup;
    }

    ptr = buffer;
    while (1)
    {
        wc = fgetwc(file);
        if (wc == WEOF)
            break;
        *ptr = wc;
        ptr += 1;
    }
    *ptr = L'\0';

    result = make_string(buffer);

cleanup:
    if (file_name)
        free(file_name);
    if (file)
        fclose(file);
    if (buffer)
        free(buffer);
    if (error_message)
        error(error_message);

    return result;
}

data _write_file(data d)
{
    FILE * file = NULL;
    char * file_name = NULL;
    wchar_t * error_message = NULL;

    if (!is_string(car(d)))
    {
        error_message = L"The first argment of write_file must be string";
        goto cleanup;
    }

    if (!is_string(cadr(d)))
    {
        error_message = L"The second argument of write_file must be string";
        goto cleanup;
    }

    file_name = wstring_to_string(raw_string(car(d)));
    if (!file_name)
        goto cleanup;

    file = fopen(file_name, "w");
    if (!file)
    {
        error_message = L"failed to open file at write_file";
        goto cleanup;
    }

    if (fputws(raw_string(cadr(d)), file) == EOF)
    {
        error_message = L"failed to write string to file write_file";
        goto cleanup;
    }

cleanup:
    if (file_name)
        free(file_name);
    if (file)
        fclose(file);
    if (error_message)
        error(error_message);
}
