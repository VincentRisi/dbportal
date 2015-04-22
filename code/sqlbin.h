#ifndef _SQLBIN_H_
#define _SQLBIN_H_ 

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct SqlToken TSqlToken, *PSqlToken;
typedef struct SqlQuery TSqlQuery, *PSqlQuery;
typedef struct SqlField TSqlField, *PSqlField;
typedef struct SqlBin   TSqlBin,   *PSqlBin;

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
  PSqlField Fields;
  ushort    Size;
  pchar     Command;
  ushort    isSql;
  ushort    isSingle;
  ushort    isManyQuery;
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
  PSqlQuery  Queries;
  ushort     NoServers;
  pchar      *Servers;
  ushort     NoSchemas;
  pchar      *Schemas;
  ushort     NoTables;
  pchar      *Tables;
  short      Error;
  pchar      ErrorMsg;
  ushort     NoTokens;
  PSqlToken  Tokens;
};

int APPLAPI SqlBinOpen(PSqlBin *SqlBin, pchar FileName);
// Reads Binary File into MEMORY and Closes File.
int APPLAPI SqlBinQuery(PSqlBin SqlBin, PSqlQuery *SqlQuery, pchar QueryName);
// Retrieves the query from MEMORY
int APPLAPI SqlBinToken(PSqlBin SqlBin, PSqlToken *SqlToken, pchar TokenName);
// Retrieves the token from MEMORY
int APPLAPI SqlBinClose(PSqlBin SqlBin);
// Frees up the MEMORY.

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

