#include "string.h"

#include "wctype.h"

int chartype(wint_t wi)
{
    if (is_digit(wi))
        return(wctype_digit);
    if (is_symbol_char(wi))
        return(wctype_letter);
    if (is_lparen(wi))
        return(wctype_lparen);
    if (is_rparen(wi))
        return(wctype_rparen);
    if (is_dot(wi))
        return(wctype_dot);
    if (is_space(wi))
        return(wctype_space);
    return(wctype_other);
}

int is_digit(wint_t wi)
{
    return((wi >= L'0') && (wi <= L'9'));
}

int is_eof(wint_t wi)
{
    return(wi == WEOF);
}

int is_space(wint_t wi)
{
    return((wi == L' ') || (wi == L'\t'));
}

int is_lparen(wint_t wi)
{
    return(wi == L'(');
}

int is_rparen(wint_t wi)
{
    return(wi == L')');
}

int is_symbol_char(wint_t wi)
{
    return(
        ((wi >= L'a') && (wi <= L'z'))
     || ((wi >= L'A') && (wi <= L'Z'))
     || (wi == L'_')
    );
}

int is_operator_char(wint_t wi)
{
    return((wi == L'.')
        || (wi == L',')
        || (wi == L':')
        || (wi == L';')
        || (wi == L'+')
        || (wi == L'-')
        || (wi == L'*')
        || (wi == L'/')
        || (wi == L'%')
        || (wi == L'=')
        || (wi == L'<')
        || (wi == L'>')
        || (wi == L'!')
        || (wi == L'?')
        || (wi == L'\'')
        || (wi == L'&')
        || (wi == L'|')
        || (wi == L'~')
        || (wi == L'^')
        || (wi == L'#')
        || (wi == L'$')
        || (wi == L'@')
    );
}

int is_dot(wint_t wi)
{
    return(wi == L'.');
}

int is_print(wint_t wi)
{
    return(iswprint(wi));
}

int is_crlf(wint_t wi)
{
    return(wi == L'\n' || wi == L'\r');
}

int is_semicolon(wint_t wi)
{
    return(wi == L';');
}

int is_comma(wint_t wi)
{
    return(wi == L',');
}

/**********/
/* String */
/**********/
void string_init(string * str)
{
    str->buffer[0] = L'0';
    str->heap = NULL;
    str->length = 0;
}

void string_cleanup(string * str)
{
    free(str->heap);
}

void string_copy_buffer_to_heap(string * str)
{
    str->heap_size = string_default_buffer_size * 2;
    str->heap = malloc(str->heap_size * sizeof(wchar_t));
    if (!str->heap)
    {
        error(L"bad alloc");
        return;
    }
    wcscpy_s(str->heap, str->heap_size, str->buffer);
    free(str->heap);
    str->buffer[0] = L'\0';
}

void string_reallocate_heap(string * str)
{
    wchar_t * buffer;
    str->heap_size * 2;
    buffer = malloc(str->heap_size * sizeof(wchar_t));
    if (!buffer)
    {
        error(L"bad alloc");
        return;
    }
    wcscpy_s(buffer, str->heap_size, str->heap);
    free(str->heap);
    str->heap = buffer;
}

void string_append_char(string * str, wchar_t wc)
{
    if (str->heap)
    {
        if (str->length + 2 >= str->heap_size)
            string_reallocate_heap(str);
        str->heap[str->length++] = wc;
        str->heap[str->length] = L'\0';
    }
    else
    {
        if (str->length + 2 >= string_default_buffer_size)
        {
            string_copy_buffer_to_heap(str);
            if (!str->heap)
            {
                error(L"bad arroc");
                return;
            }
            str->heap[str->length++] = wc;
            str->heap[str->length] = L'\0';
        }
        else
        {
            str->buffer[str->length++] = wc;
            str->buffer[str->length] = L'\0';
        }
    }
}

void string_append_string(string * str, const wchar_t * source)
{
    size_t source_length;
    source_length = wcslen(source);
    if (str->heap)
    {
        while (str->length + source_length + 1 >= str->heap_size)
            string_reallocate_heap(str);
        wcscpy_s(str->heap + str->length, str->heap_size - str->length, source);
    }
    else
    {
        if (str->length + source_length + 1 >= string_default_buffer_size)
        {
            string_copy_buffer_to_heap(str);
            if (!str->heap)
            {
                error(L"bad arroc");
                return;
            }
            wcscpy_s(str->heap + str->length, str->heap_size - str->length, source);
        }
        else
            wcscpy_s(str->buffer + str->length, string_default_buffer_size - str->length, source);
    }
    str->length += source_length;
}

void string_clear(string * str)
{
    str->length = 0;
    str->buffer[0] = L'\0';
    if (str->heap)
        str->heap[0] = L'\0';
}

const wchar_t * string_raw_string(string * str)
{
    if (str->heap)
        return(str->heap);
    return(str->buffer);
}

size_t string_length(string * str)
{
    return(str->length);
}
