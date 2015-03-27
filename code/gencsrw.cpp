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
#ifdef M_NT
  #include <io.h>
  #include <sys\stat.h>
#endif
#include "binio.h"

#define US Table->UnderScore ? "_" : ""

static FILE *CSRWFile  = 0;

static char *Dehash(const char *Name)
{
  int i;
  static int n = 0;
  static char Buffer[1024];
  char *Work = Buffer+(n*128);
  n = (n+1)%8;
  strcpy(Work, Name);
  for (i=0; i < (int)strlen(Work); i++)
  {
    if (Work[i] == '#' || Work[i] == '$')
      Work[i] = '_';
  }
  return Work;
}

static char *NameOf(PYYField Field)
{
  return Field->Alias ? Field->Alias : Dehash(Field->Name);
}

static bool IsNullableField(PYYField Field)
{
  if (Field->Type == ftypeImage
  ||  Field->Type == ftypeFloat
  ||  Field->Type == ftypeInt
  ||  Field->Type == ftypeLong
  ||  Field->Type == ftypeBoolean
  ||  Field->Type == ftypeImagePtr
  ||  Field->Type == ftypeBLOB
  ||  Field->Type == ftypeCLOB
  ||  Field->Type == ftypeZLOB
  ||  Field->Type == ftypeXMLTYPE
  ||  Field->Type == ftypeHugeCHAR
  ||  Field->Type == ftypeSmallint
  ||  Field->Type == ftypeTinyint
  ||  Field->Type == ftypeVarNum
  ||  Field->Type == ftypeTimeStamp
  ||  Field->Type == ftypeDate
  ||  Field->Type == ftypeDateTime)
    return true;
  return false;
}

static void CSAsProperty(char *work, const char *type, const char *name)
{
  sprintf(work,
    "public %-8s %-15s {get{return _%s_;}set{_%s_=value;}}%s _%s_;",
    type, name, name, name, type, name
    );
}

static const char *CSFieldType(PYYField Field)
{
  static char work[256];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeDynamic:
    CSAsProperty(work, "string", name);
    return work;
  case ftypeHugeCHAR:
    CSAsProperty(work, "HugeCHAR", name);
    return work;
  case ftypeMoney:
    CSAsProperty(work, "decimal", name);
    return work;
  case ftypeTimeStamp:
  case ftypeDate:
  case ftypeDateTime:
    CSAsProperty(work, "DateTime", name);
    return work;
  case ftypeImage:
    CSAsProperty(work, "SmallImage", name);
    return work;
  case ftypeBoolean:
    CSAsProperty(work, "bool", name);
    return work;
  case ftypeTinyint:
    CSAsProperty(work, "sbyte", name);
    return work;
  case ftypeSmallint:
    CSAsProperty(work, "short", name);
    return work;
  case ftypeInt:
    CSAsProperty(work, "int", name);
    return work;
  case ftypeLong:
    CSAsProperty(work, "long", name);
    return work;
  case ftypeFloat:
    if (Field->Precision > 15)
      CSAsProperty(work, "decimal", name);
    else
      CSAsProperty(work, "double", name);
    return work;
  }
  return "junk";
}

static void CSAsInitialise(char *work, const char *init, const char *name)
{
  sprintf(work,
    "  %-15s = %s;",
    name, init
    );
}

static const char *CSFieldInit(PYYField Field)
{
  static char work[256];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeDynamic:
    CSAsInitialise(work, "\"\"", name);
    return work;
  case ftypeHugeCHAR:
    CSAsInitialise(work, "new HugeCHAR()", name);
    return work;
  case ftypeMoney:
    CSAsInitialise(work, "0.0M", name);
    return work;
  case ftypeTimeStamp:
  case ftypeDate:
  case ftypeDateTime:
    CSAsInitialise(work, "DateTime.Now", name);
    return work;
  case ftypeImage:
    CSAsInitialise(work, "new SmallImage()", name);
    return work;
  case ftypeBoolean:
    CSAsInitialise(work, "false", name);
    return work;
  case ftypeTinyint:
    CSAsInitialise(work, "0", name);
    return work;
  case ftypeSmallint:
    CSAsInitialise(work, "0", name);
    return work;
  case ftypeInt:
    CSAsInitialise(work, "0", name);
    return work;
  case ftypeLong:
    CSAsInitialise(work, "0", name);
    return work;
  case ftypeFloat:
    if (Field->Precision > 15)
      CSAsInitialise(work, "0.0M", name);
    else
      CSAsInitialise(work, "0.0", name);
    return work;
  }
  return "junk";
}

static FILE* OpenFile(const char *FileFlag, const char *FileName)
{
  char Work[512];
  sprintf(Work, "%-13.13s: %s\n", FileFlag, FileName);
  yyerror(Work);
  #ifdef M_NT
  int exists = access(FileName, 0);
  int readonly = access(FileName, 02);
  if (exists == 0 && readonly == -1)
    chmod(FileName, S_IWRITE);
  #endif
  FILE* File = fopen(FileName, "wt");
  if (File == 0)
    yyerror("Cannot open %s : %s\n", FileFlag, FileName);
  else
    setvbuf(File, 0, _IOFBF, 32768);
  return File;
}

static int OpenCSFile(const char *CSFileName)
{
  CSRWFile = OpenFile("CS File", CSFileName);
  if (CSRWFile == 0)
    return 1;
  return 0;
}

static void MakeCSName(PYYTable Table)
{
  char newpath[512];
  char fname[512];
  char dir[512];
  char ext[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->CSRWDir))
    FNameMerge(newpath, Table->CSRWDir, fname, Table->CSRWExt);
  else
    FNameMerge(newpath, dir, fname, Table->CSRWExt);
  Table->CSRWFileName = strdup(newpath);
}

static void filler(int size)
{
  if (size == 0)
    fprintf(CSRWFile, "      ");
  else
    fprintf(CSRWFile, "      writer.Filler(%d);\n      ", size);
}

static void filler(bool eightByte, int length)
{
  int padvalue = 4, pad;
  if (eightByte)
    padvalue = 8;
  if (pad = length % padvalue)
    fprintf(CSRWFile, "      writer.Filler(%d);\n", padvalue - pad);
}

static void writeCall(PYYField field, bool null=false)
{
  char *name = NameOf(field);
  filler(field->Filler);
  switch (field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeDynamic:
    fprintf(CSRWFile, "writer.WriteString(%s, %d);\n", name, field->Length);
    break;
  case ftypeHugeCHAR:
    fprintf(CSRWFile, "%s.Write(writer);\n", name);
    break;
  case ftypeMoney:
    fprintf(CSRWFile, "writer.WriteDecimal(%s, %d);\n", name, field->Length);
    break;
  case ftypeTimeStamp:
  case ftypeDate:
  case ftypeDateTime:
    fprintf(CSRWFile, "writer.WriteDateTime(%s, %d);\n", name, field->Length);
    break;
  case ftypeImage:
    fprintf(CSRWFile, "%s.Write(writer, %d);\n", name, field->Length);
    break;
  case ftypeBoolean:
    fprintf(CSRWFile, "writer.WriteBool(%s);\n", name);
    break;
  case ftypeTinyint:
    fprintf(CSRWFile, "writer.WriteInt8(%s);\n", name);
    break;
  case ftypeSmallint:
    fprintf(CSRWFile, "writer.WriteInt16(%s);\n", name);
    break;
  case ftypeInt:
    fprintf(CSRWFile, "writer.WriteInt32(%s);\n", name);
    break;
  case ftypeLong:
    fprintf(CSRWFile, "writer.WriteInt64(%s);\n", name);
    break;
  case ftypeFloat:
    if (field->Precision > 15)
      fprintf(CSRWFile, "writer.WriteDecimal(%s, %d);\n", name, field->Precision);
    else
      fprintf(CSRWFile, "writer.WriteDouble(%s);\n", name);
    break;
  }
  if (null == true)
    fprintf(CSRWFile, "      writer.WriteBool(%sIsNull);\n", name);
}

static void skip(int size)
{
  if (size == 0)
    fprintf(CSRWFile, "      ");
  else
    fprintf(CSRWFile, "      reader.Skip(%d);\n      ", size);
}

static void skip(bool eightByte, int length)
{
  int padvalue = 4, pad;
  if (eightByte)
    padvalue = 8;
  if (pad = length % padvalue)
    fprintf(CSRWFile, "      reader.Skip(%d);\n", padvalue - pad);
}

static void readCall(PYYField field, bool null=false)
{
  char *name = NameOf(field);
  skip(field->Filler);
  switch (field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeDynamic:
    fprintf(CSRWFile, "%s = reader.ReadString(%d);\n", name, field->Length);
    break;
  case ftypeHugeCHAR:
    fprintf(CSRWFile, "%s.Read(reader);\n", name);
    break;
  case ftypeMoney:
    fprintf(CSRWFile, "%s = reader.ReadDecimal(%d);\n", name, field->Length);
    break;
  case ftypeTimeStamp:
  case ftypeDate:
  case ftypeDateTime:
    fprintf(CSRWFile, "%s = reader.ReadDateTime(%d);\n", name, field->Length);
    break;
  case ftypeImage:
    fprintf(CSRWFile, "%s.Read(reader, %d);\n", name, field->Length);
    break;
  case ftypeBoolean:
    fprintf(CSRWFile, "%s = reader.ReadBool();\n", name);
    break;
  case ftypeTinyint:
    fprintf(CSRWFile, "%s = reader.ReadInt8();\n", name);
    break;
  case ftypeSmallint:
    fprintf(CSRWFile, "%s = reader.ReadInt16();\n", name);
    break;
  case ftypeInt:
    fprintf(CSRWFile, "%s = reader.ReadInt32();\n", name);
    break;
  case ftypeLong:
    fprintf(CSRWFile, "%s = reader.ReadInt64();\n", name);
    break;
  case ftypeFloat:
    if (field->Precision > 15)
      fprintf(CSRWFile, "%s = reader.ReadDecimal(%d);\n", name, field->Precision);
    else
      fprintf(CSRWFile, "%s = reader.ReadDouble();\n", name);
    break;
  }
  if (null == true)
    fprintf(CSRWFile, "      %sIsNull = reader.ReadBool();\n", name);
}

static char *ValueOf(PYYConst Const, char *work)
{
  if (Const->Type == 0)
    sprintf(work, "%d", Const->Value);
  else
    sprintf(work, "'%c'", Const->Value);
  return work;
}

static void GenerateCSTableConsts(PYYTable Table)
{
  int i;
  char work[32];
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts == 0)
      continue;
    fprintf(CSRWFile,
      "  public class %s%sOrd\n"
      "  {\n"
      , Table->Name
      , NameOf(Field)
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSRWFile,
      "    public const int c%s = %s;\n"
      , Const->Name
      , ValueOf(Const, work)
      );
    }
    fprintf(CSRWFile,
      "    public static string ToString(int ordinal)\n"
      "    {\n"
      "      switch (ordinal)\n"
      "      {\n"
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSRWFile,
      "      case c%s: return \"%s\";\n"
      , Const->Name
      , Const->Name
      );
    }
    fprintf(CSRWFile,
      "      }\n"
      "      return \"\";\n"
      "    }\n"
      "  }\n"
      );
  }
}

static void GenerateCSTable(PYYTable Table)
{
  int i;
  bool hasConsts = false;
  fprintf(CSRWFile,
      "  public partial class %sRec\n"
      "  {\n"
      , Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
      hasConsts = true;
    fprintf(CSRWFile,
        "    %s\n", CSFieldType(Field)
        );
    if (Table->isNullEnabled && Field->isNull && IsNullableField(Field))
    {
      char nullName[256];
      sprintf(nullName, "%sIsNull", Field->Name);
      fprintf(CSRWFile,
          "    public bool     %-15s {get{return _%s_;}set{_%s_=value;}}bool _%s_;\n"
          , nullName, nullName, nullName, nullName
          );
    }
  }
  fprintf(CSRWFile,
      "    public %sRec()\n"
      "    {\n"
      , Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(CSRWFile,
        "    %s\n", CSFieldInit(Field)
        );
    if (Table->isNullEnabled && Field->isNull && IsNullableField(Field))
    {
      char nullName[256];
      sprintf(nullName, "%sIsNull", Field->Name);
      fprintf(CSRWFile,
          "      %-15s = false;\n"
          , nullName
          );
    }
  }
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Read(RWReader reader)\n"
      "    {\n"
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    readCall(Field, null);
  }
  skip(Table->EightByte, Table->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Write(RWWriter writer)\n"
      "    {\n"
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    writeCall(Field, null);
  }
  filler(Table->EightByte, Table->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "  }\n"
      );
  if (hasConsts == true)
    GenerateCSTableConsts(Table);
  Table->noBuffers++;
}

static void GenerateCSKey(PYYTable Table, PYYKey Key)
{
  if (Key->noFields == 0)
    return;
  int i;
  fprintf(CSRWFile,
      "  public partial class %sDeleteOneRec\n"
      "  {\n"
      , Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSRWFile,
        "    %s\n", CSFieldType(Field)
        );
  }
  fprintf(CSRWFile,
      "    public %sDeleteOneRec()\n"
      "    {\n"
      , Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSRWFile,
        "    %s\n", CSFieldInit(Field)
        );
    if (Table->isNullEnabled && Field->isNull && IsNullableField(Field))
    {
      char nullName[256];
      sprintf(nullName, "%sIsNull", Field->Name);
      fprintf(CSRWFile,
          "      %-15s = false;\n"
          , nullName
          );
    }
  }
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Read(RWReader reader)\n"
      "    {\n"
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    readCall(Field, null);
  }
  skip(Table->EightByte, Key->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Write(RWWriter writer)\n"
      "    {\n"
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    writeCall(Field, null);
  }
  filler(Table->EightByte, Key->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSRW == 0 && (Proc->extProc & doCSRW) == 0)
    return;
  int i;
  fprintf(CSRWFile,
      "  public partial class %s%sRec\n"
      "  {\n"
      , Dehash(Table->Name), Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(CSRWFile,
        "    %s\n", CSFieldType(Field)
        );
    if (Table->isNullEnabled && Field->isNull && IsNullableField(Field))
    {
      char nullName[256];
      sprintf(nullName, "%sIsNull", Field->Name);
      fprintf(CSRWFile,
          "    public bool     %-15s {get{return _%s_;}set{_%s_=value;}}bool _%s_;\n"
          , nullName, nullName, nullName, nullName
          );
    }
  }
  fprintf(CSRWFile,
      "    public %s%sRec()\n"
      "    {\n"
      , Dehash(Table->Name), Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(CSRWFile,
        "    %s\n", CSFieldInit(Field)
        );
    if (Table->isNullEnabled && Field->isNull && IsNullableField(Field))
    {
      char nullName[256];
      sprintf(nullName, "%sIsNull", Field->Name);
      fprintf(CSRWFile,
          "      %-15s = false;\n"
          , nullName
          );
    }
  }
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Read(RWReader reader)\n"
      "    {\n"
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    readCall(Field, null);
  }
  skip(Table->EightByte, Proc->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "    public void Write(RWWriter writer)\n"
      "    {\n"
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    bool null = Table->isNullEnabled && Field->isNull && IsNullableField(Field);
    writeCall(Field, null);
  }
  filler(Table->EightByte, Proc->Length);
  fprintf(CSRWFile,
      "    }\n"
      );
  fprintf(CSRWFile,
      "  }\n"
      );
  Table->noBuffers++;
}


void GenerateCSRW(PYYTable Table)
{
  MakeCSName(Table);
  ushort i;
  int rc = OpenCSFile(Table->CSRWFileName);
  if (rc)
    return;
  fprintf(CSRWFile,
      "using System;\n"
      "using System.Collections;\n"
      "using System.Data;\n"
      "using bbd.rw;\n"
      "\n"
      "namespace bbd.rw\n"
      "{\n"
      );
  if (Table->noFields > 0)
    GenerateCSTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateCSKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProc(Table, &Table->Procs[i]);
  fprintf(CSRWFile,
      "}\n"
      );
  fclose(CSRWFile);
}



