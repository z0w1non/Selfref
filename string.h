#ifndef CHAR_H
#define CHAR_H

#include <wchar.h>

enum
{
    wctype_other,
    wctype_space,
    wctype_lparen,
    wctype_rparen,
    wctype_letter,
    wctype_digit,
    wctype_dot,
};

int chartype(wint_t);
int is_digit(wint_t);
int is_eof(wint_t);
int is_space(wint_t);
int is_lparen(wint_t);
int is_rparen(wint_t);
int is_symbol_char(wint_t);
int is_operator_char(wint_t);
int is_dot(wint_t);
int is_print(wint_t);
int is_crlf(wint_t);
int is_semicolon(wint_t);
int is_comma(wint_t);

/**********/
/* String */
/**********/
enum
{
    string_default_buffer_size = 32
};

typedef struct string_tag
{
    wchar_t buffer[string_default_buffer_size];
    wchar_t * heap;
    size_t heap_size;
    size_t length;
} string;

void string_init(string * str);
void string_cleanup(string * str);
void string_copy_buffer_to_heap(string * str);
void string_reallocate_heap(string * str);
void string_append_char(string * str, wchar_t wc);
void string_append_string(string * str, const wchar_t * source);
void string_clear(string * str);
const wchar_t * string_raw_string(string * str);
size_t string_length(string * str);

#endif
