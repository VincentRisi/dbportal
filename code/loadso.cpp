#include "loadso.h"

#define ADD_TO_LIST(POINTER, TYPE, DATA, INDEX, DELTA) do {\
  if (INDEX % DELTA == 0) \
    POINTER = (TYPE*)realloc(POINTER, sizeof(DATA)*(INDEX + DELTA)); \
  POINTER[INDEX++] = DATA; \
  } while(0)

#include "sqldef.h"
#include "global.h"

ushort sqldefSign     = 0x0C1A;
ushort sqldefSign2    = 0x0C2A;
ushort sqldefTailMark = 0xBEEF;

int SqlSO::loadInFile(const char *inFileName)
{
  int result = 0;
  FILE* inFile;
  int fileSize;
  SqlQuery query;
  PSqlQuery pQ;
  PSqlToken token;
  ushort i, j, noProcs, sign;
  printf("%-13s: %s\n", "Input", inFileName);
  inFile = fopen(inFileName, "rb");
  if (inFile == 0)
  {
    result = 1;
    printf("loadinFile Access for file [%s] failed!\n", inFileName);
    goto RETURN;
  }
  if (setvbuf(inFile, 0, _IOFBF, 8192))
    printf("Not enough memory to buffer file!\n");
  fileSize = fseek(inFile, 0L, SEEK_END);
  fseek(inFile, 0L, SEEK_SET);
  sign = GetUShort(inFile);
  if (sign != sqldefSign && sign != sqldefSign2)
  {
    result = 2;
    printf("Not a valid SO file!\n");
    goto RETURN;
  }
  query.ServerNo = addServer(GetString(inFile));
  query.SchemaNo = addSchema(GetString(inFile));
  query.TableNo  = addTable(GetString(inFile));
  noProcs = GetUShort(inFile);
  for (i=0; i<noProcs; i++)
  {
    query.Name = GetString(inFile);
    query.NoFields = GetUShort(inFile);
    query.isSql = query.NoFields & 0x8000;
    query.isFetch = query.NoFields & 0x4000;
    query.isMultiFetch = query.NoFields & 0x2000;
    query.isNullEnabled = query.NoFields & 0x1000;
    query.NoFields &= 0x0FFF;
    if (query.NoFields)
    {
      query.Fields = (PSqlField)calloc(query.NoFields, sizeof(SqlField));
      for (j=0; j<query.NoFields; j++)
      {
        PSqlField field = &query.Fields[j];
        field->Name = GetString(inFile);
        field->CType = GetShort(inFile);
        field->SqlType = GetShort(inFile);
        if (sign == sqldefSign2)
        {
          field->Size = GetULong(inFile);
          field->Offset = GetULong(inFile);
          field->Precision = GetULong(inFile);
          field->Scale = GetULong(inFile);
        }
        else
        {
          field->Size = GetUShort(inFile);
          field->Offset = GetUShort(inFile);
          field->Precision = GetUShort(inFile);
          field->Scale = GetUShort(inFile);
        }
        field->isBind = GetUShort(inFile);
        field->isDefine = GetUShort(inFile);
      }
    }
    else
      query.Fields = 0;
    query.Size = GetUShort(inFile);
    query.Command = GetExact(query.Size, inFile);
    pQ = (PSqlQuery)malloc(sizeof(query));
    *pQ = query;
    ADD_TO_LIST(queries, PSqlQuery, pQ, noQueries, 16);
  }
  if (ftell(inFile) < fileSize)
  {
    sign = GetUShort(inFile);
    if (sign == sqldefTailMark)
      goto RETURN;
    fseek(inFile, -2L, SEEK_CUR);
    token = (PSqlToken)calloc(sizeof(SqlToken), 1);
    token->Name = GetString(inFile);
    token->Value = GetString(inFile);
    ADD_TO_LIST(tokens, PSqlToken, token, noTokens, 16);
  }
RETURN:
  if (inFile) fclose(inFile);
  return result;
}

int SqlSO::addServer(char *name)
{
  int n;
  for (n=0; n<noServers; n++)
    if ((stricmp(name, servers[n])) == 0)
    {
      free(name);
      return n;
    }
  n = noServers;
  ADD_TO_LIST(servers, char*, name, noServers, 4);
  return n;
}

int SqlSO::addSchema(char *name)
{
  int n;
  for (n=0; n<noSchemas; n++)
    if ((stricmp(name, schemas[n])) == 0)
      {
      free(name);
      return n;
      }
  n = noSchemas;
  ADD_TO_LIST(schemas, char*, name, noSchemas, 4);
  return n;
}

int SqlSO::addTable(char *name)
{
  int n;
  for (n=0; n<noTables; n++)
    if ((stricmp(name, tables[n])) == 0)
      {
      free(name);
      return n;
      }
  n = noTables;
  ADD_TO_LIST(tables, char*, name, noTables, 16);
  return n;
}

inline char* _copy(char* target, const char* source, int len)
{
  char* result = strncpy(target, source, len-1);
  target[len-1] = 0;
  return result;
}

char* SqlSO::dehash(char* work, int worklen, const char *name)
{
  int i;
  _copy(work, name, worklen);
  for (i=0; i < (int)strlen(work); i++)
  {
    if (work[i] == '#' || work[i] == '$')
      work[i] = '_';
  }
  return work;
}

char* SqlSO::nameExt(char* work, int worklen, const char *path)
{
  char dir[1024];
  char ext[1024];
  char fname[1024];
  char result[1024];
  FNameSplit(path, dir, fname, ext);
  FNameMerge(result, "", fname, ext);
  return _copy(work, result, worklen);
}

char* SqlSO::makeOutName(char* work, int worklen
                       , const char* inFileName
                       , char* outExt, char* outDir)
{
  char result[1024];
  char dir[1024];
  char ext[1024];
  char fname[1024];
  FNameSplit(inFileName, outDir, fname, ext);
  if (outDir[0] != 0)   
    FNameMerge(result, outDir, fname, outExt);
  else
    FNameMerge(result, dir, fname, outExt);
  return _copy(work, result, worklen);
}
