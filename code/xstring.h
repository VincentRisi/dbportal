#ifndef _XSTRING_H_
#define _XSTRING_H_

#include <string.h>

#define stricmp(a, b)      tolower_stricmp((a), (b))
#define strnicmp(a, b, c)  tolower_strnicmp((a), (b), (c))

/*  macros to test equality of 2 strings */
/*  with/without bounded length or case sensitivity */
#define streq(s, t)        (strcmp((s), (t)) == 0)
#define strieq(s, t)       (stricmp((s), (t)) == 0)
#define strneq(s, t, n)    (strncmp((s), (t), (n)) == 0)
#define strnieq(s, t, n)   (strnicmp((s), (t), (n)) == 0)

/*  remove leading and trailing spaces and tabs */
#define strtrim(s)         (strltrim(strrtrim(s)))

/*  case insensitive strcmp */
int tolower_stricmp(const char *a, const char *b);

/*  case insensitive bounded length strcmp */
int tolower_strnicmp(const char *a, const char *b, const size_t n);

#if defined(M_AIX) || defined(M_GNU)
/*  convert string to lower case */
char *strlwr(char *s);
/*  convert string to upper case */
char *strupr(char *s);
#elif defined(M_W32)
#define strlwr _strlwr
#define strupr _strupr
#endif

/*  remove leading spaces and tabs */
char *strltrim(char *s);

/*  remove trailing spaces and tabs */
char *strrtrim(char *s);

/*  pad string in field of length fldlen (the data area length is fldlen + 1) */
/*  eg "abc"   --->  "       abc" */
char *strlpad(char *s, const int fldlen, const char padchar);

/*  pad string in field of length fldlen (the data area length is fldlen + 1) */
/*  eg "abc"   --->  "   abc    " */
char *strcpad(char *s, const int fldlen, const char padchar);

/*  pad string in field of length fldlen (the data area length is fldlen + 1) */
/*  eg "abc"   --->  "abc       " */
char *strrpad(char *s, const int fldlen, const char padchar);

/*  copies src string to dest for at most n bytes. */
/*  dest is null-terminated in all cases (unlike strncpy) */
/*  example usage: strncpyz(dest, src, sizeof(dest) - 1); */
/*  after the function call strlen(dest) = min(n, strlen(src)) */
char *strncpyz(char *dest, const char *src, const size_t n);

/*  allocate an array - the same length of s, and copy s into it */
/*  Self exists in the aix library but is not in the header file */
/*  unless one uses the cc (extended compiler) - Self is not feasible */
/*  since cc does not expand macros correctly (in particular the */
/*  stringizing operator used in the assert macro) */
/* char* strdup(const char* s); */

#endif
