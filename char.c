#include "char.h"

#include "wctype.h"

int chartype(wint_t wi)
{
    if (isdigit(wi))
        return(wctype_digit);
    if (isletter(wi))
        return(wctype_letter);
    if (islparen(wi))
        return(wctype_lparen);
    if (isrparen(wi))
        return(wctype_rparen);
    if (isdot(wi))
        return(wctype_dot);
    if (isspace(wi))
        return(wctype_space);
    return(wctype_other);
}

int isdigit(wint_t wi)
{
    return((wi >= L'0') && (wi <= L'9'));
}

int iseof(wint_t wi)
{
    return(wi == WEOF);
}

int isspace(wint_t wi)
{
    return((wi == L' ') || (wi == L'\t'));
}

int islparen(wint_t wi)
{
    return(wi == L'(');
}

int isrparen(wint_t wi)
{
    return(wi == L')');
}

int isletter(wint_t wi)
{
    return(((wi >= L'a') && (wi <= L'z')) || ((wi >= L'A') && (wi <= L'Z')) || (wi == L'_'));
}

int isdot(wint_t wi)
{
    return(wi == L'.');
}

int isprint(wint_t wi)
{
    return(iswprint(wi));
}

int iscrlf(wint_t wi)
{
    return(wi == L'\n' || wi == L'\r');
}
