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
int is_dot(wint_t);
int is_print(wint_t);
int is_crlf(wint_t);
int is_semicolon(wint_t);
int is_comma(wint_t);

#endif
