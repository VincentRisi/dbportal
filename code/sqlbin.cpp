
#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#if !defined(M_AIX) && !defined(M_GNU)
#include <io.h>
#endif
#include "xtalloc.h"
#include "xstring.h"
#include "sqlbin.h"
#include "binio.h"
#include "xdir.h"

enum
{
  SqlBinOK
, SqlBinMem0Err
, SqlBinAccErr
, SqlBinMem1Err
, SqlBinSignErr
, SqlBinMem2Err
, SqlBinMem3Err
, SqlBinMem4Err
, SqlBinMem5Err
, SqlBinMem6Err
, SqlBinNoCB
, SqlBinNoQuery
, SqlBinMem7Err
, SqlBinNoToken
};

static char * Errors[] =
{
  "No error"
, "Not enough memory for control block!"
, "Access for file failed!"
, "Not enough memory to buffer file!"
, "Invalid buffer file signature incorrect!"
, "No memory for Server list!"
, "No memory for Schema list!"
, "No memory for Table list!"
, "No memory for Query list!"
, "No memory for Query field list!"
, "No Control Block Passed!"
, "Query not found!"
, "No memory for Tokens list!"
, "Token not found!"
};

static int SearchComp(const pchar p1, PSqlQuery p2);
static int SearchComp2(const pchar p1, PSqlToken p2);
static TSqlBin ABNSqlBin;

static PSqlBin _SqlBinOpen(pchar InFileName);

int APPLAPI SqlBinOpen(PSqlBin *SqlBin, pchar InFileName)
{
  *SqlBin = _SqlBinOpen(InFileName);
  return (*SqlBin)->Error;
}

static ushort slinkSignOCI   = 0x0C1B;
static ushort slinkSignOCI2  = 0x0C2B;
static ushort slinkSignODBC  = 0x0DBB;

static PSqlBin _SqlBinOpen(pchar aFileName)
{
  ushort i,j, sign;
  int RC=SqlBinOK;
  FILE *BinFile=0;
  char InFileName[DIRMAX_PATH];
  PSqlBin SqlBin;
  int FileSize;

  strncpy(InFileName, aFileName, sizeof(InFileName)-1);
  InFileName[sizeof(InFileName)-1];
  SqlBin = (PSqlBin) malloc(sizeof(TSqlBin));
  if (SqlBin == 0)
  {
    SqlBin = &ABNSqlBin;
    RC = SqlBinMem0Err;
    goto Error;
  }
  memset(SqlBin,0,sizeof(TSqlBin));
  BinFile = fopen(InFileName, "rb");
  if (BinFile == 0)
  {
    free (SqlBin);
    SqlBin = &ABNSqlBin;
    RC = SqlBinAccErr;
    goto Error;
  }
  if (setvbuf(BinFile, 0, _IOFBF, 8192))
  {
    RC = SqlBinMem1Err;
    goto Error;
  }
  FileSize = fseek(BinFile, 0L, SEEK_END);
  fseek(BinFile, 0L, SEEK_SET);
  sign = getUInt16(BinFile);

  if (sign != slinkSignOCI
  &&  sign != slinkSignOCI2
  &&  sign != slinkSignODBC)
  {
    RC = SqlBinSignErr;
    goto Error;
  }
  SqlBin->NoServers = getUInt16(BinFile);
  if((SqlBin->Servers = (char**) calloc(sizeof(pchar), SqlBin->NoServers))==0)
  {
    RC = SqlBinMem2Err;
    goto Error;
  }
  for (i=0; i<SqlBin->NoServers; i++)
    SqlBin->Servers[i] = getString(BinFile);
  SqlBin->NoSchemas = getUInt16(BinFile);
  if((SqlBin->Schemas = (char**) calloc(sizeof(pchar), SqlBin->NoSchemas))==0)
  {
    RC = SqlBinMem3Err;
    goto Error;
  }
  for (i=0; i<SqlBin->NoSchemas; i++)
    SqlBin->Schemas[i] = getString(BinFile);
  SqlBin->NoTables = getUInt16(BinFile);
  if((SqlBin->Tables = (char**) calloc(sizeof(pchar), SqlBin->NoTables))==0)
  {
    RC = SqlBinMem4Err;
    goto Error;
  }
  for (i=0; i<SqlBin->NoTables; i++)
    SqlBin->Tables[i] = getString(BinFile);
  SqlBin->NoQueries = getUInt16(BinFile);
  if((SqlBin->Queries = (PSqlQuery) calloc(sizeof(TSqlQuery), SqlBin->NoQueries))==0)
  {
    RC = SqlBinMem5Err;
    goto Error;
  }
  for (i=0; i<SqlBin->NoQueries; i++)
  {
    PSqlQuery Query      = &SqlBin->Queries[i];
    Query->Name          = getString(BinFile);
    Query->ServerNo      = getUInt16(BinFile);
    Query->SchemaNo      = getUInt16(BinFile);
    Query->isSql         = Query->SchemaNo & 0x8000;
    Query->isSingle       = Query->SchemaNo & 0x4000;
    Query->isManyQuery  = Query->SchemaNo & 0x2000;
    Query->isNullEnabled = Query->SchemaNo & 0x1000;
    Query->SchemaNo     &= 0x0FFF;
    Query->TableNo       = getUInt16(BinFile);
    Query->NoFields      = getUInt16(BinFile);
    if (Query->NoFields > 0)
    {
      if((Query->Fields = (PSqlField) calloc(sizeof(TSqlField), Query->NoFields))==0)
      {
        RC = SqlBinMem6Err;
        goto Error;
      }
      for (j=0; j<Query->NoFields; j++)
      {
        PSqlField Field  = &Query->Fields[j];
        Field->Name      = getString(BinFile);
        Field->CType     = getInt16(BinFile);
        Field->SqlType   = getInt16(BinFile);
        if (sign == slinkSignOCI2)
        {
          Field->Size      = getUInt32(BinFile);
          Field->Offset    = getUInt32(BinFile);
          Field->Precision = getUInt32(BinFile);
          Field->Scale     = getUInt32(BinFile);
        }
        else
        {
          Field->Size      = getUInt16(BinFile);
          Field->Offset    = getUInt16(BinFile);
          Field->Precision = getUInt16(BinFile);
          Field->Scale     = getUInt16(BinFile);
        }
        Field->isBind    = getUInt16(BinFile);
        Field->isDefine  = getUInt16(BinFile);
      }
    }
    Query->Size    = getUInt16(BinFile);
    Query->Command = getExact(Query->Size, BinFile);
  }
  if (ftell(BinFile) < FileSize)
  {
    SqlBin->NoTokens = getUInt16(BinFile);
    if((SqlBin->Tokens = (PSqlToken) calloc(sizeof(TSqlToken), SqlBin->NoTokens))==0)
    {
      RC = SqlBinMem7Err;
      goto Error;
    }
    for (i=0; i<SqlBin->NoTokens; i++)
    {
      PSqlToken Token      = &SqlBin->Tokens[i];
      Token->Name          = getString(BinFile);
      Token->Value         = getString(BinFile);
    }
  }
  goto Return;

Error:
  SqlBinClose(SqlBin);

Return:
  if (BinFile) fclose(BinFile);
  SqlBin->Error = RC;
  SqlBin->ErrorMsg = Errors[RC];
  return SqlBin;
}

typedef int (*fptr)(const void*, const void*);
int APPLAPI SqlBinQuery(PSqlBin SqlBin, PSqlQuery *SqlQuery, pchar FileName)
{
  *SqlQuery = (PSqlQuery) bsearch(FileName, SqlBin->Queries,
               SqlBin->NoQueries, sizeof(TSqlQuery),
               (fptr)SearchComp);
  if (*SqlQuery)
    SqlBin->Error = SqlBinOK;
  else
    SqlBin->Error = SqlBinNoQuery;
  SqlBin->ErrorMsg = Errors[SqlBin->Error];
  return SqlBin->Error;
}

int APPLAPI SqlBinToken(PSqlBin SqlBin, PSqlToken *SqlToken, pchar FileName)
{
  *SqlToken = (PSqlToken) bsearch(FileName, SqlBin->Tokens,
               SqlBin->NoTokens, sizeof(TSqlToken),
               (fptr)SearchComp2);
  if (*SqlToken)
    SqlBin->Error = SqlBinOK;
  else
    SqlBin->Error = SqlBinNoToken;
  SqlBin->ErrorMsg = Errors[SqlBin->Error];
  return SqlBin->Error;
}

int APPLAPI SqlBinClose(PSqlBin SqlBin)
{
  ushort i, j;

  if (SqlBin == &ABNSqlBin)
    return 0;

  if (SqlBin)
  {
    if (SqlBin->NoTokens)
    {
      for (i=0; i<SqlBin->NoTokens; i++)
      {
        PSqlToken Token = &SqlBin->Tokens[i];
        if (Token->Name)
        {
          free(Token->Name);
          Token->Name = 0;
        }
        if (Token->Value)
        {
          free(Token->Value);
          Token->Value = 0;
        }
      }
      free(SqlBin->Tokens);
      SqlBin->Tokens = 0;
    }
    if (SqlBin->NoQueries)
    {
      for (i=0; i<SqlBin->NoQueries; i++)
      {
        PSqlQuery Query = &SqlBin->Queries[i];
        if (Query->Name)
        {
          free(Query->Name);
          Query->Name = 0;
        }
        if (Query->Fields)
        {
          for (j=0; j<Query->NoFields; j++)
          {
            PSqlField Field = &Query->Fields[j];
            if (Field->Name)
            {
              free(Field->Name);
              Field->Name = 0;
            }
          }
          free(Query->Fields);
          Query->Fields = 0;
          Query->NoFields = 0;
        }
        if (Query->Command)
        {
          free(Query->Command);
          Query->Command = 0;
        }
      }
      free(SqlBin->Queries);
      SqlBin->Queries = 0;
    }
    if (SqlBin->Servers)
    {
      for (i=0; i<SqlBin->NoServers; i++)
      {
        if (SqlBin->Servers[i])
        {
          free(SqlBin->Servers[i]);
          SqlBin->Servers[i] = 0;
        }
      }
      free(SqlBin->Servers);
      SqlBin->Servers = 0;
    }
    if (SqlBin->Schemas)
    {
      for (i=0; i<SqlBin->NoSchemas; i++)
      {
        if (SqlBin->Schemas[i])
        {
          free(SqlBin->Schemas[i]);
          SqlBin->Schemas[i] = 0;
        }
      }
      free(SqlBin->Schemas);
      SqlBin->Schemas = 0;
      SqlBin->NoSchemas = 0;
    }
    if (SqlBin->Tables)
    {
      for (i=0; i<SqlBin->NoTables; i++)
      {
        if (SqlBin->Tables[i])
        {
          free(SqlBin->Tables[i]);
          SqlBin->Tables[i] = 0;
        }
      }
      free(SqlBin->Tables);
      SqlBin->Tables = 0;
      SqlBin->NoTables = 0;
    }
    free(SqlBin);
    SqlBin = 0;
  }
  else
    return SqlBinNoCB;
  return SqlBinOK;
}

static int SearchComp(const pchar p1, PSqlQuery p2)
{
  int n = strlen(p1) > strlen(p2->Name) ? strlen(p2->Name) : strlen(p1);
  return strnicmp(p1, p2->Name, n+1);
}

static int SearchComp2(const pchar p1, PSqlToken p2)
{
  int n = strlen(p1) > strlen(p2->Name) ? strlen(p2->Name) : strlen(p1);
  return strnicmp(p1, p2->Name, n+1);
}

