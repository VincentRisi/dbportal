#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#include "xstring.h"
#include "xtalloc.h"
#include "xdir.h"
#include "binio.h"
#include "slink.h"

#define AddList(Pointer, Type, Data, Index, Delta) do {\
  if (Index % Delta == 0) \
    Pointer = (Type*)realloc(Pointer, sizeof(Data)*(Index + Delta)); \
  Pointer[Index++] = Data; \
  } while(0)

ushort     NoQueries = 0;
pSqlQuery *Queries = 0;

static ushort      NoTokens = 0;
static pSqlToken  *Tokens;
static ushort      NoServers = 0;
static pchar      *Servers;
static ushort      NoSchemas = 0;
static pchar      *Schemas;
static ushort      NoTables  = 0;
static pchar      *Tables;

static FILE *BinFile     = 0;

static ushort AddServer(char *Name);
static ushort AddSchema(char *Name);
static ushort AddTable(char *Name);
static void   AddQuery(tSqlQuery Query);

int SortComp(const pSqlQuery *p1, pSqlQuery *p2);

#include "sqldef.h"
#include "global.h"

ushort sqldefSign  = 0x0C1A;
ushort slinkSign   = 0x0C1B;
ushort sqldefSign2 = 0x0C2A;
ushort slinkSign2  = 0x0C2B;
ushort sqldefTailMark = 0xBEEF;

void OpenBinFile(const char *path, const char *BinDir, const char *BinExt)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(path, dir, fname, ext);
  if (strlen(BinDir))
    FNameMerge(newpath, BinDir, fname, BinExt);
  else
    FNameMerge(newpath, dir, fname, BinExt);
  BinFile = fopen(newpath, "wb");
  if (BinFile == 0)
    yyerror("error: Can't open file %s\n", newpath);
  else if (setvbuf(BinFile, 0, _IOFBF, 8192))
    yyerror("error: Can't allocate buffer for file %s\n", newpath);
  yyerror("%-13s: %s\n", "Output", newpath);
}

#if !defined(STANDALONE_APP)
int PortalLink(const char *ProjectName, const char *BinDir, const char *BinExt, const char *InputList, ushort version)
{
  char NextFileName[512];
  char *s = (char*)InputList, *t;
  ushort slink = version==1 ? slinkSign2 : slinkSign;
  for (;;)
  {
    t = strchr(s, '\r');
    if (t == 0)
      break;
    strncpyz(NextFileName, s, t-s);
    s = t+2;
    LoadInFile(NextFileName, slink);
  }
  qsort(Queries, NoQueries, sizeof(pSqlQuery), (fptr)SortComp);
  OpenBinFile(ProjectName, BinDir, BinExt);
  StoreBinFile(slink);
  return 0;
}
#endif

int SortComp(const pSqlQuery *p1, pSqlQuery *p2)
{
  return stricmp(p1[0]->Name, p2[0]->Name);
}

void LoadInFile(const char *InFileName, ushort &slink)
{
  FILE* InFile;
  int FileSize;
  tSqlQuery Query;
  pSqlQuery Q;
  pSqlToken Token;
  ushort i, j, NoProcs, sign;
  yyerror("%-13s: %s\n", "Input", InFileName);
  InFile = fopen(InFileName, "rb");
  if (InFile == 0)
  {
    yyerror("LoadinFile Access for file [%s] failed!\n", InFileName);
    goto Return;
  }
  if (setvbuf(InFile, 0, _IOFBF, 8192))
    yyerror("Not enough memory to buffer file!\n");
  FileSize = fseek(InFile, 0L, SEEK_END);
  fseek(InFile, 0L, SEEK_SET);
  sign = GetUShort(InFile);
  if (sign != sqldefSign && sign != sqldefSign2)
  {
    yyerror("Not a valid SO file!\n");
    goto Return;
  }
  else if (sign == sqldefSign2) // if anywhere a binary is sign2
    slink = slinkSign2;         // then the whole bin must be sign2
  Query.ServerNo = AddServer(GetString(InFile));
  Query.SchemaNo = AddSchema(GetString(InFile));
  Query.TableNo  = AddTable(GetString(InFile));
  NoProcs = GetUShort(InFile);
  for (i=0; i<NoProcs; i++)
  {
    Query.Name = GetString(InFile);
    Query.NoFields = GetUShort(InFile);
    Query.isSql = Query.NoFields & 0x8000;
    Query.isFetch = Query.NoFields & 0x4000;
    Query.isMultiFetch = Query.NoFields & 0x2000;
    Query.isNullEnabled = Query.NoFields & 0x1000;
    Query.NoFields &= 0x0FFF;
    if (Query.NoFields)
    {
      Query.Fields = (pSqlField)calloc(Query.NoFields, sizeof(tSqlField));
      for (j=0; j<Query.NoFields; j++)
      {
        pSqlField Field = &Query.Fields[j];
        Field->Name = GetString(InFile);
        Field->CType = GetShort(InFile);
        Field->SqlType = GetShort(InFile);
        if (sign == sqldefSign2)
        {
          Field->Size = GetULong(InFile);
          Field->Offset = GetULong(InFile);
          Field->Precision = GetULong(InFile);
          Field->Scale = GetULong(InFile);
        }
        else
        {
          Field->Size = GetUShort(InFile);
          Field->Offset = GetUShort(InFile);
          Field->Precision = GetUShort(InFile);
          Field->Scale = GetUShort(InFile);
        }
        Field->isBind = GetUShort(InFile);
        Field->isDefine = GetUShort(InFile);
      }
    }
    else
      Query.Fields = 0;
    Query.Size = GetUShort(InFile);
    Query.Command = GetExact(Query.Size, InFile);
    Q = (pSqlQuery)malloc(sizeof(Query));
    *Q = Query;
    AddList(Queries, pSqlQuery, Q, NoQueries, 16);
  }
  if (ftell(InFile) < FileSize)
  {
    sign = GetUShort(InFile);
    if (sign == sqldefTailMark)
      goto Return;
    fseek(InFile, -2L, SEEK_CUR);
    Token = (pSqlToken)calloc(sizeof(tSqlToken), 1);
    Token->Name = GetString(InFile);
    Token->Value = GetString(InFile);
    AddList(Tokens, pSqlToken, Token, NoTokens, 16);
  }
Return:
  if (InFile) fclose(InFile);
}

static ushort AddServer(char *Name)
{
  ushort n;
  for (n=0; n<NoServers; n++)
    if ((stricmp(Name, Servers[n])) == 0)
    {
      free(Name);
      return n;
    }
  n = NoServers;
  AddList(Servers, char*, Name, NoServers, 4);
  return n;
}

static ushort AddSchema(char *Name)
{
  ushort n;
  for (n=0; n<NoSchemas; n++)
    if ((stricmp(Name, Schemas[n])) == 0)
      {
      free(Name);
      return n;
      }
  n = NoSchemas;
  AddList(Schemas, char*, Name, NoSchemas, 4);
  return n;
}

static ushort AddTable(char *Name)
{
  ushort n;
  for (n=0; n<NoTables; n++)
    if ((stricmp(Name, Tables[n])) == 0)
      {
      free(Name);
      return n;
      }
  n = NoTables;
  AddList(Tables, char*, Name, NoTables, 16);
  return n;
}

void StoreBinFile(ushort sign)
{
  ushort i,j,dupl;
  PutUShort(sign, BinFile);
  PutUShort(NoServers, BinFile);
  for (i=0; i<NoServers; i++)
    PutString(Servers[i], BinFile);
  PutUShort(NoSchemas, BinFile);
  for (i=0; i<NoSchemas; i++)
    PutString(Schemas[i], BinFile);
  PutUShort(NoTables, BinFile);
  for (i=0; i<NoTables; i++)
    PutString(Tables[i], BinFile);
  for (i=0,dupl=0; i<NoQueries; i++)
  {
    if (i>0 && (stricmp(Queries[i-1]->Name, Queries[i]->Name)) == 0)
    {
      yyerror("Duplicate query %s not stored %100.100s\n",
                   Queries[i]->Name, Queries[i]->Command);
      dupl++;
    }
  }
  PutUShort(NoQueries-dupl, BinFile);
  for (i=0; i<NoQueries; i++)
  {
    if (i>0 && (stricmp(Queries[i-1]->Name, Queries[i]->Name)) == 0)
      continue;
    PutString(Queries[i]->Name,     BinFile);
    PutUShort(Queries[i]->ServerNo, BinFile);
    PutUShort(Queries[i]->SchemaNo | Queries[i]->isSql
                                   | Queries[i]->isFetch
                                   | Queries[i]->isMultiFetch
                                   | Queries[i]->isNullEnabled, BinFile);
    PutUShort(Queries[i]->TableNo,  BinFile);
    PutUShort(Queries[i]->NoFields, BinFile);
    for (j=0; j<Queries[i]->NoFields; j++)
    {
      pSqlField Field = &Queries[i]->Fields[j];
      PutString(Field->Name, BinFile);
      PutShort(Field->CType, BinFile);
      PutShort(Field->SqlType, BinFile);
      if (sign == slinkSign2)
      {
        PutULong(Field->Size, BinFile);
        PutULong(Field->Offset, BinFile);
        PutULong(Field->Precision, BinFile);
        PutULong(Field->Scale, BinFile);
      }
      else
      {
        PutUShort(Field->Size, BinFile);
        PutUShort(Field->Offset, BinFile);
        PutUShort(Field->Precision, BinFile);
        PutUShort(Field->Scale, BinFile);
      }
      PutUShort(Field->isBind, BinFile);
      PutUShort(Field->isDefine, BinFile);
    }
    PutUShort(Queries[i]->Size,     BinFile);
    PutExact (Queries[i]->Command,  BinFile);
  }
  PutUShort(NoTokens, BinFile);
  for (i=0; i<NoTokens; i++)
  {
    PutString(Tokens[i]->Name,     BinFile);
    PutString(Tokens[i]->Value,    BinFile);
  }
  if (BinFile) fclose(BinFile);
  if (Servers)
  {
    for (i=0; i<NoServers; i++)
      free(Servers[i]);
    free(Servers);
  }
  if (Schemas)
  {
    for (i=0; i<NoSchemas; i++)
      free(Schemas[i]);
    free(Schemas);
  }
  if (Tables)
  {
    for (i=0; i<NoTables; i++)
      free(Tables[i]);
    free(Tables);
  }
  if (Queries)
  {
    for (i=0; i<NoQueries; i++)
    {
      free(Queries[i]->Name);
      for (j=0; j<Queries[i]->NoFields; j++)
      {
        pSqlField Field = &Queries[i]->Fields[j];
        free(Field->Name);
      }
      if (Queries[i]->Fields)
        free(Queries[i]->Fields);
      free(Queries[i]->Command);
      free(Queries[i]);
    }
    if (i)
      free(Queries);
  }
  if (Tokens)
  {
    for (i=0; i<NoTokens; i++)
    {
      free(Tokens[i]->Name);
      free(Tokens[i]->Value);
      free(Tokens[i]);
    }
    if (i)
      free(Tokens);
  }
  Servers = 0;
  NoServers = 0;
  Schemas = 0;
  NoSchemas = 0;
  Tables = 0;
  NoTables = 0;
  Queries = 0;
  NoQueries = 0;
  Tokens = 0;
  NoTokens = 0;
  BinFile = 0;
}


