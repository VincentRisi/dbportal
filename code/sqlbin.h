#ifndef _SQLBIN_H_
#define _SQLBIN_H_ 

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct SqlToken tSqlToken, *pSqlToken;
typedef struct SqlQuery tSqlQuery, *pSqlQuery;
typedef struct SqlField tSqlField, *pSqlField;
typedef struct SqlBin    tSqlBin,  *pSqlBin;

enum { fieldIsPrimaryKey   = 0x8000
     , fieldIsNull         = 0x4000
     , fieldIsInput        = 0x2000
     , fieldIsBound        = 0x1000
     , fieldIsOutput       = 0x0800
     , fieldIsWork         = 0x0400
     , fieldIsRowSequence  = 0x0200
     , fieldIsSequence     = 0x0100
     , fieldIsNullable     = 0x0010
     };

struct SqlField
{
  pchar  Name;
  short  CType;
  short  SqlType;
  uint32 Size;
  uint32 Offset;
  uint32 Precision;
  uint32 Scale;
  ushort isBind;
  ushort isDefine;
};

struct SqlQuery
{
  pchar     Name;
  ushort    ServerNo;
  ushort    SchemaNo;
  ushort    TableNo;
  ushort    NoFields;
  pSqlField Fields;
  ushort    Size;
  pchar     Command;
  ushort    isSql;
  ushort    isFetch;
  ushort    isMultiFetch;
  ushort    isNullEnabled;
};

struct SqlToken
{
  pchar     Name;
  pchar     Value;
};

struct SqlBin
{
  ushort     NoQueries;
  pSqlQuery  Queries;
  ushort     NoServers;
  pchar      *Servers;
  ushort     NoSchemas;
  pchar      *Schemas;
  ushort     NoTables;
  pchar      *Tables;
  short      Error;
  pchar      ErrorMsg;
  ushort     NoTokens;
  pSqlToken  Tokens;
};

int APPLAPI SqlBinOpen(pSqlBin *SqlBin, pchar FileName);
// Reads Binary File into MEMORY and Closes File.
int APPLAPI SqlBinQuery(pSqlBin SqlBin, pSqlQuery *SqlQuery, pchar QueryName);
// Retrieves the query from MEMORY
int APPLAPI SqlBinToken(pSqlBin SqlBin, pSqlToken *SqlToken, pchar TokenName);
// Retrieves the token from MEMORY
int APPLAPI SqlBinClose(pSqlBin SqlBin);
// Frees up the MEMORY.

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

