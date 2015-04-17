#include "genproc.h"
#include <string.h>
#include "sqlcons.h"

static const char* outDirName = "";
static const char* inListName = "";
static const char* whitespace = " ;\r\n\"";

ARG argtab[] =
{ {'o', STRING,  (int*) &outDirName, "Output Directory"}
, {'f', STRING,  (int*) &inListName, "Input file list"} 
};
#define TABSIZE (sizeof(argtab) / sizeof(ARG))

enum
{ COMPLETED_OK
, INLIST_FILE_NOT_OPENED
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
    fprintf(ofile, "  // %04x %s;\n", (unsigned)field->CType, name);
    break;
  }
}

static void doQueryStructs(FILE* ofile, PSqlQuery query)
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
    fprintf(ofile, "  } null;\n");
  }
  fprintf(ofile, "} %sRec;\n\n", query->Name);
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

static int doHeader(SqlSO& sqlSO, const char* inName)
{
  int result = COMPLETED_OK;
  char outName[1024];
  SqlSO::makeOutName(outName, sizeof(outName), inName, ".h", outDirName);
  AutoXdir ax(outName);  
  FILE* ofile = fopen(outName, "wt");
  fprintf(ofile, "#ifndef _%s_H_\n", ax.name);
  fprintf(ofile, "#define _%s_H_\n\n", ax.name);
  for (int q = 0; q < sqlSO.noQueries; q++)
  {
    PSqlQuery query = sqlSO.queries[q];
    doQueryStructs(ofile, query);
  }
  fprintf(ofile, "#endif\n");
  fclose(ofile);
  return result;
}

static void doQueryCode(FILE* ofile, PSqlQuery query)
{
  int f;
  bool useInds = false;
  bool hasDynamic = false;
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
}

static int doCode(SqlSO& sqlSO, const char* inName)
{
  int result = COMPLETED_OK;
  char outName[1024];
  SqlSO::makeOutName(outName, sizeof(outName), inName, ".pc", outDirName);
  AutoXdir ax(outName);  
  FILE* ofile = fopen(outName, "wt");
  fprintf(ofile, "#include \"%s.h\"\n", ax.name);
  for (int q = 0; q < sqlSO.noQueries; q++)
  {
    PSqlQuery query = sqlSO.queries[q];
    doQueryCode(ofile, query);
  }
  fclose(ofile);
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
  FILE* inList = fopen(inListName, "rt");
  if (inList == 0)
    return INLIST_FILE_NOT_OPENED;
  while (1)
  {
    if (fgets(buff, sizeof(buff), inList) == 0)
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
  if (inList) fclose(inList);
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
