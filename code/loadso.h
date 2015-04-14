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

typedef pSqlQuery PSqlQuery;
typedef pSqlField PSqlField;
typedef pSqlToken PSqlToken;

struct SqlSO
{
  int         noQueries;
  int         noTokens;
  int         noServers;
  int         noSchemas;
  int         noTables;
  PSqlQuery  *queries;
  PSqlToken  *tokens;
  pchar      *servers;
  pchar      *schemas;
  pchar      *tables;
  SqlSO()
  {
    noQueries = 0;
    noTokens = 0;
    noServers = 0;
    noSchemas = 0;
    noTables = 0;
    queries = 0;
    tokens = 0;
    servers = 0;
    schemas = 0;
    tables = 0;
  }
  ~SqlSO()
  {
    noQueries = 0;
    noTokens = 0;
    noServers = 0;
    noSchemas = 0;
    noTables = 0;
    realloc(queries, 0);
    realloc(tokens, 0);
    realloc(servers, 0);
    realloc(schemas, 0);
    realloc(tables, 0);
  }
  int addServer(char *name);
  int addSchema(char *name);
  int addTable(char *name);
  int loadInFile(const  char *name);
  static char* dehash(char* work, int worklen, const char *name);
  static char* nameExt(char* work, int worklen, const char *path);
  static char* makeOutName(char* work, int worklen
                         , const char* inFileName
                         , char* outExt
                         , char* outDir);

};

#endif