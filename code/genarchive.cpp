#include "machine.h"

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>

#include "sqlcons.h"
#include "sqldef.h"
#include "global.h"
#include "xdir.h"
#ifndef M_AIX
#include "dir.h"
#endif

static FILE *ArchiveFile = 0;

static FILE* OpenFile(const char *FileFlag, const char *FileName)
{
  char Work[512];
  sprintf(Work, "%-13.13s: %s\n", FileFlag, FileName);
  yyerror(Work);
  FILE* File = fopen(FileName, "wt");
  if (File == 0)
    yyerror("Cannot open %s : %s\n", FileFlag, FileName);
  else
    setvbuf(File, 0, _IOFBF, 32768);
  return File;
}

static int OpenArchiveFile(char *ArchiveFileName)
{
  ArchiveFile = OpenFile("Archive File", ArchiveFileName);
  if (ArchiveFile == 0)
    return 1;
  return 0;
}

static void MakeArchiveName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->ArchiveDir))
    FNameMerge(newpath, Table->ArchiveDir, fname, Table->ArchiveExt);
  else
    FNameMerge(newpath, dir, fname, Table->ArchiveExt);
  Table->ArchiveFileName = strdup(newpath);
}

const char ReservedWords[] =
              ":"
              "all:"
              "ansichar:"
              "application:"
              "boolean:"
              "byte:"
              "cascade:"
              "char:"
              "check:"
              "date:"
              "datetime:"
              "delete:"
              "float:"
              "double:"
              "money:"
              "flags:"
              "int:"
              "integer:"
              "identity:"
              "key:"
              "link:"
              "long:"
              "lookup:"
              "money:"
              "nodomain:"
              "not:"
              "null:"
              "options:"
              "order:"
              "output:"
              "password:"
              "primary:"
              "registry:"
              "relation:"
              "sequence:"
              "server:"
              "short:"
              "show:"
              "smallint:"
              "sizes:"
              "table:"
              "time:"
              "timestamp:"
              "tinyint:"
              "unique:"
              "upper:"
              "uppercase:"
              "use:"
              "user:"
              "userstamp:"
              "value:"
              "viewonly:"
              ;

static char *CheckReserved(char *result, char *name)
{
  char Work[256];
  result[0] = 0;
  sprintf(Work, ":%s:", name);
  strlwr(Work);
  if (strstr(ReservedWords, Work) != 0)
    sprintf(result, "L'%s'", name);
  else
    strcat(result, name);
  return result;
}

static char *NameOf(char *result, PYYField Field)
{
  char CheckWork[256];
  result[0] = 0;
  strcat(result, CheckReserved(CheckWork, Field->Name));
  if (Field->Alias != 0)
  {
    strcat(result, " (");
    strcat(result, CheckReserved(CheckWork, Field->Alias));
    strcat(result, ")");
  }
  return result;
}

static char *TypeOf(char *result, PYYField Field)
{
  strcpy(result, "unhandled");
  switch (Field->Type)
  {
  case ftypeBinary:
    break;
  case ftypeChar:
    sprintf(result, "char(%d)", Field->Length-1);
    break;
  case ftypeTimeStamp:
    strcpy(result, "timestamp");
    break;
  case ftypeUserStamp:
    strcpy(result, "userstamp");
    break;
  case ftypeStatus:
    break;
  case ftypeMoney:
    break;
  case ftypeDynamic:
    break;
  case ftypeImage:
    break;
  case ftypeDate:
    strcpy(result, "date");
    break;
  case ftypeDateTime:
    strcpy(result, "datetime");
    break;
  case ftypeBoolean:
    strcpy(result, "boolean");
    break;
  case ftypeTinyint:
    strcpy(result, "tinyint");
    break;
  case ftypeSmallint:
    strcpy(result, "smallint");
    break;
  case ftypeInt:
    strcpy(result, "int");
    break;
  case ftypeLong:
    strcpy(result, "long");
    break;
  case ftypeFloat:
    strcpy(result, "float");
    break;
  }
  char Work[256];
  char CheckWork[256];
  if (Field->noConsts > 0)
  {
    strcat(result, " ");
    for (int i=0; i<Field->noConsts; i++)
    {
      PYYConst Const = &Field->Consts[i];
      sprintf(Work, "%s%s=%d", i==0?"(":", ", CheckReserved(CheckWork, Const->Name), Const->Value);
      strcat(result, Work);
    }
    strcat(result, ")");
  }
  return result;
}

static int CountOf(PYYTable Table)
{
  int result = Table->noFields, i;
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Type == ftypeTimeStamp
    ||  stricmp(Field->Name, "USId") == 0)
      result--;
  }
  return result;
}

static void GenKey(PYYKey Key)
{
  int i;
  char CheckWork[256];
  fprintf(ArchiveFile, "KEY %s", Key->Name);
  for (i=0; i<Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    CheckReserved(CheckWork, Field->Name);
    fprintf(ArchiveFile, "%s%s", i==0?"(":" ", CheckWork);
  }
  fprintf(ArchiveFile, ")");
  if (Key->isPrimary)
    fprintf(ArchiveFile, "  PRIMARY");
  else if (Key->isUnique)
    fprintf(ArchiveFile, "  UNIQUE");
  fprintf(ArchiveFile, "\n");
}

static void GenKeys(PYYTable Table)
{
  int i;
  for (i=0; i<Table->noKeys; i++)
  {
    PYYKey Key = &Table->Keys[i];
    GenKey(Key);
  }
  fprintf(ArchiveFile, "\n");
}

static void GenTable(PYYTable Table)
{
  int i;
  char NameWork[256];
  char CheckWork[256];
  fprintf(ArchiveFile, "// TABLE %s\n", CheckReserved(CheckWork, Table->Name));
  fprintf(ArchiveFile, "\"SELECT '<%s'\"\n", CheckReserved(CheckWork, Table->Name));
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(ArchiveFile, "\" ||' %s=\\\"'||%s||'\\\"'\"\n"
                     , NameOf(NameWork, Field)
                     , NameOf(NameWork, Field)
                     );
  }
  fprintf(ArchiveFile, "\" ||'/>'\"\n");
  fprintf(ArchiveFile, "\" FROM %s\"\n", CheckReserved(CheckWork, Table->Name));
  PYYKey Key = Table->hasPrimaryKey ? &Table->Keys[Table->PrimaryKey] : 0;
  if (Key != 0)
  {
    for (int i=0; i<Key->noFields; i++)
    {
      PYYField Field = &Key->Fields[i];
      CheckReserved(CheckWork, Field->Name);
      fprintf(ArchiveFile, " \"%s %s=:%s\"\n", i==0?" WHERE":" AND", CheckWork, CheckWork);
    }
  }
  if (Table->noKeys > 0)
    GenKeys(Table);
}

void GenerateArchive(PYYTable Table)
{
  if (Table->noFields == 0)
    return;
  MakeArchiveName(Table);
  int rc = OpenArchiveFile(Table->ArchiveFileName);
  if (rc)
    return;
  GenTable(Table);
  fclose(ArchiveFile);
}


