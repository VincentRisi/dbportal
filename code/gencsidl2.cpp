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
#include "binio.h"

#define US Table->UnderScore ? "_" : ""

static FILE *CSIDL2File  = 0;
static int padValue = 4;

static char *Dehash(const char *Name, char *Work)
{
  int i;
  strcpy(Work, Name);
  for (i=0; i < (int)strlen(Work); i++)
  {
    if (Work[i] == '#' || Work[i] == '$')
      Work[i] = '_';
  }
  return Work;
}

static char *NameOf(PYYField Field, char *Work)
{
  return Field->Alias ? Field->Alias : Dehash(Field->Name, Work);
}

static char *NameOf(PYYTable Table, char *Work)
{
  return Table->Alias ? Table->Alias : Dehash(Table->Name, Work);
}

static bool isString(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeImage:
    return true;
  }
  return false;
}

static const char *CSFieldType(PYYField Field, char* work)
{
  char nwork[128], *name = NameOf(Field, nwork);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeImage:
    sprintf(work, "public string %s {get{return _%s;}set{_%s=value;}}[Field(Size=%d)] string _%s;"
                , name
                , name
                , name
                , Field->Length
                , name);
    return work;
  case ftypeBoolean:
    sprintf(work, "public byte %s {get{return _%s;}set{_%s=value;}} byte _%s;", name, name, name, name);
    return work;
  case ftypeTinyint:
    sprintf(work, "public sbyte %s {get{return _%s;}set{_%s=value;}} sbyte _%s;", name, name, name, name);
    return work;
  case ftypeSmallint:
    sprintf(work, "public short %s {get{return _%s;}set{_%s=value;}} short _%s;", name, name, name, name);
    return work;
  case ftypeInt:
    sprintf(work, "public int %s {get{return _%s;}set{_%s=value;}} int _%s;", name, name, name, name);
    return work;
  case ftypeLong:
    sprintf(work, "public long  %s {get{return _%s;}set{_%s=value;}} long _%s;", name, name, name, name);
    return work;
  case ftypeFloat:
    sprintf(work, "public double %s {get{return _%s;}set{_%s=value;}} double _%s;", name, name, name, name);
    return work;
  }
  return "junk";
}

static bool hasStdRtns(PYYTable Table)
{
  return (bool )(Table->hasInsert
      || Table->hasSelectAll
      || Table->hasSelectAllUpd
      || Table->hasSelectOne
      || Table->hasSelectOneUpd
      || Table->hasDeleteAll
      || Table->hasUpdate);
}

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

static int OpenCSFile(const char *CSFileName)
{
  CSIDL2File = OpenFile("CS File", CSFileName);
  if (CSIDL2File == 0)
    return 1;
  return 0;
}

static void MakeCSName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char fname[512];
  char ext[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->CSIDL2Dir))
    FNameMerge(newpath, Table->CSIDL2Dir, fname, Table->CSIDL2Ext);
  else
    FNameMerge(newpath, dir, fname, Table->CSIDL2Ext);
  Table->CSIDL2FileName = strdup(newpath);
}

static void GenerateCSTable(PYYTable Table)
{
  int i;
  char nwork[128], cswork[128];
  fprintf(CSIDL2File,
      "  [Serializable()]\n"
      "  public class %s%s%s\n"
      "  {\n"
      , Table->LittleTCSIDL2 ? "t" : ""
      , US
      , NameOf(Table, nwork)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Filler)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , Field->Filler
        , i
        , Field->Filler);
    fprintf(CSIDL2File,
        "    %s\n"
        , CSFieldType(Field, cswork)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSIDL2File,
          "    public bool %sIsNull;\n"
          , NameOf(Field, nwork)
          );
  }
  int pad;
  if (pad = Table->Length % padValue)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , padValue - pad
        , i
        , padValue - pad);
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(CSIDL2File,
            "    public enum %sConst\n"
            , NameOf(Field, nwork)
            );
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(CSIDL2File,
            "     %s%s = %d\n"
            , (f == 0) ? "{ " : ", "
            , Const->Name, Const->Value
            );
      }
      fprintf(CSIDL2File,
            "    }\n"
            );
    }
  }
  fprintf(CSIDL2File,
      "  }\n"
      );
  Table->noBuffers++;
}

static void CSCursorVars(const char *ProcName)
{
  fprintf(CSIDL2File,
      "    Cursor %sCursor;\n"
      , ProcName
      );
}

static void GenerateCSKey(PYYTable Table, PYYKey Key)
{
  char nwork[128], cswork[128];
  if (Key->noFields == 0)
    return;
  int i;
  fprintf(CSIDL2File,
      "  [Serializable()]\n"
      "  public class %s%s%sKey\n"
      "  {\n"
      , Table->LittleTCSIDL2 ? "t" : ""
      , US
      , NameOf(Table, nwork)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , Field->Filler
        , i
        , Field->Filler);
    fprintf(CSIDL2File,
        "    %s\n",
        CSFieldType(Field, cswork)
        );
  }
  int pad;
  if (pad = Key->Length % padValue)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , padValue - pad
        , i
        , padValue - pad);
  fprintf(CSIDL2File,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSIDL2 == 0 && (Proc->extProc & doCSIDL2) == 0)
    return;
  int i;
  char nwork[128], cswork[128];
  fprintf(CSIDL2File,
      "  [Serializable()]\n"
      "  public class %s%s%s%s%s\n"
      "  {\n"
      , Table->LittleTCSIDL2 ? "t" : ""
      , US
      , NameOf(Table, nwork)
      , US
      , Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , Field->Filler
        , i
        , Field->Filler);
    fprintf(CSIDL2File,
        "    %s\n"
        , CSFieldType(Field, cswork)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSIDL2File,
          "  bool %sIsNull;\n"
          , NameOf(Field, nwork)
          );
  }
  int pad;
  if (pad = Proc->Length % padValue)
      fprintf(CSIDL2File,
        "    [Field(Size=%d)] byte[] filler%d = new byte[%d];\n"
        , padValue - pad
        , i
        , padValue - pad);
  fprintf(CSIDL2File,
      "  }\n"
      );
  Table->noBuffers++;
}

void GenerateCSIDL2(PYYTable Table)
{
  MakeCSName(Table);
  ushort i;
  if (Table->EightByte)
    padValue = 8;
  int rc = OpenCSFile(Table->CSIDL2FileName);
  if (rc)
    return;
  fprintf(CSIDL2File,
      "using System;\n"
      "using Bbd.Idl2;\n"
      "\n"
      "namespace Bbd.Idl2.DBPortal\n"
      "{\n"
      );
  if (Table->noFields > 0)
    GenerateCSTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateCSKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProc(Table, &Table->Procs[i]);
  fprintf(CSIDL2File,
      "}\n"
      );
  fclose(CSIDL2File);
}


