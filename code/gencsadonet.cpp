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

static FILE *CSAdoNetFile  = 0;

static char *Dehash(char* newName, const char *Name)
{
  int i;
  strcpy(newName, Name);
  for (i=0; i < (int)strlen(newName); i++)
  {
    if (newName[i] == '#' || newName[i] == '$')
      newName[i] = '_';
  }
  return newName;
}

static char *FieldNameOf(char *fieldName, PYYField Field)
{
  if (Field->Alias)
    strcpy(fieldName, Field->Alias);
  else
    Dehash(fieldName, Field->Name);
  return fieldName;
}

static char *TableNameOf(char *tableName, PYYTable Table)
{
  if (Table->Alias)
    strcpy(tableName, Table->Alias);
  else
    Dehash(tableName, Table->Name);
  return tableName;
}

static char* ConvNameOf(char* convName, PYYField Field)
{
  char work[256];
  switch (Field->Type)
  {
    case ftypeDate:
    case ftypeDateTime:
    case ftypeTimeStamp:
      sprintf (convName, "_%s", FieldNameOf(work, Field));
      return convName;
  }
  return FieldNameOf(convName, Field);
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
  case ftypeHugeCHAR:
    return true;
  }
  return false;
}

static void CSAsProperty(char *propertyDef, const char *type, const char *name, bool isNullable)
{
  char ch = isNullable ? '?' : ' ';
  sprintf(propertyDef,
    "public %-8s%c%-24s { get { return _%s; } set{ _%s = value; } } internal %-8s%c_%s;",
    type, ch, name, name, name, type, ch, name
    );
}

static void CSAsProperty(char *propertyDef, const char *type, const char *name, bool isNullable, eFieldType fType)
{
  switch (fType)
  {
    case ftypeDate:
      sprintf(propertyDef,
        "public DateTime %-24s { get { return DateTime.ParseExact(_%s, \"yyyyMMdd\", null); } set { _%s = value.ToString(\"yyyyMMdd\"); } } internal %s _%s;",
        name, name, name, type, name
        );
      return;
    case ftypeDateTime:
    case ftypeTimeStamp:
      sprintf(propertyDef,
        "public DateTime %-24s { get { return DateTime.ParseExact(_%s, \"yyyyMMddHHmmss\", null); } set { _%s = value.ToString(\"yyyyMMddHHmmss\"); } } internal %s _%s;",
        name, name, name, type, name
        );
      return;
  }
  CSAsProperty(propertyDef, type, name, isNullable);
}

static char *CSFieldType(char* fieldType, PYYField Field, bool isNullable)
{
  char name[256];
  FieldNameOf(name, Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    CSAsProperty(fieldType, "string", name, isNullable);
    break;
  case ftypeDate:
    CSAsProperty(fieldType, "string", name, isNullable, Field->Type);
    break;
  case ftypeDateTime:
  case ftypeTimeStamp:
    CSAsProperty(fieldType, "string", name, isNullable, Field->Type);
    break;
  case ftypeXMLTYPE:
    CSAsProperty(fieldType, "string", name, isNullable);
    break;
  case ftypeHugeCHAR:
    CSAsProperty(fieldType, "string", name, isNullable);
    break;
  case ftypeImage:
    CSAsProperty(fieldType, "byte[]", name, isNullable);
    break;
  case ftypeBoolean:
    CSAsProperty(fieldType, "bool", name, isNullable);
    break;
  case ftypeTinyint:
    CSAsProperty(fieldType, "byte", name, isNullable);
    break;
  case ftypeSmallint:
    CSAsProperty(fieldType, "short", name, isNullable);
    break;
  case ftypeInt:
    CSAsProperty(fieldType, "int", name, isNullable);
    break;
  case ftypeLong:
    CSAsProperty(fieldType, "long", name, isNullable);
    break;
  case ftypeFloat:
    CSAsProperty(fieldType, "double", name, isNullable);
    break;
  default:
    CSAsProperty(fieldType, "unknown", name, isNullable);
    break;
  }
  return fieldType;
}

static char *CSFieldTypeOf(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
    return "String";
  case ftypeHugeCHAR:
    return "HugeChar";
  case ftypeXMLTYPE:
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
    return "Image";
  case ftypeBoolean:
    return "Boolean";
  case ftypeTinyint:
    return "Byte";
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

static char *CSCursorTypeOf(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
    return "String";
  case ftypeHugeCHAR:
    return "String";
  case ftypeXMLTYPE:
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
    return "byte[]";
  case ftypeBoolean:
    return "Boolean";
  case ftypeTinyint:
    return "Byte";
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

static char *CSDataTableTypeOf(char *dataType, PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeHugeCHAR:
  case ftypeXMLTYPE:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    strcpy(dataType, "string");
    break;
  case ftypeStatus:
  case ftypeBoolean:
    strcpy(dataType, "bool");
    break;
  case ftypeImage:
    strcpy(dataType, "byte[]");
    break;
  case ftypeTinyint:
    strcpy(dataType, "byte)(short");
    break;
  case ftypeSmallint:
    strcpy(dataType, "short");
    break;
  case ftypeInt:
    strcpy(dataType, "int");
    break;
  case ftypeLong:
    strcpy(dataType, "long");
    break;
  case ftypeFloat:
    strcpy(dataType, "double");
    break;
  default:
    strcpy(dataType, "junk");
    break;
  }
  return dataType;
}

#include <ctype.h>
static char *LowerFirst(char *lowerFirst, const char *name)
{
  strcpy(lowerFirst, name);
  if (lowerFirst[0] >= 'A' || lowerFirst[0] <= 'Z')
    lowerFirst[0] |= ('A' ^ 'a');
  return lowerFirst;
}

static char *CSCastTypeOf(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeHugeCHAR:
  case ftypeXMLTYPE:
    return "string";
  case ftypeStatus:
    return "bool";
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    return "string";
  case ftypeImage:
    return "byte[]";
  case ftypeBoolean:
    return "bool";
  case ftypeTinyint:
    return "byte";
  case ftypeSmallint:
    return "short";
  case ftypeInt:
    return "int";
  case ftypeLong:
    return "long";
  case ftypeFloat:
    return "double";
  }
  return "junk";
}

static char *CSParmTypeOf(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeUserStamp:
  case ftypeHugeCHAR:
  case ftypeXMLTYPE:
    return "string";
  case ftypeStatus:
    return "bool";
  case ftypeMoney:
  case ftypeDynamic:
    return "string";
  case ftypeDate:
  case ftypeDateTime:
  case ftypeTimeStamp:
    return "DateTime";
  case ftypeImage:
    return "byte[]";
  case ftypeBoolean:
    return "bool";
  case ftypeTinyint:
    return "byte";
  case ftypeSmallint:
    return "short";
  case ftypeInt:
    return "int";
  case ftypeLong:
    return "long";
  case ftypeFloat:
    return "double";
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
  char work[256];
  sprintf(work, "%-13.13s: %s\n", FileFlag, FileName);
  yyerror(work);
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
  CSAdoNetFile = OpenFile("CS File", CSFileName);
  if (CSAdoNetFile == 0)
    return 1;
  return 0;
}

static void GenerateCSClassTop(const char *FName, bool isArrayed=false)
{
  fprintf(CSAdoNetFile,
      "  [Serializable()]\n"
      "  public partial class %s\n"
      , FName
      );
  fprintf(CSAdoNetFile,
      "  {\n"
      "    private %sRec rec;\n"
      "    public  %sRec Rec      {get {return rec;} set {rec = value;} }\n"
      , FName, FName
      );
  if (isArrayed == true) 
  {
    fprintf(CSAdoNetFile,
      "    private List<%sRec> recList = new List<%sRec>();\n"
      "    public List<%sRec> RecList { get { return recList; } }\n"
      , FName, FName, FName
      );
    fprintf(CSAdoNetFile,
      "    public int Count { get { return recList.Count; } }\n"
      "    public %sRec this[int i]\n"
      "    {\n"
      "      get\n"
      "      {\n"
      "        return (%sRec) recList[i];\n"
      "      }\n"
      "      set\n"
      "      {\n"
      "        if (i < recList.Count)\n"
      "          recList.RemoveAt(i);\n"
      "        recList.Insert(i, value);\n"
      "      }\n"
      "    }\n"
      , FName, FName
      );
  }
  fprintf(CSAdoNetFile,
      "    public void Clear()\n"
      "    {\n"
      );
  if (isArrayed == true) fprintf(CSAdoNetFile,
      "      recList = new List<%sRec>();\n"
      , FName
      );
  fprintf(CSAdoNetFile,
      "      rec = new %sRec();\n"
      "    }\n"
      "    public %s()\n"
      "    {\n"
      "      Clear();\n"
      "    }\n"
      , FName, FName
      );
}

static bool IsNullField(PYYField Field)
{
  if (Field->Type == ftypeFloat
  ||  Field->Type == ftypeInt
  ||  Field->Type == ftypeLong
  ||  Field->Type == ftypeBoolean
  ||  Field->Type == ftypeSmallint
  ||  Field->Type == ftypeTinyint)
    return true;
  return false;
}

static bool IsNull(PYYTable Table, PYYField Field)
{
  return Table->isNullEnabled && Field->isNull && IsNullField(Field);
}

static void CSNetOutputProcTop(PYYTable Table, PYYProc Proc, const char *Functor, bool Global=false)
{
  int noBinds = 0;
  int i, n;
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput)
      noBinds++;
  }
  char *pad = "";
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSAdoNetFile,
    "    public %s %s(Connect connect)\n"
    "    {\n"
    , Functor, Proc->Name
    );
  fprintf(CSAdoNetFile,
    "      %s%sCursor = new Cursor(connect);\n"
    , Global == true ? "" : "Cursor ", procName
    );
  if (Global == false)
  {
    fprintf(CSAdoNetFile,
      "      try\n"
      "      {\n"
      );
    pad = "  ";
  }
  fprintf(CSAdoNetFile,
      "%s      %sCursor.Format(%sCommand, %d);\n"
      ,pad , procName, procName, noBinds
      );
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0)
      continue;
    if (Field->isOutput == 1)
      continue;
    char work[256], work2[256];
    if (IsNull(Table, Field) == false)
      strcpy(work2, "");
    else
      strcpy(work2, ", true");
    fprintf(CSAdoNetFile,
      "%s      %sCursor.Parameter(%d, rec.%s%s); // %d I:%d O:%d\n"
      ,pad , procName, n++, ConvNameOf(work, Field)
      , work2, i, Field->isInput, Field->isOutput
      );
  }
  int p;
  for (i=0, p=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0)
      continue;
    if (Field->isOutput == 0)
      continue;
    char work[256], work2[256];
    if (IsNull(Table, Field) == false)
      strcpy(work2, "");
    else
      strcpy(work2, ", true");
    fprintf(CSAdoNetFile,
      "%s      IDbDataParameter p%d = %sCursor.Parameter(%d, rec.%s%s); // %d I:%d O:%d\n"
      ,pad ,p ,procName, n++, ConvNameOf(work, Field)
      , work2 ,i, Field->isInput, Field->isOutput
      );
    fprintf(CSAdoNetFile,
      "%s      p%d.Direction = ParameterDirection.Output;\n"
      ,pad ,p
      );
    p++;
  }
}

static void CSWithInputArgs(PYYProc Proc, const char* FName)
{
  int i;
  bool hasInputs = false;
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 1)
      continue;
    hasInputs = true;
    char work[256];
    LowerFirst(work, Field->Name);
    fprintf(CSAdoNetFile,
      "\n"
      "    , %s %s"
      , CSParmTypeOf(Field)
      , work
      );
  }
  if (hasInputs == true)
  {
    fprintf(CSAdoNetFile,
        "\n"
        "    )\n"
        "    {\n"
        "      rec = new %sRec\n"
        , FName
        );
    char comma = '{';
    for (i=0; i<Proc->noFields; i++)
    {
      PYYField Field = &Proc->Fields[i];
      if (Field->isOutput == 1)
        continue;
      char work[256];
      LowerFirst(work, Field->Name);
      fprintf(CSAdoNetFile,
        "      %c %s = %s\n"
        , comma
        , Field->Name
        , work
        );
      comma = ',';
    }
    fprintf(CSAdoNetFile,
        "      };\n"
        );
  }
  else
  {
    fprintf(CSAdoNetFile,
        ")\n"
        "    {\n"
        "      rec = new %sRec();\n"
        , FName
        );
  }
}

static void getter(PYYTable Table, PYYField Field, char* procName, int n, bool &nullDefined, char* pad="")
{
  char work1[256];
  char work2[256];
  work2[0] = 0;
  if (IsNull(Table, Field) == true && nullDefined == false)
  {
    nullDefined = true;
    fprintf(CSAdoNetFile,
      "%s        bool _isNull_;\n"
      , pad
      );
    strcpy(work2, ", out _isNull_");
  }
  fprintf(CSAdoNetFile,
    "%s        rec.%s = (%s)%sCursor.Get%s(%d%s);\n"
    , pad
    , ConvNameOf(work1, Field)
    , CSCastTypeOf(Field)
    , procName
    , CSFieldTypeOf(Field)
    , n
    , work2
   );
  if (IsNull(Table, Field) == true)
  {
    fprintf(CSAdoNetFile,
      "%s        if (_isNull_ == true) rec.%s = null;\n"
      , pad
      , ConvNameOf(work1, Field)
    );
  }
}

static void CSNetOutputProcAfter(PYYTable Table, PYYProc Proc)
{
  int i;
  char procName[128]; LowerFirst(procName, Proc->Name);
  bool nullDefined = false;
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0 || Field->isOutput == 0)
      continue;
    getter(Table, Field, procName, i, nullDefined);
  }
}

static void CSNetNoOutputProcAfter(PYYTable Table, PYYProc Proc)
{
  int i, p;
  char procName[128]; LowerFirst(procName, Proc->Name);
  bool nullDefined = false;
  char work1[256];
  for (i=0, p=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0 || Field->isOutput == 0)
      continue;
    char *pad = "";
    fprintf(CSAdoNetFile,
      "%s        rec.%s = (%s)p%d.Value;\n"
      , pad
      , ConvNameOf(work1, Field)
      , CSCastTypeOf(Field)
      , p++
      );
  }
}

static void CSNetNoOutputProc(PYYTable Table, PYYProc Proc)
{
  CSNetOutputProcTop(Table, Proc, "void");
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSAdoNetFile,
      "        %sCursor.Exec();\n"
      , procName
      );
  CSNetNoOutputProcAfter(Table, Proc);
  fprintf(CSAdoNetFile,
      "      }\n"
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , procName
      );
}

static void CSNetSingleProc(PYYTable Table, PYYProc Proc)
{
  CSNetOutputProcTop(Table, Proc, "bool");
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSAdoNetFile,
      "        %sCursor.Run();\n"
      , procName
      );
  fprintf(CSAdoNetFile,
      "        bool result = (%sCursor.HasReader() && %sCursor.Read());\n"
      "        if (result == true)\n"
      "        {\n"
      , procName
      , procName
      );
  int i, n;
  bool nullDefined = false;
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 0)
      continue;
    getter(Table, Field, procName, n, nullDefined, "  ");n++;
  }
  fprintf(CSAdoNetFile,
      "        }\n"
      "        if (%sCursor.HasReader())\n"
      "          %sCursor.Close();\n"
      "        return result;\n"
      "      }\n"
      , procName
      , procName
      );
  fprintf(CSAdoNetFile,
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , procName
      );
  char FName[256];
  char TableName[256];
  TableNameOf(TableName, Table);
  if ((Proc->useStd > 0) == true)
    sprintf(FName, "%s", TableName);
  else
    sprintf(FName, "%s%s", TableName, Proc->Name);
  fprintf(CSAdoNetFile,
      "    public %sRec %sSingle(Connect connect"
      , FName
      , Proc->Name
      );
  CSWithInputArgs(Proc, FName);
  fprintf(CSAdoNetFile,
      "      if (%s(connect) == true)\n"
      "        return rec;\n"
      "      return null;\n"
      "    }\n"
      , Proc->Name
      );
}

static void CSNetMultiProc(PYYTable Table, PYYProc Proc, const char *FullName)
{
  int i, n;
  bool nullDefined = false;
  CSNetOutputProcTop(Table, Proc, "void", true);
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSAdoNetFile,
      "      %sCursor.Run();\n"
      , procName
      );
  fprintf(CSAdoNetFile,
      "    }\n"
      );
  fprintf(CSAdoNetFile,
      "    public DataTable %sDataTable(Connect connect)\n"
      "    {\n" 
      "      %s(connect);\n"
      "      if (%sCursor.HasReader())\n"
      "      {\n"
      "        DataTable table = new DataTable();\n"
      "        table.Load(%sCursor.Reader);\n"
      "        return table;\n"
      "      }\n"  
      "      else return null;\n"
      "    }\n"
      , Proc->Name
      , Proc->Name
      , procName
      , procName
      );
  fprintf(CSAdoNetFile,
      "    public IEnumerable<%sRec> %sYield(Connect connect)\n"
      "    {\n"
      "      %s(connect);\n"
      "      try\n"
      "      {\n"
      "        if (recList.Count > 0)\n"
      "          Clear();\n"
      "        while (%sFetch())\n"
      "          yield return rec;\n"
      "      }\n"
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , FullName
      , Proc->Name
      , Proc->Name
      , Proc->Name
      , procName
      );
  bool hasInputs = false;
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 1)
      continue;
    hasInputs = true;
    break;
  }
  if (hasInputs == true)
  {
    fprintf(CSAdoNetFile,
        "    public IEnumerable<%sRec> %sYield(Connect connect"
        , FullName
        , Proc->Name
        );
    CSWithInputArgs(Proc, FullName);
    fprintf(CSAdoNetFile,
        "      return %sYield(connect);\n"
        "    }\n"
        , Proc->Name
        );
  }
  fprintf(CSAdoNetFile,
      "    public void Set%sRec(DataTable table, int row)\n"
      "    {\n" 
      "      rec = new %sRec();\n"
      "      DataRowCollection rows = table.Rows;\n"
      "      if (row < rows.Count)\n"
      "      {\n"
      , FullName
      , FullName
      );
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    char work1[256],work2[256],work3[256];
    if (IsNull(Table, Field) == true)
    {
      switch (Field->Type)
      {
      case ftypeFloat:
        strcpy(work2, "getDouble");
        break;
      case ftypeInt:
        strcpy(work2, "GetInt32");
        break;
      case ftypeLong:
        strcpy(work2, "GetInt64");
        break;
      case ftypeBoolean:
        strcpy(work2, "GetBoolean");
        break;
      case ftypeSmallint:
        strcpy(work2, "GetInt16");
        break;
      case ftypeTinyint:
        strcpy(work2, "GetByte");
        break;
      }
      fprintf(CSAdoNetFile,
        "        rec.%s = Nullable.%s(rows[row][\"%s\"]);\n"
        , ConvNameOf(work1, Field)
        , work2
        , ConvNameOf(work3, Field)
        );
    }
    else
    {
      fprintf(CSAdoNetFile,
        "        rec.%s = (%s)rows[row][\"%s\"];\n"
        , ConvNameOf(work1, Field)
        , CSDataTableTypeOf(work2, Field)
        , ConvNameOf(work3, Field)
        );
    }
  }
  fprintf(CSAdoNetFile,
      "      }\n"
      "    }\n"
      );
 fprintf(CSAdoNetFile,
      "    public bool %sFetch()\n"
      "    {\n"
      "      bool result = (%sCursor.HasReader() && %sCursor.Read());\n"
      "      if (result == true)\n"
      "      {\n"
      , Proc->Name
      , procName
      , procName
      );
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 0)
      continue;
    getter(Table, Field, procName, n, nullDefined);n++;
  }
  fprintf(CSAdoNetFile,
      "      }\n"
      "      else if (%sCursor.HasReader())\n"
      "        %sCursor.Dispose();\n"
      "      return result;\n"
      "    }\n"
      , procName
      , procName
      );
  fprintf(CSAdoNetFile,
      "    // Please note - if you use the part functions xxx, xxxFetch, then it is your\n"
      "    //  responsiblity to call this xxxDispose if you do not fetch all the records.\n"
      "    public void %sDispose()\n"
      "    {\n"
      "      %sCursor.Dispose();\n"
      "    }\n"
      , Proc->Name
      , procName
      );
  fprintf(CSAdoNetFile,
      "    public void %sLoad(Connect connect)\n"
      "    {\n"
      "      try\n"
      "      {\n"
      "        if (recList.Count > 0)\n"
      "          Clear();\n"
      "        %s(connect);\n"
      "        while (%sFetch())\n"
      "        {\n"
      "          recList.Add(rec);\n"
      "          rec = new %sRec();\n"
      "        }\n"
      "      }\n"
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , Proc->Name
      , Proc->Name
      , Proc->Name
      , FullName
      , procName
      );
}

static void CSNetNoIOProc(PYYTable Table, const char *ProcName)
{
  char procName[128]; LowerFirst(procName, ProcName);
  fprintf(CSAdoNetFile,
      "    public void %s(Connect connect)\n"
      "    {\n"
      "      using (Cursor %sCursor = new Cursor(connect))\n"
      "      {\n"
      "        %sCursor.Exec();\n"
      "      };\n"
      "    }\n"
      , ProcName
      , procName
      , procName
      );
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
  if (strlen(Table->CSAdoNetDir))
    FNameMerge(newpath, Table->CSAdoNetDir, fname, Table->CSAdoNetExt);
  else
    FNameMerge(newpath, dir, fname, Table->CSAdoNetExt);
  Table->CSAdoNetFileName = strdup(newpath);
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
  char work[256], work2[256];
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts == 0)
      continue;
    fprintf(CSAdoNetFile,
      "  public class %s%sOrd\n"
      "  {\n"
      , TableNameOf(work, Table)
      , FieldNameOf(work2, Field)
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSAdoNetFile,
      "    public const int c%s = %s;\n"
      , Const->Name
      , ValueOf(Const, work)
      );
    }
    fprintf(CSAdoNetFile,
      "    public static string ToString(int ordinal)\n"
      "    {\n"
      "      switch (ordinal)\n"
      "      {\n"
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSAdoNetFile,
      "      case c%s: return \"%s\";\n"
      , Const->Name
      , Const->Name
      );
    }
    fprintf(CSAdoNetFile,
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
  char work[256];
  fprintf(CSAdoNetFile,
      "  [Serializable()]\n"
      "  public partial class %sRec\n"
      "  {\n"
      , TableNameOf(work, Table)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
      hasConsts = true;
    fprintf(CSAdoNetFile,
        "    %s\n"
        , CSFieldType(work, Field, IsNull(Table, Field))
        );
  }
  fprintf(CSAdoNetFile,
      "  }\n"
      );
  if (hasConsts == true)
    GenerateCSTableConsts(Table);
  Table->noBuffers++;
}

static void CSCursorVars(const char *ProcName)
{
  char procName[128]; LowerFirst(procName, ProcName);
  fprintf(CSAdoNetFile,
      "    private Cursor %sCursor;\n"
      , procName
      );
}

struct ALine
{
  char *data;
  int  len;
  uchar isEOL;
  ALine(PYYLine line)
  {
    isEOL = line->isEOL;
    data = strdup(line->p);
    len = strlen(data);
  }
  ~ALine()
  {
    free(data);
  }
};

static char *CSNetDynamicCheck(char *data, char *work)
{
  char *p = data;
  char *q, *r;
  char *result = data;
  work[0] = 0;
  while ((q = strchr(p, '$')) != 0)
  {
    result = work;
    q[0] = 0;q++;
    strcat(work, p);
    strcat(work, "\" + rec.");
    if ((r = strchr(q, ' ')) != 0)
    {
      r[0] = 0;r++;
      strcat(work, q);
      strcat(work, " + \"");
      p = r;
    }
    else
    {
      strcat(work, q);
      strcat(work, " + \"");
      p = q+strlen(q);
    }
  }
  return result;
}

static char *CSNetBindCheck(PYYTable Table, PYYProc Proc, char *data, char *work)
{
  if (data[0] != ':')
    return CSNetDynamicCheck(data, work);
  int i, n;
  for (i=0,n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0)
      continue;
    if (Field->isOutput == 1)
      continue;
    if (stricmp(Field->Name, data+1) == 0)
    {
      sprintf(work, "{%d}", n);
      return work;
    }
    n++;
  }
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0)
      continue;
    if (Field->isOutput == 0)
      continue;
    if (stricmp(Field->Name, data+1) == 0)
    {
      sprintf(work, "{%d}", n);
      return work;
    }
    n++;
  }
  return data;
}

static void CSNetCommand(PYYTable Table, PYYProc Proc)
{
  int i;
  const int start = 1;
  const int end   = 2;
  int state = start;
  char work[256];
  char work2[256];
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSAdoNetFile,
       "    private string %sCommand\n"
       , procName
       );
  const char *plus = "    {\n"
                     "      get\n"
                     "      { return ";
  for (i=0; i<Proc->noLines; i++)
  {
    ALine copy = ALine(&Proc->Lines[i]);
    if (copy.isEOL)
    {
      if (copy.data[copy.len-1] == '\n')
        copy.data[copy.len-1]=0;
      state |= end;
    }
    if (strlen(copy.data) == 0)
      continue;
    strcpy(work2, CSNetBindCheck(Table, Proc, copy.data, work));
    char *cc = strstr(work2, "--");
    if (cc != 0) *cc = 0;
    while ((cc = strstr(work2, "\n")) != 0)
      *cc = ' ';
    if (state & start == start)
      strltrim(work2);
    fprintf(CSAdoNetFile,
       "%s%s%s%s"
       , plus
       , (state & start == start) ? "\"" : ""
       , work2
       , (state & end == end) ? " \"\n" : ""
       );
    plus = "      + ";
    if (state & end == end)
      state = start;
    else
      state = 0;
  }
  fprintf(CSAdoNetFile,
       "      ;\n"
       "      }\n"
       "    }\n"
       );
}

static void GenerateCSTableProcs(PYYTable Table)
{
  bool isArrayed = false;
  int i;
  char TableName[256];
  TableNameOf(TableName, Table);
  if (hasStdRtns(Table))
  {
    if (Table->hasSelectAll)
      if (Table->MarkedCSAdoNet == 0 || Table->extSelectAll & doCSADONET)
        isArrayed = true;
    if (Table->hasSelectAllUpd)
      if (Table->MarkedCSAdoNet == 0 || Table->extSelectAllUpd & doCSADONET)
        isArrayed = true;
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isFetch)
        continue;
      if (Table->TargetCSAdoNet == 0 && (Proc->extProc & doCSADONET) == 0)
        continue;
      if (Proc->useStd && Proc->noOutputs > 0)
        isArrayed = true;
    }
  }
  GenerateCSClassTop(TableName, isArrayed);
  if (hasStdRtns(Table))
  {
    if (Table->hasSelectAll)
      if (Table->MarkedCSAdoNet == 0 || Table->extSelectAll & doCSADONET)
        CSCursorVars("SelectAll");
    if (Table->hasSelectAllUpd)
      if (Table->MarkedCSAdoNet == 0 || Table->extSelectAllUpd & doCSADONET)
        CSCursorVars("SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isFetch)
        continue;
      if (Table->TargetCSAdoNet == 0 && (Proc->extProc & doCSADONET) == 0)
        continue;
      if (Proc->useStd)
        CSCursorVars(Proc->Name);
    }
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData)
        continue;
      if (Table->TargetCSAdoNet == 0 && (Proc->extProc & doCSADONET) == 0)
        continue;
      if (Proc->noFields == 0)
      {
        CSNetCommand(Table, Proc);
        CSNetNoIOProc(Table, Proc->Name);
      }
      else if (Proc->useStd)// || Proc->isStd)
      {
        CSNetCommand(Table, Proc);
        if (Proc->isSql && Proc->isFetch)
          CSNetSingleProc(Table, Proc);
        else if (Proc->isSql && Proc->noOutputs > 0)
          CSNetMultiProc(Table, Proc, TableName);
        else
          CSNetNoOutputProc(Table, Proc);
      }
    }
  }
  fprintf(CSAdoNetFile,
      "  }\n"
      );
}

static void GenerateCSKey(PYYTable Table, PYYKey Key)
{
  if (Key->noFields == 0)
    return;
  int i;
  char work[256];
  fprintf(CSAdoNetFile,
      "  [Serializable()]\n"
      "  public partial class %sDeleteOneRec\n"
      "  {\n"
      , TableNameOf(work, Table)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(CSAdoNetFile,
        "    %s\n", CSFieldType(work, Field, false)
        );
  }
  fprintf(CSAdoNetFile,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSKeyProcs(PYYTable Table, PYYProc Proc, PYYKey Key)
{
  char TableName[256];
  TableNameOf(TableName, Table);
  char work[256];
  if (Key != 0 && Key->noFields == 0)
    return;
  sprintf(work, "%s%s", TableName, Proc->Name);
  GenerateCSClassTop(work, false);
  CSNetCommand(Table, Proc);
  CSNetNoOutputProc(Table, Proc);
  fprintf(CSAdoNetFile,
      "  }\n"
      );
}

static void GenerateCSProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSAdoNet == 0 && (Proc->extProc & doCSADONET) == 0)
    return;
  int i;
  char work[256];
  fprintf(CSAdoNetFile,
      "  [Serializable()]\n"
      "  public partial class %s%sRec\n"
      "  {\n"
      , TableNameOf(work, Table), Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(CSAdoNetFile,
        "    %s\n"
        , CSFieldType(work, Field, IsNull(Table, Field))
        );
  }
  fprintf(CSAdoNetFile,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSProcProcs(PYYTable Table, PYYProc Proc)
{
  char TableName[256];
  TableNameOf(TableName, Table);
  char work[256];
  bool isArrayed = false;
  if (Proc->useKey)
  {
    GenerateCSKeyProcs(Table, Proc, &Table->Keys[Table->PrimaryKey]);
    return;
  }
  if (Proc->noFields == 0 || Proc->useStd)
    return;
  if (Table->TargetCSAdoNet == 0 && (Proc->extProc & doCSADONET) == 0)
    return;
  if (Proc->isSql && !Proc->isFetch && Proc->noOutputs > 0)
    isArrayed = true;
  sprintf(work, "%s%s", TableName, Proc->Name);
  GenerateCSClassTop(work, isArrayed);
  CSNetCommand(Table, Proc);
  if (Proc->isSql && Proc->isFetch)
  {
    CSNetSingleProc(Table, Proc);
  }
  else if (Proc->isSql && Proc->noOutputs > 0)
  {
    CSCursorVars(Proc->Name);
    CSNetMultiProc(Table, Proc, work);
  }
  else
  {
    CSNetNoOutputProc(Table, Proc);
  }
  fprintf(CSAdoNetFile,
      "  }\n"
      );
}

void GenerateCSAdoNet(PYYTable Table)
{
  MakeCSName(Table);
  ushort i;
  int rc = OpenCSFile(Table->CSAdoNetFileName);
  if (rc)
    return;
  fprintf(CSAdoNetFile,
      "using System;\n"
      "using System.Collections.Generic;\n"
      "using System.Data;\n"
      "\n"
      "namespace %s\n"
      "{\n"
      , Table->ControlDB
      );
  if (Table->noFields > 0)
    GenerateCSTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateCSKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProc(Table, &Table->Procs[i]);
  if (Table->noFields > 0)
    GenerateCSTableProcs(Table);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProcProcs(Table, &Table->Procs[i]);
  fprintf(CSAdoNetFile,
      "}\n"
      );
  fclose(CSAdoNetFile);
}



