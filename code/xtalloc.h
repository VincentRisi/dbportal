#ifndef _XTALLOC_H_
#define _XTALLOC_H_ 

#include <malloc.h>
#include <string.h>

/*#define XTALLOC*/

#ifdef XTALLOC
#define XTALLOC_EXTENDED_DEFINED 1
#ifndef _XTALLOC_NOMACS_
#define malloc(a)    extended_malloc((a),       __FILE__, __LINE__)
#define calloc(a,b)  extended_calloc((a), (b),  __FILE__, __LINE__)
#define realloc(a,b) extended_realloc((a),(b),  __FILE__, __LINE__)
#define strdup(a)    extended_strdup((a),       __FILE__, __LINE__)
#define free(a)      extended_free((a),         __FILE__, __LINE__)
#ifndef __WATCOMC__
#define tempnam(a,b) extended_tempnam((a),(b),  __FILE__, __LINE__)
#endif
#endif
#else
#define XTALLOC_EXTENDED_DEFINED 1
#endif /*XTALLOC*/

extern int record_xtalloc; /* default = 0 (must set to 1 if required) */

void* extended_malloc (size_t size, char *file, int lineno);
void* extended_calloc (size_t nitems, size_t size, char *file, int lineno);
void* extended_realloc(void *block, size_t size, char *file, int lineno);
char* extended_strdup (const char *s, char *file, int lineno);
void  extended_free   (void *block, char *file, int lineno);
#ifndef __WATCOMC__
char* extended_tempnam(char *dir, char *prefix, char *file, int lineno);
#endif

#endif /*_XTALLOC_H_*/

