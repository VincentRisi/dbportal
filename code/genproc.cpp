#include "genproc.h"
#include <string.h>
#include "sqlcons.h"

static const char* outDirName = "";
static const char* inListName = "";
static const char* whitespace = " ;\r\n\"";
static const char* ADD_TO_LIST =
  "#define ADD_TO_LIST(POINTER, TYPE, DATA, INDEX, DELTA) do {\\\n"
  "  if (INDEX % DELTA == 0)\\\n"
  "    POINTER = (TYPE*)realloc(POINTER, sizeof(DATA)*(INDEX + DELTA));\\\n"
  "  POINTER[INDEX++] = DATA;\\\n"
  "  } while(0)\n\n";

ARG argtab[] =
{ {'o', STRING,  (int*) &outDirName,    "Output Directory"}
, {'f', STRING,  (int*) &inListName,    "Input file list"} 
};
#define TABSIZE (sizeof(argtab) / sizeof(ARG))

enum
{ COMPLETED_OK
, INLIST_FILE_NOT_OPENED
};

struct AutoFILE
{
  FILE* file;
  AutoFILE(const char *name, const char* mode)
  {
    file = fopen(name, mode);
  }
  ~AutoFILE() 
  {
    if (file) fclose(file);
  }
};

struct AutoXdir
{
  char* dir;
  char* name;
  char* ext;
  AutoXdir(char* path)
  {
    name = FNameName(path);    
    dir  = FNameDir(path);
    ext  = FNameExt(path);
  }
  ~AutoXdir()
  {
    free(name);
    free(dir);
    free(ext);
  }
};

static void doStructField(FILE* ofile, PSqlField field)
{
  char type[32];
  char *name = field->Name+1;
  switch (field->CType)
  {
  case SQL_C_BINARY:
    fprintf(ofile, "  unsigned char %s[%hu];\n", name, field->Size);
    break;
  case SQL_C_CHAR:
  case SQL_C_DATE:
  case SQL_C_TIME:
  case SQL_C_TIMESTAMP:
    fprintf(ofile, "  char %s[%hu];\n", name, field->Size);
    break;
  case SQL_C_DOUBLE:
  case SQL_C_FLOAT:
    fprintf(ofile, "  double %s;\n", name);
    break;
  case SQL_C_LONG64:
    fprintf(ofile, "  long long %s;\n", name);
    break;
  case SQL_C_LONG:
    fprintf(ofile, "  int %s;\n", name);
    break;
  case SQL_C_SHORT:
    fprintf(ofile, "  short int %s;\n", name);
    break;
  case SQL_C_TINYINT:
  case SQL_C_BIT:
    fprintf(ofile, "  char %s;\n", name);
    break;
  case SQL_C_CLIMAGE:
    fprintf(ofile, "  struct {short int len;unsigned char data[%u];} %s;\n"
                 , field->Size-2, name);
    break;
  case SQL_C_BLIMAGE:
    fprintf(ofile, "  struct {int len;unsigned char data[%u];} %s;\n"
                 , field->Size-4, name);
    break;
  case SQL_C_ZLIMAGE:
    fprintf(ofile, "  // SQL_C_ZLIMAGE} %s;\n", name);
    break;
  case SQL_C_HUGECHAR:
    fprintf(ofile, "  // SQL_C_HUGECHAR} %s;\n", name);
    break;
  case SQL_C_XMLTYPE:
    fprintf(ofile, "  // SQL_C_XMLTYPE} %s;\n", name);
    break;
  default:  
    if (field->isBind == 0 && field->isDefine == 0)
      fprintf(ofile, "  char %s[%hu];\n", name, field->Size);
    else  
      fprintf(ofile, "  // %04x %s;\n", (unsigned)field->CType, name);
    break;
  }
}

static void doQueryStructs(FILE* ofile, PSqlQuery query, const char* table, bool isStd)
{
  int f;
  fprintf(ofile, "typedef struct\n");
  fprintf(ofile, "{\n");
  bool useInds = false;
  bool hasDynamic = false;
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (((field->isBind|field->isDefine) & fieldIsNullable)
    &&  ((field->isBind|field->isDefine) & fieldIsNull))
      useInds = true;
    if (field->isBind|field->isDefine)
      doStructField(ofile, field);
    else
    {
      hasDynamic = true;
      fprintf(ofile, "  char %s[%hu]; //DYN\n", field->Name+1, field->Size+1);
    }
  }
  if (query->isNullEnabled && useInds == true)
  {
    fprintf(ofile, "  struct\n");
    fprintf(ofile, "  {\n");
    for (f=0; f<query->NoFields; f++)
    {
      PSqlField field = &query->Fields[f];
      if (((field->isBind|field->isDefine) & fieldIsNullable)
      &&  ((field->isBind|field->isDefine) & fieldIsNull))
        fprintf(ofile, "    unsigned short %s;\n"
                     , field->Name+1
                     );
    }
    fprintf(ofile, "  } ind;\n");
  }
  if (isStd == true)
    fprintf(ofile, "} %sRec, *P%sRec;\n\n", table, table);
  else
    fprintf(ofile, "} %sRec, *P%sRec;\n\n", query->Name, query->Name);
  if (hasDynamic == false)
    return;
  fprintf(ofile, "inline char* DYNAMIC(%sRec &rec, const char* name)\n{\n"
               , query->Name
                 );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (field->isBind|field->isDefine)
      continue;
    fprintf(ofile, "  if ((strcmp(name, \"%s\")) == 0)\n"
                   "    return rec.%s;\n"
                 , field->Name+1, field->Name+1
                 );
  }
  fprintf(ofile, "  return \"\";\n"
                 "}\n\n");
}

static bool queryIsStd(PSqlQuery query)
{
  char *result;
  result = strstr(query->Name, "Insert"); 
  if (result != 0 && strlen(result) == 6) 
    return true;
  result = strstr(query->Name, "Update"); 
  if (result != 0 && strlen(result) == 6) 
    return true;
  result = strstr(query->Name, "SelectOne"); 
  if (result != 0 && strlen(result) == 9) 
    return true;
  result = strstr(query->Name, "SelectOneUpd"); 
  if (result != 0 && strlen(result) == 12) 
    return true;
  result = strstr(query->Name, "SelectAll"); 
  if (result != 0 && strlen(result) == 9) 
    return true;
  result = strstr(query->Name, "SelectAllUpd"); 
  if (result != 0 && strlen(result) == 12) 
    return true;
  return false;
}

static int doHeader(SqlSO& sqlSO, const char* inName)
{
  int result = COMPLETED_OK;
  char outName[1024];
  SqlSO::makeOutName(outName, sizeof(outName), inName, ".h", outDirName);
  AutoXdir ax(outName);  
  AutoFILE out(outName, "wt");
  fprintf(out.file, "#ifndef _%s_H_\n", ax.name);
  fprintf(out.file, "#define _%s_H_\n\n", ax.name);
  bool hasStd = false;
  for (int q = 0; q < sqlSO.noQueries; q++)
  {
    PSqlQuery query = sqlSO.queries[q];
    bool isStd = queryIsStd(query);
    if (query->isSql)
    {
      if (isStd == true && hasStd == true)
        continue;
      if (isStd == true)
        hasStd = true;
      doQueryStructs(out.file, query, sqlSO.table, isStd);
    }
  }
  fprintf(out.file, "#endif\n");
  return result;
}

static void doCodeField(FILE* ofile, PSqlField field)
{
  char type[32];
  char *name = field->Name+1;
  switch (field->CType)
  {
  case SQL_C_BINARY:
    fprintf(ofile, "  unsigned char *%s = rec->%s;\n", name, name);
    break;
  case SQL_C_CHAR:
  case SQL_C_DATE:
  case SQL_C_TIME:
  case SQL_C_TIMESTAMP:
    fprintf(ofile, "  char *%s = rec->%s;\n", name, name);
    break;
  case SQL_C_DOUBLE:
  case SQL_C_FLOAT:
    fprintf(ofile, "  double *%s = &rec->%s;\n", name, name);
    break;
  case SQL_C_LONG64:
    fprintf(ofile, "  long long *%s = &rec->%s;\n", name, name);
    break;
  case SQL_C_LONG:
    fprintf(ofile, "  int *%s = &rec->%s;\n", name, name);
    break;
  case SQL_C_SHORT:
    fprintf(ofile, "  short int *%s = &rec->%s;\n", name, name);
    break;
  case SQL_C_TINYINT:
  case SQL_C_BIT:
    fprintf(ofile, "  char *%s = &rec->%s;\n", name, name);
    break;
  case SQL_C_CLIMAGE:
    fprintf(ofile, "  short int *%s_len = &rec->%s.len;\n", name, name);
    fprintf(ofile, "  unsigned char *%s_data = rec->%s.data;\n", name, name);
    break;
  case SQL_C_BLIMAGE:
    fprintf(ofile, "  int *%s_len = &rec->%s.len;\n", name, name);
    fprintf(ofile, "  unsigned char *%s_data = rec->%s.data;\n", name, name);
    break;
  case SQL_C_ZLIMAGE:
    fprintf(ofile, "  // SQL_C_ZLIMAGE} %s;\n", name);
    break;
  case SQL_C_HUGECHAR:
    fprintf(ofile, "  // SQL_C_HUGECHAR} %s;\n", name);
    break;
  case SQL_C_XMLTYPE:
    fprintf(ofile, "  // SQL_C_XMLTYPE} %s;\n", name);
    break;
  default:  
    if (field->isBind == 0 && field->isDefine == 0)
      fprintf(ofile, "  char *%s = rec->%s;\n", name, name);
  else
      fprintf(ofile, "  // %04x %s;\n", (unsigned)field->CType, name);
    break;
  }
  if (((field->isBind|field->isDefine) & fieldIsNullable)
  &&  ((field->isBind|field->isDefine) & fieldIsNull))
    fprintf(ofile, "  short int *%s_ind = &rec->ind.%s;\n", name, name);
}

static void doSingle(FILE* ofile, PSqlQuery query
  , const char *name
  , bool useInds
  , bool hasDynamic
  )
{
  int f;
  fprintf(ofile, "bool %s(P%sRec rec)\n{\n"
               , query->Name, name
               );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    doCodeField(ofile, field);
  }
  fprintf(ofile, "}\n\n");
}

static void doManyQuery(FILE* ofile, PSqlQuery query
  , const char *name
  , bool useInds
  , bool hasDynamic
  )
{
  int f;
  fprintf(ofile, "void %s_Exec(P%sRec rec)\n{\n"
               , query->Name, name
               );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    doCodeField(ofile, field);
  }
  fprintf(ofile, "}\n\n");
  fprintf(ofile, "bool %s_Fetch(P%sRec rec)\n{\n"
               , query->Name, name
               );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    doCodeField(ofile, field);
  }
  fprintf(ofile, "  return false;\n");
  fprintf(ofile, "}\n\n");
  fprintf(ofile, "int %s(P%sRec rec, P%sRec* recs)\n{\n"
               , query->Name, name
               , name
               );
  fprintf(ofile, "  int noRecs = 0;\n");
  fprintf(ofile, "  %s_Exec(rec);\n", query->Name);
  fprintf(ofile, "  while (%s_Fetch(rec))\n", query->Name);
  fprintf(ofile, "    ADD_TO_LIST(recs, P%sRec, rec, noRecs, 32);\n", query->Name);
  fprintf(ofile, "  return noRecs;\n");
  fprintf(ofile, "}\n\n");
}

static void doAction(FILE* ofile, PSqlQuery query
  , const char *name
  , bool useInds
  , bool hasDynamic
  )
{
  int f;
  fprintf(ofile, "void %s(P%sRec rec)\n{\n", query->Name, name);
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    doCodeField(ofile, field);
  }
  fprintf(ofile, "}\n\n");
}

static void doQueryCode(FILE* ofile, PSqlQuery query, const char* table, bool isStd)
{
  int f;
  bool useInds = false;
  bool hasDynamic = false;
  const char *name;
  if (isStd)
    name = table;
  else
    name = query->Name;
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (((field->isBind|field->isDefine) & fieldIsNullable)
    &&  ((field->isBind|field->isDefine) & fieldIsNull))
      useInds = true;
    if (field->isBind|field->isDefine)
      continue;
    hasDynamic = true;
  }
  if (query->isSingle)
  {
    doSingle(ofile, query, name, useInds, hasDynamic);
    return;
  }
  if (query->isManyQuery)
  {
    doManyQuery(ofile, query, name, useInds, hasDynamic);
    return;
  }
  doAction(ofile, query, name, useInds, hasDynamic);
}

static int doCode(SqlSO& sqlSO, const char* inName)
{
  int result = COMPLETED_OK;
  char outName[1024];
  SqlSO::makeOutName(outName, sizeof(outName), inName, ".pc", outDirName);
  AutoXdir ax(outName);  
  AutoFILE out(outName, "wt");
  fprintf(out.file, "#include \"%s.h\"\n\n", ax.name);
  fprintf(out.file, "%s", ADD_TO_LIST);
  for (int q = 0; q < sqlSO.noQueries; q++)
  {
    PSqlQuery query = sqlSO.queries[q];
    bool isStd = queryIsStd(query);
    if (query->isSql)
      doQueryCode(out.file, query, sqlSO.table, isStd);
  }
  return result;
}

static int generate(const char* inName)
{
  int result = COMPLETED_OK;
  SqlSO sqlSO;
  int rc = sqlSO.loadInFile(inName);
  if (result == COMPLETED_OK)
    result = rc;
  if (rc == 0)
  {
    rc = doHeader(sqlSO, inName) || doCode(sqlSO, inName);
    if (result == COMPLETED_OK)
      result = rc;
  }
  return result;
}

static int generateInList(const char* inListName)
{
  int result = COMPLETED_OK;
  if (inListName[0] == 0)
    return result;
  char buff[8192], fileName[1024], *pb;
  const char* ws;
  AutoFILE in(inListName, "rt");
  if (in.file == 0)
    return INLIST_FILE_NOT_OPENED;
  while (1)
  {
    if (fgets(buff, sizeof(buff), in.file) == 0)
      break;
    pb = buff;
    while(1)
    {
      int i=0, no = strspn(pb, whitespace);
      pb += no;
      if (no > 0 && pb[no-1] == '"')
        i = 1;
      no = strcspn(pb, whitespace+i);
      if (no == 0)
        break;
      inline_copy(fileName, pb-i, no+i+i);
      int rc = generate(fileName);
      if (result == COMPLETED_OK)
        result = rc;
    }
  }
  return result;
}

int main(int argc, char *argv[])
{
  int result = COMPLETED_OK;
  argc = GetArgs(argc, argv, argtab, TABSIZE);
  int rc = generateInList(inListName);
  if (result == COMPLETED_OK)
    result = rc;
  for (int i=1; i<argc; i++)
  {
    rc = generate(argv[i]);
    if (result == COMPLETED_OK)
      result = rc;
  }
  return result;
}
