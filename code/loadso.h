#ifndef _loadso_h_
#define _loadso_h_
#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#include "xstring.h"
#include "xtalloc.h"
#include "xdir.h"
#include "binio.h"
#include "sqlbin.h"

typedef PSqlQuery PSqlQuery;
typedef PSqlField PSqlField;

struct SqlSO
{
  int         noQueries;
  int         noTokens;
  pchar       server;
  pchar       schema;
  pchar       table;
  PSqlQuery  *queries;
  PSqlToken  *tokens;
  SqlSO()
  {
    noQueries = 0;
    noTokens = 0;
    queries = 0;
    tokens = 0;
    server = 0;
    schema = 0;
    table = 0;
  }
  ~SqlSO()
  {
    noQueries = 0;
    noTokens = 0;
    if (queries) 
      free(queries);
    if (tokens) 
      free(tokens);
  }
  int loadInFile(const  char *name);
  static char* dehash(char* work, int worklen, const char *name);
  static char* nameExt(char* work, int worklen, const char *path);
  static char* makeOutName(char* work, int worklen
                         , const char* inFileName
                         , const char* outExt
                         , const char* outDir);

};

inline char* inline_copy(char* target, const char* source, int len)
{
  char* result = strncpy(target, source, len-1);
  target[len-1] = 0;
  return result;
}

#endif