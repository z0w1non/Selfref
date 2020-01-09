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
int isdigit(wint_t);
int iseof(wint_t);
int isspace(wint_t);
int islparen(wint_t);
int isrparen(wint_t);
int isletter(wint_t);
int isdot(wint_t);
int isprint(wint_t);
int iscrlf(wint_t);

#endif
