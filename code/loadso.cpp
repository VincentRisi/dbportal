#include "loadso.h"
#include "sqldef.h"
#include "global.h"

#define ADD_TO_LIST(POINTER, TYPE, DATA, INDEX, DELTA) do {\
  if (INDEX % DELTA == 0) \
    POINTER = (TYPE*)realloc(POINTER, sizeof(DATA)*(INDEX + DELTA)); \
  POINTER[INDEX++] = DATA; \
  } while(0)

const ushort sqldefSign     = 0x0C1A;
const ushort sqldefSign2    = 0x0C2A;
const ushort sqldefTailMark = 0xBEEF;

int SqlSO::loadInFile(const char *inFileName)
{
  int result = 0;
  FILE* inFile;
  int fileSize;
  PSqlQuery query;
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
  sign = getUInt16(inFile);
  if (sign != sqldefSign && sign != sqldefSign2)
  {
    result = 2;
    printf("Not a valid SO file!\n");
    goto RETURN;
  }
  server  = getString(inFile);
  schema  = getString(inFile);
  table   = getString(inFile);
  noProcs = getUInt16(inFile);
  for (i=0; i<noProcs; i++)
  {
    query = (PSqlQuery) calloc(1, sizeof(SqlQuery));
    query->Name = getString(inFile);
    query->NoFields = getUInt16(inFile);
    if (query->NoFields & 0x8000) query->isSql = 1;
    if (query->NoFields & 0x4000) query->isSingle = 1;
    if (query->NoFields & 0x2000) query->isManyQuery = 1;
    if (query->NoFields & 0x1000) query->isNullEnabled = 1;
    query->NoFields &= 0x0FFF;
    if (query->NoFields)
    {
      query->Fields = (PSqlField) calloc(query->NoFields, sizeof(SqlField));
      for (j=0; j<query->NoFields; j++)
      {
        PSqlField field = &query->Fields[j];
        field->Name = getString(inFile);
        field->CType = getInt16(inFile);
        field->SqlType = getInt16(inFile);
        if (sign == sqldefSign2)
        {
          field->Size = getUInt32(inFile);
          field->Offset = getUInt32(inFile);
          field->Precision = getUInt32(inFile);
          field->Scale = getUInt32(inFile);
        }
        else
        {
          field->Size = getUInt16(inFile);
          field->Offset = getUInt16(inFile);
          field->Precision = getUInt16(inFile);
          field->Scale = getUInt16(inFile);
        }
        field->isBind = getUInt16(inFile);
        field->isDefine = getUInt16(inFile);
      }
    }
    else
      query->Fields = 0;
    query->Size = getUInt16(inFile);
    query->Command = getExact(query->Size, inFile);
    ADD_TO_LIST(queries, PSqlQuery, query, noQueries, 16);
  }
  if (ftell(inFile) < fileSize)
  {
    sign = getUInt16(inFile);
    if (sign == sqldefTailMark)
      goto RETURN;
    fseek(inFile, -2L, SEEK_CUR);
    token = (PSqlToken)calloc(sizeof(SqlToken), 1);
    token->Name = getString(inFile);
    token->Value = getString(inFile);
    ADD_TO_LIST(tokens, PSqlToken, token, noTokens, 16);
    printf("%s=%s\n", token->Name, token->Value);
  }
RETURN:
  if (inFile) fclose(inFile);
  return result;
}

char* SqlSO::dehash(char* work, int worklen, const char *name)
{
  int i;
  inline_copy(work, name, worklen);
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
  return inline_copy(work, result, worklen);
}

char* SqlSO::makeOutName(char* work, int worklen
                       , const char* inFileName
                       , const char* outExt
                       , const char* outDir)
{
  char result[1024];
  char dir[1024];
  char ext[1024];
  char fname[1024];
  FNameSplit(inFileName, dir, fname, ext);
  if (outDir[0] != 0)   
    FNameMerge(result, outDir, fname, outExt);
  else
    FNameMerge(result, dir, fname, outExt);
  return inline_copy(work, result, worklen);
}
