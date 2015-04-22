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

static FILE *CSNet7File  = 0;

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

static char *CSFieldInit(PYYField Field)
{
  static char work[80];
  char *name = NameOf(Field);
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
    sprintf(work, "%s = \"\";", name);
    break;
  default:
    sprintf(work, "%s = 0;", name);
    break;
  }
  return work;
}

static const char *CSFieldType(PYYField Field, int &Offset, bool UseOffsets)
{
  static char work[80];
  char *name = NameOf(Field);
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
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public string %s;", Offset, name);
      Offset += 4;
    }
    else
      sprintf(work, "public string %s;", name);
    return work;
  case ftypeBoolean:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public byte %s;", Offset, name);
      Offset += 1;
    }
    else
      sprintf(work, "public byte %s;", name);
    return work;
  case ftypeTinyint:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public sbyte %s;", Offset, name);
      Offset += 1;
    }
    else
      sprintf(work, "public sbyte %s;", name);
    return work;
  case ftypeSmallint:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public short %s;", Offset, name);
      Offset += 2;
    }
    else
      sprintf(work, "public short %s;", name);
    return work;
  case ftypeInt:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public int %s;", Offset, name);
      Offset += 4;
    }
    else
      sprintf(work, "public int %s;", name);
    return work;
  case ftypeLong:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public long %s;", Offset, name);
      Offset += 4;
    }
    else
      sprintf(work, "public long %s;", name);
    return work;
  case ftypeFloat:
    if (UseOffsets)
    {
      sprintf(work, "[FieldOffset(%d)] public double %s;", Offset, name);
      Offset += 8;
    }
    else
      sprintf(work, "public double %s;", name);
    return work;
  }
  return "junk";
}

static const char *CSFieldTypeOf(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
    return "String";
  case ftypeBinary:
    return "String";
  case ftypeTimeStamp:
    return "String";
  case ftypeUserStamp:
    return "String";
  case ftypeStatus:
    return "Boolean";
  case ftypeMoney:
    return "String";
  case ftypeDynamic:
    return "String";
  case ftypeDate:
    return "String";
  case ftypeDateTime:
    return "String";
  case ftypeImage:
    return "String";
  case ftypeBoolean:
    return "Boolean";
  case ftypeTinyint:
    return "SByte";
  case ftypeSmallint:
    return "Int16";
  case ftypeInt:
    return "Int32";
  case ftypeLong:
    return "Int64";
  case ftypeFloat:
    return "Double";
  }
  return "junk";
}

static const char *CSSpaceType(PYYField Field)
{
  static char work[80];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeImage:
    sprintf(work, "SqlApi.Pad(%s, %hu)", name, Field->Length);
    return work;
  case ftypeDate:
    sprintf(work, "SqlApi.Pad(%s, 9)", name);
    return work;
  case ftypeDateTime:
    sprintf(work, "SqlApi.Pad(%s, 15)", name);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeInt:
  case ftypeLong:
  case ftypeFloat:
    sprintf(work, "%s", name);
    return work;
  }
  return "junk";
}

static const char *CSMakerType(PYYField Field)
{
  static char work[80];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeImage:
    sprintf(work, "C%hu", Field->Length);
    return work;
  case ftypeDate:
    sprintf(work, "C9");
    return work;
  case ftypeDateTime:
    sprintf(work, "C15");
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "I1");
    return work;
  case ftypeSmallint:
    sprintf(work, "I2");
    return work;
  case ftypeInt:
    sprintf(work, "I4");
    return work;
  case ftypeLong:
    sprintf(work, "I8");
    return work;
  case ftypeFloat:
    sprintf(work, "D8");
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
  CSNet7File = OpenFile("CS File", CSFileName);
  if (CSNet7File == 0)
    return 1;
  return 0;
}

static void GenerateCSClassTop(const char *FName, bool isArrayed=true)
{
  fprintf(CSNet7File,
      "  public class %s\n"
      "  {\n"
      "    private int erc;\n"
      "    private V%s vRec;\n"
      "    private T%s tRec;\n"
      , FName, FName, FName
      );
  if (isArrayed == true)
  {
    fprintf(CSNet7File,
        "    private ArrayList vArray;\n"
        "    public int Count { get { return vArray.Count; } }\n"
        );
  }
}

static void GenerateCSInit(const char *FName, bool isArrayed=false)
{
  fprintf(CSNet7File,
      "    [DllImport(\"SQLAPI32.DLL\", CharSet=CharSet.Ansi)]\n"
      "    public static extern int DBRunProc(int connect, string queryName, ref T%s data);\n"
      , FName
      );
  if (isArrayed == true)
    fprintf(CSNet7File,
      "    [DllImport(\"SQLAPI32.DLL\", CharSet=CharSet.Ansi)]\n"
      "    public static extern int DBOpenExec(int connect, ref int cursor, string queryName, ref T%s data);\n"
      "    [DllImport(\"SQLAPI32.DLL\", CharSet=CharSet.Ansi)]\n"
      "    public static extern int DBFetchData(int cursor, ref T%s data);\n"
      , FName
      , FName
      );
  fprintf(CSNet7File,
      "    public void Clear()\n"
      "    {\n"
      );
  if (isArrayed == true)
    fprintf(CSNet7File,
      "      vArray = new ArrayList();\n"
      );
  fprintf(CSNet7File,
      "      vRec = new V%s();\n"
      "    }\n"
      , FName
      );
  if (isArrayed == true)
    fprintf(CSNet7File,
      "    public V%s this[int i]\n"
      "    {\n"
      "      get\n"
      "      {\n"
      "        if (i < vArray.Count)\n"
      "          return (V%s)vArray[i];\n"
      "        return null;\n"
      "      }\n"
      "      set\n"
      "      {\n"
      "        if (i <= vArray.Count)\n"
      "          vArray.RemoveAt(i);\n"
      "        vArray.Insert(i, value);\n"
      "      }\n"
      "    }\n"
      , FName
      , FName
      );
  fprintf(CSNet7File,
      "    public V%s Rec\n"
      "    {\n"
      "      get\n"
      "      {\n"
      "        return vRec;\n"
      "      }\n"
      "      set\n"
      "      {\n"
      "        vRec = value;\n"
      "      }\n"
      "    }\n"
      , FName
      );
  fprintf(CSNet7File,
      "    public %s()\n"
      "    {\n"
      "      tRec = new T%s();\n"
      "      Clear();\n"
      "    }\n"
      , FName
      , FName
      );
}

static void CSNetNoOutputProc(PYYTable Table, const char *ProcName, const char *RecName, bool doToVRec=true)
{
  fprintf(CSNet7File,
      "    public void %s(Connect connect)\n"
      "    {\n"
      "      Rec.FromV(ref tRec);\n"
      "      erc = DBRunProc(connect.Handle, \"%s%s%s\", ref tRec);\n"
      , ProcName, Dehash(Table->Name), US, ProcName);
  if (doToVRec)
    fprintf(CSNet7File,
      "      Rec.ToV(tRec);\n"
      );
  fprintf(CSNet7File,
      "      if (erc != SqlApi.OK) throw new DBPortalException(erc, \"%s%s%s\", connect.ErrorMsg());\n"
      "    }\n"
      , Dehash(Table->Name), US, ProcName
      );
}

static void CSNetSingleProc(PYYTable Table, const char *ProcName, const char *RecName)
{
  fprintf(CSNet7File,
      "    public bool %s(Connect connect)\n"
      "    {\n"
      "      Rec.FromV(ref tRec);\n"
      "      erc = DBRunProc(connect.Handle, \"%s%s%s\", ref tRec);\n"
      "      Rec.ToV(tRec);\n"
      "      return connect.CheckSingle(erc, \"%s%s%s\");\n"
      "    }\n"
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , Dehash(Table->Name), US, ProcName
      );
}

static void CSNetMultiProc(PYYTable Table, const char *ProcName, const char *RecName, PYYField Fields, int noFields)
{
  int i;
  fprintf(CSNet7File,
      "    public void %s(Connect connect)\n"
      "    {\n"
      "      %sCursor = new Cursor();\n"
      "      int h=0;\n"
      "      Rec.FromV(ref tRec);\n"
      "      erc = DBOpenExec(connect.Handle, ref h, \"%s%s%s\", ref tRec);\n"
      "      %sCursor.Handle = h;\n"
      "      if (erc != SqlApi.OK) throw new DBPortalException(erc, \"%s%s%s\", %sCursor.ErrorMsg());\n"
      "    }\n"
      , ProcName
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , ProcName
      , Dehash(Table->Name), US, ProcName, ProcName
      );
  fprintf(CSNet7File,
      "    public bool %sFetch()\n"
      "    {\n"
      "      if (%sCursor == null)\n"
      "        throw new DBPortalException(0, \"%s%s%sFetch\", \"Run %s first\");\n"
      , ProcName
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , ProcName
      );
  fprintf(CSNet7File,
      "      erc = DBFetchData(%sCursor.Handle, ref tRec);\n"
      "      Rec.ToV(tRec);\n"
      "      return %sCursor.CheckEOF(erc, \"%s%s%s\");\n"
      "    }\n"
      , ProcName
      , ProcName, Dehash(Table->Name), US, ProcName
      );
  fprintf(CSNet7File,
      "    public void %sLoad(Connect connect)\n"
      "    {\n"
      "      %s(connect);\n"
      "      while (%sFetch())\n"
      "        vArray.Add(new V%s(tRec));\n"
      "    }\n"
      "    public ArrayList Loaded { get { return vArray; } }\n"
      , ProcName
      , ProcName
      , ProcName
      , RecName
      );
  fprintf(CSNet7File,
      "    public DataTable %sDataTable()\n"
      "    {\n"
      "      DataTable result = new DataTable();\n"
      , ProcName
      );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(CSNet7File,
      "      result.Columns.Add(new DataColumn(\"%s\", typeof(%s)));\n"
      , Field->Name, CSFieldTypeOf(Field)
      );
  }
  fprintf(CSNet7File,
      "      foreach (V%s v in Loaded)\n"
      "      {\n"
      "        DataRow dr = result.NewRow();\n"
      , RecName
      );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(CSNet7File,
      "        dr[%d] = v.%s;\n"
      , i, Field->Name
      );
  }
  fprintf(CSNet7File,
      "        result.Rows.Add(dr);\n"
      "      }\n"
      "      return result;\n"
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public DataTable %sDataTable(Connect connect)\n"
      "    {\n"
      "      %sLoad(connect);\n"
      "      return %sDataTable();\n"
      "    }\n"
      , ProcName, ProcName, ProcName
      );
}

static void CSNetNoIOProc(PYYTable Table, const char *ProcName)
{
  fprintf(CSNet7File,
      "    public void %s(Connect connect)\n"
      "    {\n"
      "      erc = DBRunProc(connect.Handle, \"%s%s%s\", 0);\n"
      "      if (erc != SqlApi.OK) throw new DBPortalException(erc, \"%s%s%s\", Cursor.ErrorMsg());\n"
      "    }\n"
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , Dehash(Table->Name), US, ProcName
      );
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
  if (strlen(Table->CSNet7Dir))
    FNameMerge(newpath, Table->CSNet7Dir, fname, Table->CSNet7Ext);
  else
    FNameMerge(newpath, dir, fname, Table->CSNet7Ext);
  Table->CSNet7FileName = strdup(newpath);
}

static void GenerateCSTableConsts(PYYTable Table)
{
  int i;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(CSNet7File,
          "    public string %sConstDescr(int index)\n"
          "    {\n"
          "      switch(index)\n"
          "      {\n"
          , NameOf(Field)
          );
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(CSNet7File,
          "      case %s%s: return \"%s\";\n"
          , NameOf(Field), Const->Name, Const->Name
          );
      }
      fprintf(CSNet7File,
          "      }\n"
          "      return \"\";\n"
          "    }\n"
          );
    }
  }
}

static void GenerateCSTable(PYYTable Table)
{
  int i;
  int offset;
  for (i=0, offset=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      offset += 2;
    CSFieldType(Field, offset, true);
  }
  fprintf(CSNet7File,
      "  [StructLayout(LayoutKind.Explicit, Size=%d, CharSet=CharSet.Ansi)]\n"
      "  public struct T%s%s\n"
      "  {\n"
      , offset
      , US, Dehash(Table->Name)
      );
  for (i=0, offset=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(CSNet7File,
        "    %s\n", CSFieldType(Field, offset, true)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
    {
      fprintf(CSNet7File,
          "    [FieldOffset(%d)] public short %sIsNull;\n"
          , offset
          , Field->Name
          );
      offset += 2;
    }
  }
  fprintf(CSNet7File,
      "  }\n"
      );
  fprintf(CSNet7File,
      "  public class V%s%s\n"
      "  {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0, offset=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "    public bool %sIsNull;\n"
          , Field->Name
          );
    fprintf(CSNet7File,
        "    %s\n"
        , CSFieldType(Field, offset, false)
        );
  }
  fprintf(CSNet7File,
      "    public V%s%s()\n"
      "    {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "      %sIsNull = 0;\n"
          , Field->Name
          );
    fprintf(CSNet7File,
        "      %s\n"
        , CSFieldInit(Field)
        );
  }
  fprintf(CSNet7File,
      "    }\n"
      "    public V%s%s(T%s%s tRec)\n"
      "    {\n"
      "      ToV(tRec);\n"
      "    }\n"
      , US, Dehash(Table->Name)
      , US, Dehash(Table->Name)
      );
  fprintf(CSNet7File,
      "    public void ToV(T%s%s tRec)\n"
      "    {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "      if (tRec.%sIsNull == 0) %sIsNull = true; else %sIsNull = false;\n"
          , Field->Name, Field->Name, Field->Name
          );
    if (isString(Field))
      fprintf(CSNet7File,
          "      %s = tRec.%s.TrimEnd();\n"
          , Field->Name, Field->Name
          );
    else
      fprintf(CSNet7File,
          "      %s = tRec.%s;\n"
          , Field->Name, Field->Name
          );
  }
  fprintf(CSNet7File,
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public void FromV(ref T%s%s tRec)\n"
      "    {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "      if (%sIsNull) tRec.%sIsNull = 1; else tRec.%sIsNull = 0;\n"
          , Field->Name, Field->Name, Field->Name
          );
    fprintf(CSNet7File,
        "      tRec.%s = %s;\n"
        , Field->Name, CSSpaceType(Field)
        );
  }
  fprintf(CSNet7File,
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public const string _make_up = \""
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "I2"
          );
    fprintf(CSNet7File,
        "%s"
        , CSMakerType(Field)
        );
  }
  fprintf(CSNet7File,
       "\";\n"
       );
  int hasConsts = 0;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      hasConsts = 1;
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(CSNet7File,
            "    public const int %s%s = %d;\n"
            , NameOf(Field), Const->Name, Const->Value
            );
      }
    }
  }
  if (hasConsts == 1)
    GenerateCSTableConsts(Table);
  fprintf(CSNet7File,
      "  }\n"
      );
  Table->noBuffers++;
}

static void CSCursorVars(const char *ProcName)
{
  fprintf(CSNet7File,
      "    Cursor %sCursor;\n"
      , ProcName
      );
}

static void GenerateCSTableProcs(PYYTable Table)
{
  bool isArrayed = false;
  int i;
  if (hasStdRtns(Table))
  {
    if (Table->hasSelectAll)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAll & doCSNET7)
        isArrayed = true;
    if (Table->hasSelectAllUpd)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAllUpd & doCSNET7)
        isArrayed = true;
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isSingle)
        continue;
      if (Table->TargetCSNet7 == 0 && (Proc->extProc & doCSNET7) == 0)
        continue;
      if (Proc->useStd)
        isArrayed = true;
    }
  }
  GenerateCSClassTop(Table->Name, isArrayed);
  if (hasStdRtns(Table))
  {
    if (Table->hasSelectAll)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAll & doCSNET7)
        CSCursorVars("SelectAll");
    if (Table->hasSelectAllUpd)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAllUpd & doCSNET7)
        CSCursorVars("SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isSingle)
        continue;
      if (Table->TargetCSNet7 == 0 && (Proc->extProc & doCSNET7) == 0)
        continue;
      if (Proc->useStd)
        CSCursorVars(Proc->Name);
    }
    GenerateCSInit(Table->Name, isArrayed);
    if (Table->hasInsert)
      if (Table->MarkedCSNet7 == 0 || Table->extInsert & doCSNET7)
        CSNetNoOutputProc(Table, "Insert", Dehash(Table->Name));
    if (Table->hasUpdate)
      if (Table->MarkedCSNet7 == 0 || Table->extUpdate & doCSNET7)
        CSNetNoOutputProc(Table, "Update", Dehash(Table->Name));
    if (Table->hasDeleteAll)
      if (Table->MarkedCSNet7 == 0 || Table->extDeleteAll & doCSNET7)
        CSNetNoIOProc(Table,     "DeleteAll");
    if (Table->hasSelectOne)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectOne & doCSNET7)
        CSNetSingleProc(Table,   "SelectOne", Dehash(Table->Name));
    if (Table->hasSelectOneUpd)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectOneUpd & doCSNET7)
        CSNetSingleProc(Table,   "SelectOneUpd", Dehash(Table->Name));
    if (Table->hasSelectAll)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAll & doCSNET7)
        CSNetMultiProc(Table,    "SelectAll", Dehash(Table->Name), Table->Fields, Table->noFields);
    if (Table->hasSelectAllUpd)
      if (Table->MarkedCSNet7 == 0 || Table->extSelectAllUpd & doCSNET7)
        CSNetMultiProc(Table,    "SelectAllUpd", Dehash(Table->Name), Table->Fields, Table->noFields);
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd)
        continue;
      if (Table->TargetCSNet7 == 0 && (Proc->extProc & doCSNET7) == 0)
        continue;
      if (Proc->noFields == 0)
        CSNetNoIOProc(Table, Proc->Name);
      else if (Proc->useStd)
      {
        if (Proc->isSql && Proc->isSingle)
          CSNetSingleProc(Table, Proc->Name, Dehash(Table->Name));
        else if (Proc->isSql && Proc->noOutputs > 0)
          CSNetMultiProc(Table, Proc->Name, Dehash(Table->Name), Table->Fields, Table->noFields);
        else
          CSNetNoOutputProc(Table, Proc->Name, Dehash(Table->Name));
      }
    }
  }
  fprintf(CSNet7File,
      "  }\n"
      );
}

static void GenerateCSKey(PYYTable Table, PYYKey Key)
{
  if (Key->noFields == 0)
    return;
  int i;
  int offset;
  for (i=0, offset=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      offset += 2;
    CSFieldType(Field, offset, true);
  }
  fprintf(CSNet7File,
      "  [StructLayout(LayoutKind.Explicit, Size=%d, CharSet=CharSet.Ansi)]\n"
      "  public struct T%s%sKEY\n"
      "  {\n"
      , offset
      , US, Dehash(Table->Name)
      );
  for (i=0, offset=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSNet7File,
        "    %s\n", CSFieldType(Field, offset, true)
        );
  }
  fprintf(CSNet7File,
      "  }\n"
      );
  fprintf(CSNet7File,
      "  public class V%s%sKEY\n"
      "  {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSNet7File,
        "    %s\n",
        CSFieldType(Field, offset, false)
        );
  }
  fprintf(CSNet7File,
      "    public void FromV(ref T%s%sKEY tRec)\n"
      "    {\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSNet7File,
        "    tRec.%s = %s;\n"
        , Field->Name, CSSpaceType(Field)
        );
  }
  fprintf(CSNet7File,
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public const string _make_up = \""
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "I2"
          );
    fprintf(CSNet7File,
        "%s"
        , CSMakerType(Field)
        );
  }
  fprintf(CSNet7File,
       "\";\n"
       );
  fprintf(CSNet7File,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSKeyProcs(PYYTable Table, PYYKey Key)
{
  char Work[256];
  if (Key->noFields == 0)
    return;
  sprintf(Work, "%s%sKEY", Dehash(Table->Name), US);
  GenerateCSClassTop(Work);
  GenerateCSInit(Work);
  CSNetNoOutputProc(Table, "DeleteOne", Work, false);
  fprintf(CSNet7File,
      "  }\n"
      );
}

static void GenerateCSProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSNet7 == 0 && (Proc->extProc & doCSNET7) == 0)
    return;
  int i;
  int offset;
  for (i=0, offset=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      offset += 2;
    CSFieldType(Field, offset, true);
  }
  fprintf(CSNet7File,
      "  [StructLayout(LayoutKind.Explicit, Size=%d, CharSet=CharSet.Ansi)]\n"
      "  public struct T%s%s%s%s\n"
      "  {\n"
      , offset
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0, offset=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(CSNet7File,
        "    %s\n", CSFieldType(Field, offset, true)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "    short %sIsNull;\n"
          , Field->Name
          );
  }
  fprintf(CSNet7File,
      "  }\n"
      );
  fprintf(CSNet7File,
      "  public class V%s%s%s%s\n"
      "  {\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "  bool %sIsNull;\n"
          , Field->Name
          );
    fprintf(CSNet7File,
        "    %s\n"
        , CSFieldType(Field, offset, false)
        );
  }
  fprintf(CSNet7File,
      "    public V%s%s%s%s()\n"
      "    {\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "      %sIsNull = 0;\n"
          , Field->Name
          );
    fprintf(CSNet7File,
        "      %s\n"
        , CSFieldInit(Field)
        );
  }
  fprintf(CSNet7File,
      "    }\n"
      "    public V%s%s%s%s(T%s%s%s%s tRec)\n"
      "    {\n"
      "      ToV(tRec);\n"
      "    }\n"
      , US, Dehash(Table->Name), US, Proc->Name
      , US, Dehash(Table->Name), US, Proc->Name
      );
  fprintf(CSNet7File,
      "    public void ToV(T%s%s%s%s tRec)\n"
      "    {\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "      if (tRec.%sIsNull == 0) %sIsNull = true; else %sIsNull = false;\n"
          , Field->Name, Field->Name, Field->Name
          );
    if (isString(Field))
      fprintf(CSNet7File,
          "      %s = tRec.%s.TrimEnd();\n"
          , Field->Name, Field->Name
          );
    else
      fprintf(CSNet7File,
          "      %s = tRec.%s;\n"
          , Field->Name, Field->Name
          );
  }
  fprintf(CSNet7File,
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public void FromV(ref T%s%s%s%s tRec)\n"
      "    {\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "    if (%sIsNull) tRec.%sIsNull = 1; else tRec.%sIsNull = 0;\n"
          , Field->Name, Field->Name, Field->Name
          );
    fprintf(CSNet7File,
        "    tRec.%s = %s;\n"
        , Field->Name, CSSpaceType(Field)
        );
  }
  fprintf(CSNet7File,
      "    }\n"
      );
  fprintf(CSNet7File,
      "    public const string _make_up = \""
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(CSNet7File,
          "I2"
          );
    fprintf(CSNet7File,
        "%s"
        , CSMakerType(Field)
        );
  }
  fprintf(CSNet7File,
       "\";\n"
       );
  fprintf(CSNet7File,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSProcProcs(PYYTable Table, PYYProc Proc)
{
  int i;
  char Work[256];
  bool isArrayed = false;
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSNet7 == 0 && (Proc->extProc & doCSNET7) == 0)
    return;
  if (Proc->isSql && !Proc->isSingle && Proc->noOutputs > 0)
    isArrayed = true;
  sprintf(Work, "%s%s%s", Dehash(Table->Name), US, Proc->Name);
  GenerateCSClassTop(Work, isArrayed);
  GenerateCSInit(Work, isArrayed);
  if (Proc->isSql && Proc->isSingle)
  {
    CSNetSingleProc(Table, Proc->Name, Work);
  }
  else if (Proc->isSql && Proc->noOutputs > 0)
  {
    CSCursorVars(Proc->Name);
    CSNetMultiProc(Table, Proc->Name, Work, Proc->Fields, Proc->noFields);
  }
  else
  {
    CSNetNoOutputProc(Table, Proc->Name, Work);
  }
  fprintf(CSNet7File,
      "  }\n"
      );
}

void GenerateCSNet7(PYYTable Table)
{
  MakeCSName(Table);
  ushort i;
  int rc = OpenCSFile(Table->CSNet7FileName);
  if (rc)
    return;
  fprintf(CSNet7File,
      "using System;\n"
      "using System.Runtime.InteropServices;\n"
      "using System.Collections;\n"
      "using System.Data;\n"
      "using Bbd.DBPortal;\n"
      "\n"
      "namespace Bbd.DBPortal\n"
      "{\n"
      );
  if (Table->noFields > 0)
    GenerateCSTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateCSKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProc(Table, &Table->Procs[i]);
  if (Table->noFields > 0)
    GenerateCSTableProcs(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateCSKeyProcs(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProcProcs(Table, &Table->Procs[i]);
  fprintf(CSNet7File,
      "}\n"
      );
  fclose(CSNet7File);
}


