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
  case SQL_C_BINARY:
    if (field->SqlType == SQL_OCI_IMAGE)
      fprintf(ofile, "  struct {short int len;unsigned char data[%u];} %s;\n"
                   , field->Size-2, name);
    else
      fprintf(ofile, "  unsigned char %s[%hu];\n", name, field->Size);
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
  //if (hasDynamic == false)
  //  return;
  //fprintf(ofile, "inline char* DYNAMIC(%sRec &rec, const char* name)\n{\n"
  //             , query->Name
  //               );
  //for (f=0; f<query->NoFields; f++)
  //{
  //  PSqlField field = &query->Fields[f];
  //  if (field->isBind|field->isDefine)
  //    continue;
  //  fprintf(ofile, "  if ((strcmp(name, \"%s\")) == 0)\n"
  //                 "    return rec.%s;\n"
  //               , field->Name+1, field->Name+1
  //               );
  //}
  //fprintf(ofile, "  return \"\";\n"
  //               "}\n\n");
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
  fprintf(out.file, "#include <stdio.h>\n");
  fprintf(out.file, "#include <string.h>\n");
  fprintf(out.file, "#include <sqlca.h>\n");
  fprintf(out.file, "#include <stdlib.h>\n");
  fprintf(out.file, "#include <sqlda.h>\n");
  fprintf(out.file, "#include <sqlcpr.h>\n\n");
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
  case SQL_C_BINARY:
    if (field->SqlType == SQL_OCI_IMAGE)
      fprintf(ofile, "  unsigned char *%s = (unsigned char *)&rec->%s;\n", name, name);
    else
      fprintf(ofile, "  unsigned char *%s = rec->%s;\n", name, name);
    break;
  case SQL_C_CLIMAGE:
      fprintf(ofile, "  unsigned char *%s = (unsigned char *)&rec->%s;\n", name, name);
    break;
  case SQL_C_BLIMAGE:
      fprintf(ofile, "  unsigned char *%s = (unsigned char *)&rec->%s;\n", name, name);
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
    fprintf(ofile, "  short int *ind_%s = &rec->ind.%s;\n", name, name);
}

#define SPACE_IGNORE "\n\t "
#define FRONT_SPACE_REMOVE ",()."
#define BACK_SPACE_REMOVE "(."

static char* makeInto(char* work
  , int worklen
  , PSqlQuery query
  , int &used
  )
{
  char *p = work; 
  int no = 0;
  bool useComma = false;
  no += sprintf(p+no, " INTO");
  for (int f=0; f < query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (!field->isDefine)
      continue;
    no += sprintf(p+no, "%s%s", useComma ? ", " : " ", field->Name);
    if ((field->isDefine & fieldIsNullable)
    &&  (field->isDefine & fieldIsNull))
      no += sprintf(p+no, ":ind_%s", field->Name+1);
    useComma = true;
  }
  used += no;
  return p+no;
}

static char* makeCommand(char* work
  , int worklen
  , char *dyns
  , int dynslen
  , PSqlQuery query
  , const char* name
  , bool useInds
  , bool hasDynamic
  )
{
  char token[256];
  char *source = query->Command;
  char *p = work, *d = dyns; 
  char sp[2]; 
  sp[0] = sp[1] = 0;
  int no, used = 0;
  int bcount = 0;
  bool inSelect = false;
  dyns[0] = 0;
  while (true)
  {
    source = SqlToken(source, token, sizeof(token));
    if (strchr(SPACE_IGNORE, *token) != 0)
      continue;
    if (query->isSingle)
    {
      if (*token == '(')
        bcount++;
      else if (*token == ')')
        bcount--;
      if (bcount < 0)
        bcount = 0;
      if (bcount == 0) 
      {
        if (stricmp(token, "select") == 0)
          inSelect = true;
        else if (inSelect == true && stricmp(token, "from") == 0)
          p = makeInto(p, worklen-strlen(p), query, used);
      }  
    }
    if (strchr(FRONT_SPACE_REMOVE, *token) != 0)
      sp[0] = 0;
    if (stricmp(token, "select") == 0 
    ||  stricmp(token, "where")  == 0
    ||  stricmp(token, "from")   == 0
    ||  stricmp(token, "count")  == 0
    ||  stricmp(token, "and")    == 0
    ||  stricmp(token, "or")     == 0
    ||  stricmp(token, "for")    == 0
    ||  stricmp(token, "update") == 0)
      strupr(token);
    if (*token == '$')
    {
      d += sprintf(d, ", %s", token+1);
      strcpy(token, "%s");
    }
    no = sprintf(p, "%s%s", sp, token);
    used += no;
    if (strchr(BACK_SPACE_REMOVE, *token) == 0)
      sp[0] = ' ';
    p += no;
    if (source == 0 || *source == 0)
      break;
  }
  return work;
}

static void splitLines(FILE* ofile, const char* command, const char* leader, const char* ch)
{
  char line[80];
  const char *p = command; 
  char *r;
  int max = 72 - strlen(leader), size = strlen(p);
  fprintf(ofile, "  %s", leader);
  while (size > 0) 
  {
    if (size <= max)
    {
      fprintf(ofile, "  %s%s%s;\n", ch, p, ch);
      break;
    }
    strncpy(line, p, max);
    line[max] = 0;
    r = strrchr(line, ' ');
    if (r != 0)
      *r = 0;
    fprintf(ofile, "  %s%s%s\n", ch, line, ch);
    p += strlen(line);
    size = strlen(p);
    max = 72;
  }
}

static void doSingle(FILE* ofile, PSqlQuery query
  , const char *name
  , bool useInds
  , bool hasDynamic
  )
{
  int f;
  char work[8192], dyns[1024];
  fprintf(ofile, "int %s(P%sRec rec)\n{\n"
               , query->Name, name
               );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    doCodeField(ofile, field);
  }
  fprintf(ofile, "  int result = 0;\n"
                 "  EXEC SQL WHENEVER NOT FOUND GOTO not_found;\n"
               );
  makeCommand(work, sizeof(work), dyns, sizeof(dyns), query, name, useInds, hasDynamic);
  if (hasDynamic == true)
  {
    splitLines(ofile, work, "const char * _immed_ = ", "\"");
    fprintf(ofile, "  char _command[8192];\n");
    fprintf(ofile, "  snprintf(_command, 8192, _immed%s);\n", dyns);
    fprintf(ofile, "  EXEC SQL EXECUTE IMMEDIATE :_command;\n");
  }
  else  
    splitLines(ofile, work, "EXEC SQL", "");
  fprintf(ofile, "  result = 1;\n");
  fprintf(ofile, "not_found:\n");  fprintf(ofile, "  return result;\n");
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
    if (field->isBind)
      doCodeField(ofile, field);
  }
  fprintf(ofile, "}\n\n");
  fprintf(ofile, "int %s_Fetch(P%sRec rec)\n{\n"
               , query->Name, name
               );
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (field->isDefine)
      doCodeField(ofile, field);
  }
  fprintf(ofile, "  %s_Clear(rec);\n", query->Name);
  fprintf(ofile, "  return 0;\n");
  fprintf(ofile, "}\n\n");
  fprintf(ofile, "int %s(P%sRec rec, P%sRec* recs)\n{\n"
               , query->Name, name
               , name
               );
  fprintf(ofile, "  int noRecs = 0;\n");
  fprintf(ofile, "  %s_Exec(rec);\n", query->Name);
  fprintf(ofile, "  while (%s_Fetch(rec))\n", query->Name);
  fprintf(ofile, "    ADD_TO_LIST(recs, P%sRec, rec, noRecs, 32);\n", name);
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

static void doFieldClear(FILE* ofile, PSqlField field)
{
  char type[32];
  char *name = field->Name+1;
  switch (field->CType)
  {
  case SQL_C_CHAR:
  case SQL_C_DATE:
  case SQL_C_TIME:
  case SQL_C_TIMESTAMP:
    fprintf(ofile, "  rec->%s[0] = 0;\n", name);
    break;
  case SQL_C_DOUBLE:
  case SQL_C_FLOAT:
    fprintf(ofile, "  rec->%s = 0.0;\n", name);
    break;
  case SQL_C_LONG64:
  case SQL_C_LONG:
  case SQL_C_SHORT:
  case SQL_C_TINYINT:
  case SQL_C_BIT:
    fprintf(ofile, "  rec->%s = 0;\n", name);
    break;
  case SQL_C_BINARY:
    if (field->SqlType == SQL_OCI_IMAGE)
    {
      fprintf(ofile, "  rec->%s.len = 0;\n", name);
      fprintf(ofile, "  rec->%s.data[0] = 0;\n", name);
    }
    else
      fprintf(ofile, "  rec->%s[0] = 0;\n", name);
    break;
  case SQL_C_CLIMAGE:
  case SQL_C_BLIMAGE:
    fprintf(ofile, "  rec->%s.len = 0;\n", name);
    fprintf(ofile, "  rec->%s.data[0] = 0;\n", name);
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
    fprintf(ofile, "  // %04x %s;\n", (unsigned)field->CType, name);
    break;
  }
  if (((field->isBind|field->isDefine) & fieldIsNullable)
  &&  ((field->isBind|field->isDefine) & fieldIsNull))
    fprintf(ofile, "  rec->ind.%s = 0;\n", name);
}

static void doClear(FILE* ofile, PSqlQuery query, const char* name)
{
  int f;
  fprintf(ofile, "void %s_Clear(P%sRec rec)\n{\n", query->Name, name);
  for (f=0; f<query->NoFields; f++)
  {
    PSqlField field = &query->Fields[f];
    if (field->isDefine)
      doFieldClear(ofile, field);
  }
  fprintf(ofile, "}\n\n");
}

static int doCode(SqlSO& sqlSO, const char* inName)
{
  int result = COMPLETED_OK;
  char outName[1024];
  SqlSO::makeOutName(outName, sizeof(outName), inName, ".pc", outDirName);
  AutoXdir ax(outName);  
  AutoFILE out(outName, "wt");
  fprintf(out.file, "#include \"%s.h\"\n\n", ax.name);
  fprintf(out.file, "%s\n", ADD_TO_LIST);
  for (int q = 0; q < sqlSO.noQueries; q++)
  {
    PSqlQuery query = sqlSO.queries[q];
    bool isStd = queryIsStd(query);
    if (query->isSql)
    {
      if (isStd)
        doClear(out.file, query, sqlSO.table); 
      else
        doClear(out.file, query, query->Name); 
      doQueryCode(out.file, query, sqlSO.table, isStd);
    }
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
