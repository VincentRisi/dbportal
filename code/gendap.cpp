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

static FILE *CSDapFile  = 0;

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

static char* ConvNameOf(char* work, PYYField Field)
{
  switch (Field->Type)
  {
    case ftypeDate:
    case ftypeDateTime:
    case ftypeTimeStamp:
      sprintf (work, "_%s_", NameOf(Field));
      return work;
  }
  return NameOf(Field);
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
  case ftypeHugeCHAR:
  case ftypeXMLTYPE:
    sprintf(work, "%s = \"\";", name);
    break;
  case ftypeImage:
    sprintf(work, "%s = null;", name);
    break;
  default:
    sprintf(work, "%s = 0;", name);
    break;
  }
  return work;
}

static void CSAsProperty(char *work, const char *type, const char *name, bool nullable)
{
  char work2[16];
  sprintf(work2, "%s%s", type, nullable == true ? "?" : "");
  sprintf(work,
    "public %-8s %s {get; set;}",
    work2, name
    );
}

static void CSAsProperty(char *work, const char *type, const char *name, eFieldType fType, bool nullable)
{
  switch (fType)
  {
    case ftypeDate:
      sprintf(work,
        "public DateTime %s {get {return DateTime.ParseExact(_%s_, \"yyyyMMdd\", null);} set {_%s_ = value.ToString(\"yyyyMMdd\");}} internal %s _%s_;",
        name, name, name, type, name
        );
      return;
    case ftypeDateTime:
    case ftypeTimeStamp:
      sprintf(work,
        "public DateTime %s {get {return DateTime.ParseExact(_%s_, \"yyyyMMddHHmmss\", null);} set {_%s_ = value.ToString(\"yyyyMMddHHmmss\");}} internal %s _%s_;",
        name, name, name, type, name
        );
      return;
  }
  CSAsProperty(work, type, name, nullable);
}

static const char *CSFieldType(PYYField Field, bool nullable)
{
  static char work[256];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeBinary:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    CSAsProperty(work, "string", name, nullable);
    return work;
  case ftypeDate:
    CSAsProperty(work, "string", name, Field->Type, nullable);
    return work;
  case ftypeDateTime:
  case ftypeTimeStamp:
    CSAsProperty(work, "string", name, Field->Type, nullable);
    return work;
  case ftypeXMLTYPE:
    CSAsProperty(work, "string", name, nullable);
    return work;
  case ftypeHugeCHAR:
    CSAsProperty(work, "string", name, nullable);
    return work;
  case ftypeImage:
    CSAsProperty(work, "byte[]", name, nullable);
    return work;
  case ftypeBoolean:
    CSAsProperty(work, "byte", name, nullable);
    return work;
  case ftypeTinyint:
    CSAsProperty(work, "sbyte", name, nullable);
    return work;
  case ftypeSmallint:
    CSAsProperty(work, "short", name, nullable);
    return work;
  case ftypeInt:
    CSAsProperty(work, "int", name, nullable);
    return work;
  case ftypeLong:
    CSAsProperty(work, "long", name, nullable);
    return work;
  case ftypeFloat:
    CSAsProperty(work, "double", name, nullable);
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
    return "Int";
  case ftypeTinyint:
    return "Int";
  case ftypeSmallint:
    return "Int";
  case ftypeInt:
    return "Int";
  case ftypeLong:
    return "Int";
  case ftypeFloat:
    return "Double";
  }
  return "junk";
}

static const char *CSCursorTypeOf(PYYField Field)
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

#include <ctype.h>
static char *LowerFirst(char *work, const char *name)
{
  strcpy(work, name);
  if (work[0] >= 'A' || work[0] <= 'Z')
    work[0] |= ('A' ^ 'a');
  return work;
}

static const char *CSCastTypeOf(PYYField Field)
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
    return "sbyte";
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

static int OpenCSFile(const char *FileName)
{
  CSDapFile = OpenFile("CS File", FileName);
  if (CSDapFile == 0)
    return 1;
  return 0;
}

static void GenerateCSClassTop(const char *FName)
{
  fprintf(CSDapFile,
    "  public class %s\n"
    "  {\n"
    , FName
    );
}

static void GenerateCSRepoTop(const char *FName)
{
  fprintf(CSDapFile,
    "  public static class %sRepository\n"
    "  {\n"
    , FName
    );
}

#if 0
    public void Insert(AmountsRec record)
    {
      using (IDbConnection connection = ConnectionFactory.CreateConnection())
      {
        Insert(connection, record);
      }
    }
    public void Insert(IDbConnection connection, AmountsRec rec)
    {
      string command
      = "insert into Amounts ( "
      + "AmtSeqNo "
      ...
      + ") values ( "
      + "AmountsSeq.NEXTVAL "
      + ", "
      + ":BopDataSeqNo "
      + ", "
      ...
      + ") returning "
      + "AmtSeqNo "
      + "into "
      + ":AmtSeqNo "
      ;
      var query = _parameterPrefixFormatter.FormatParameterPrefixes(command, connection);
      DynamicParameters parameters = new DynamicParameters();
      parameters.Add("AmtSeqNo", rec.AmtSeqNo, direction: ParameterDirection.Output);
      parameters.Add("BopDataSeqNo", rec.BopDataSeqNo);
      parameters.Add("RandAmt", rec.RandAmt);
      parameters.Add("Amt", rec.Amt);
      parameters.Add("ExConNo", rec.ExConNo, DbType.String, size: 35);
      ...
      connection.Execute(query, parameters);
      rec.AmtSeqNo = parameters.Get<int>("AmtSeqNo");
    }
    public IEnumerable<AmountsRec> SelectAll()
    {
      using (IDbConnection connection = ConnectionFactory.CreateConnection())
      {
        return SelectAll(connection);
      }
    }
    public IEnumerable<AmountsRec> SelectAll(IDbConnection connection)
    {
      string command
        = "select "
        + "AmtSeqNo "
        + ", BopDataSeqNo "
        ...
        + "from Amounts"
        ;
      var query = _parameterPrefixFormatter.FormatParameterPrefixes(command, connection);
      return connection.Query<AmountsRec>(query);
    }
    public void DeleteOne(int AmtSeqNo)
    {
      using (IDbConnection connection = ConnectionFactory.CreateConnection())
      {
        DeleteOne(connection, AmtSeqNo);
      }
    }
    public void DeleteOne(IDbConnection connection, int aAmtSeqNo)
    {
      string command
      = "delete from Amounts "
      + "where AmtSeqNo =  "
      + ":AmtSeqNo "
      ;
      var query = _parameterPrefixFormatter.FormatParameterPrefixes(command, connection);
      connection.Execute(query, 
        new 
        { 
          AmtSeqNo = aAmtSeqNo
        }
      );
    }
    public bool Exists(int AmtSeqNo)
    {
      using (IDbConnection connection = ConnectionFactory.CreateConnection())
      {
        return Exists(connection, AmtSeqNo);
      }
    }
    public bool Exists(IDbConnection connection, int aAmtSeqNo)
    {
      string command
      = "select count(*) from Amounts "
      + "where AmtSeqNo =  "
      + ":AmtSeqNo "
      ;
      var query = _parameterPrefixFormatter.FormatParameterPrefixes(command, connection);
      return connection.Query<int>(query, 
        new 
        {
          AmtSeqNo = aAmtSeqNo
        }
      ).FirstOrDefault() > 0;
    }
#endif

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
    if (stricmp(Field->Name, data+1) == 0)
    {
      sprintf(work, ":%s", Field->Name);
      return work;
    }
    n++;
  }
  return data;
}

static void CSNetCommand(PYYTable Table, PYYProc Proc)
{
  int i;
  enum {start = 1, end = 2};
  int state = start;
  char work[256];
  char work2[256];
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSDapFile,
       "      string %sCommand\n"
       , procName
       );
  const char *plus = "        = ";
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
    fprintf(CSDapFile,
       "%s%s%s%s"
       , plus
       , (state & start == start) ? "\"" : ""
       , work2
       , (state & end == end) ? " \"\n" : ""
       );
    plus = "        + ";
    if (state & end == end)
      state = start;
    else
      state = 0;
  }
  fprintf(CSDapFile,
       "        ;\n"
       );
}

static void CSNetOutputProcTop(PYYProc Proc, const char *Functor, bool Global=false)
{
  int noBinds = 0;
  int i, n;
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput)
      noBinds++;
  }
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSDapFile,
    "    public %s %s(Connect connect)\n"
    "    {\n"
    , Functor, Proc->Name
    );
  CSNetCommand(Table, Proc);
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0)
      continue;
    char work[256];
    fprintf(CSDapFile,
      "%s      %sCursor.Parameter(%d, rec.%s, %s);\n"
      , procName
      , n++
      , ConvNameOf(work, Field)
      , Field->isOutput ? "true" : "false"
      );
  }
}

static void CSNetOutputProcAfter(PYYProc Proc) //, bool Global=false)
{
  int i;
  char procName[128]; LowerFirst(procName, Proc->Name);
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput == 0 || Field->isOutput == 0)
      continue;
    char work[128];
    fprintf(CSDapFile,
      "      rec.%s = (%s) %sCursor[%d];\n"
      , ConvNameOf(work, Field)
      , CSCastTypeOf(Field)
      , procName
      , i
      );
  }
}

static void CSNetNoOutputProc(PYYProc Proc)
{
  CSNetOutputProcTop(Proc, "void");
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSDapFile,
      "        %sCursor.Exec();\n"
      , procName
      );
  CSNetOutputProcAfter(Proc);
  fprintf(CSDapFile,
      "      }\n"
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , procName
      );
}

static void CSNetSingleProc(PYYProc Proc)
{
  CSNetOutputProcTop(Proc, "bool");
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSDapFile,
      "        %sCursor.Run();\n"
      , procName
      );
  CSNetOutputProcAfter(Proc);
  fprintf(CSDapFile,
      "        bool result = (%sCursor.HasReader() && %sCursor.Read());\n"
      "        if (result == true)\n"
      "        {\n"
      , procName
      , procName
      );
  int i, n;
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 0)
      continue;
    char work[128];
    fprintf(CSDapFile,
      "          rec.%s = (%s)%sCursor.Get%s(%d);\n"
      , ConvNameOf(work, Field)
      , CSCastTypeOf(Field)
      , procName
      , CSFieldTypeOf(Field)
      , n++
      );
  }
  fprintf(CSDapFile,
      "        }\n"
      "        if (%sCursor.HasReader())\n"
      "          %sCursor.Close();\n"
      "        return result;\n"
      "      }\n"
      , procName
      , procName
      );
  fprintf(CSDapFile,
      "      finally\n"
      "      {\n"
      "        %sCursor.Dispose();\n"
      "      }\n"
      "    }\n"
      , procName
      );
}

static void CSNetMultiProc(PYYTable Table, PYYProc Proc, const char *FullName)
{
  CSNetOutputProcTop(Proc, "void", true);
  char procName[128]; LowerFirst(procName, Proc->Name);
  fprintf(CSDapFile,
      "      %sCursor.Run();\n"
      , procName
      );
  CSNetOutputProcAfter(Proc);
  fprintf(CSDapFile,
      "    }\n"
      );
  fprintf(CSDapFile,
      "    public DataTable %sDataTable()\n"
      "    {\n" 
      "      if (%sCursor.HasReader())\n"
      "      {\n"
      "        DataTable table = new DataTable();\n"
      "        table.Load(%sCursor.Reader);\n"
      "        return table;\n"
      "      }\n"  
      "      else return null;\n"
      "    }\n"
      , Proc->Name
      , procName
      , procName
      );
  fprintf(CSDapFile,
      "    public bool %sFetch()\n"
      "    {\n"
      "      bool result = (%sCursor.HasReader() && %sCursor.Read());\n"
      "      if (result == true)\n"
      "      {\n"
      , Proc->Name
      , procName
      , procName
      );
  int i, n;
  for (i=0, n=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isOutput == 0)
      continue;
    char work[128];
    fprintf(CSDapFile,
      "        rec.%s = (%s)%sCursor.Get%s(%d);\n"
      , ConvNameOf(work, Field)
      , CSCastTypeOf(Field)
      , procName
      , CSFieldTypeOf(Field)
      , n++
      );
  }
  fprintf(CSDapFile,
      "      }\n"
      "      else if (%sCursor.HasReader())\n"
      "        %sCursor.Dispose();\n"
      "      return result;\n"
      "    }\n"
      , procName
      , procName
      );
  fprintf(CSDapFile,
      "    // Please note - if you use the part functions xxx, xxxFetch, then it is your\n"
      "    //  responsiblity to call this xxxDispose if you do not fetch all the records.\n"
      "    public void %sDispose()\n"
      "    {\n"
      "      %sCursor.Dispose();\n"
      "    }\n"
      , Proc->Name
      , procName
      );
  fprintf(CSDapFile,
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

static void CSNetNoIOProc(PYYTable Table, PYYProc Proc)
{
  fprintf(CSDapFile,
    "    public void %s(Connect connect)\n"
    "    {\n"
    , Proc->Name
    );
  fprintf(CSDapFile,
    "      // abcd\n"
    "    }\n"
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
  if (strlen(Table->CSDapDir))
    FNameMerge(newpath, Table->CSDapDir, fname, Table->CSDapExt);
  else
    FNameMerge(newpath, dir, fname, Table->CSDapExt);
  Table->CSDapFileName = strdup(newpath);
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
    fprintf(CSDapFile,
      "  public class %s%sOrd\n"
      "  {\n"
      , Table->Name
      , NameOf(Field)
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSDapFile,
      "    public const int c%s = %s;\n"
      , Const->Name
      , ValueOf(Const, work)
      );
    }
    fprintf(CSDapFile,
      "    public static string ToString(int ordinal)\n"
      "    {\n"
      "      switch (ordinal)\n"
      "      {\n"
      );
    for (int f=0; f<Field->noConsts; f++)
    {
      PYYConst Const = &Field->Consts[f];
      fprintf(CSDapFile,
      "      case c%s: return \"%s\";\n"
      , Const->Name
      , Const->Name
      );
    }
    fprintf(CSDapFile,
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
  fprintf(CSDapFile,
      "  [Serializable()]\n"
      "  public partial class %sRec\n"
      "  {\n"
      , Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
      hasConsts = true;
    bool nullable = Table->isNullEnabled && Field->isNull && NullableField(Field);
    fprintf(CSDapFile,
        "    %s\n", CSFieldType(Field, nullable)
        );
  }
  fprintf(CSDapFile,
      "  }\n"
      );
  if (hasConsts == true)
    GenerateCSTableConsts(Table);
  Table->noBuffers++;
}

static void CSCursorVars(const char *ProcName)
{
  char procName[128]; LowerFirst(procName, ProcName);
  fprintf(CSDapFile,
      "    private Cursor %sCursor;\n"
      , procName
      );
}

static void GenerateCSTableProcs(PYYTable Table)
{
  int i;
  //if (hasStdRtns(Table))
  //{
  //  for (i=0; i<Table->noProcs; i++)
  //  {
  //    PYYProc Proc = &Table->Procs[i];
  //    if (Proc->isData || Proc->isSingle)
  //      continue;
  //    if (Table->TargetCSDap == 0 && (Proc->extProc & doCSDAP) == 0)
  //      continue;
  //  }
  //}
  if (hasStdRtns(Table))
  {
    //for (i=0; i<Table->noProcs; i++)
    //{
    //  PYYProc Proc = &Table->Procs[i];
    //  if (Proc->isData || Proc->isStd || Proc->isSingle)
    //    continue;
    //  if (Table->TargetCSDap == 0 && (Proc->extProc & doCSDAP) == 0)
    //    continue;
    //}
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData)
        continue;
      if (Table->TargetCSDap == 0 && (Proc->extProc & doCSDAP) == 0)
        continue;
      if (Proc->noFields == 0)
      {
        CSNetNoIOProc(Table, Proc);
      }
      else if (Proc->useStd)
      {
        if (Proc->isSql && Proc->isSingle)
          CSNetSingleProc(Proc);
        else if (Proc->isSql && Proc->noOutputs > 0)
          CSNetMultiProc(Table, Proc, Table->Name);
        else
          ;//CSNetNoOutputProc(Proc);
      }
    }
  }
}

static void GenerateCSKeyProcs(PYYTable Table, PYYProc Proc, PYYKey Key)
{
  char Work[256];
  if (Key->noFields == 0)
    return;
  sprintf(Work, "%s%s", Table->Name, Proc->Name);
  CSNetNoOutputProc(Proc);
  //fprintf(CSDapFile,
  //    "  }\n"
  //    );
}

static void GenerateCSProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetCSDap == 0 && (Proc->extProc & doCSDAP) == 0)
    return;
  int i;
  bool  hasOutputs = false;
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput != 0 && Field->isOutput == 0)
      continue;
    hasOutputs = true;
    break;
  }
  if (hasOutputs == false)
    return;
  fprintf(CSDapFile,
      "  [Serializable()]\n"
      "  public partial class %s%sRec\n"
      "  {\n"
      , Dehash(Table->Name), Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->isInput != 0 && Field->isOutput == 0)
      continue;
    bool nullable = Table->isNullEnabled && Field->isNull && NullableField(Field);
    fprintf(CSDapFile,
        "    %s\n", CSFieldType(Field, nullable)
        );
  }
  fprintf(CSDapFile,
      "  }\n"
      );
  Table->noBuffers++;
}

static void GenerateCSProcProcs(PYYTable Table, PYYProc Proc)
{
  char Work[256];
  if (Proc->useKey)
  {
    GenerateCSKeyProcs(Table, Proc, &Table->Keys[Table->PrimaryKey]);
    return;
  }
  if (Proc->noFields == 0 || Proc->useStd)
    return;
  if (Table->TargetCSDap == 0 && (Proc->extProc & doCSDAP) == 0)
    return;
  sprintf(Work, "%s%s", Table->Name, Proc->Name);
  if (Proc->isSql && Proc->isSingle)
  {
    CSNetSingleProc(Proc);
  }
  else if (Proc->isSql && Proc->noOutputs > 0)
  {
    CSCursorVars(Proc->Name);
    CSNetMultiProc(Table, Proc, Work);
  }
  else
  {
    CSNetNoOutputProc(Proc);
  }
}

static void GenerateRepo(PYYTable Table)
{
  char *tableName = Dehash(Table->Name);
  fprintf(CSDapFile,
    "  public class %sRepository\n"
    "  {\n"
    "    internal ParameterFormatter parameterFormatter;\n"
    "    internal DbProviderFactory  connectionFactory;\n"
    "    public %sRepository()\n"
    "    {\n"
    "      parameterFormatter = new ParameterFormatter();\n"
    "    }\n"
    "    public %sRepository(DbProviderFactory connectionFactory)\n"
    "    :this()\n"
    "    {\n"
    "      this.connectionFactory = connectionFactory;\n"
    "    }\n"
    "    internal DbProviderFactory ConnectionFactory\n"
    "    {\n"
    "      get\n"
    "      {\n"
    "        if (connectionFactory == null)\n"
    "        {\n"
    "          throw new ArgumentException(\"ConnectionFactory must be specified in the constructor\");\n"
    "        }\n"
    "        return connectionFactory;\n"
    "      }\n"
    "    }\n"
    , tableName
    , tableName
    , tableName
    );
}

void GenerateCSDap(PYYTable Table)
{
  MakeCSName(Table);
  ushort i;
  int rc = OpenCSFile(Table->CSDapFileName);
  if (rc)
    return;
  fprintf(CSDapFile,
    "using System;\n"
    "using System.Linq;\n"
    "using System.Collections;\n"
    "using System.Collections.Generic;\n"
    "using System.Data;\n"
    "using System.Data.Common;\n"
    "using Dapper;\n"
    "/**\n"
    " * Limitations and caveats of Dapper\n"
    " * =================================\n"
    " * Dapper caches information about every query it runs, this allow it to materialize objects quickly \n"
    " * and process parameters quickly. The current implementation caches this information in a ConcurrentDictionary object. \n"
    " * The objects it stores are never flushed. If you are generating SQL strings on the fly without using parameters it is \n"
    " * possible you will hit memory issues. We may convert the dictionaries to an LRU Cache.\n"
    " * \n"
    " * Dapper's simplicity means that many feature that ORMs ship with are stripped out, \n"
    " * there is no identity map, there are no helpers for update / select and so on.\n"
    " *\n"
    " * Dapper does not manage your connection's lifecycle, it assumes the connection it gets is open \n"
    " * AND has no existing datareaders enumerating (unless MARS is enabled) * \n"
    " */\n"
    "\n"
    "namespace Bbd.DBPortal\n"
    "{\n"
    );
  if (Table->noFields > 0)
    GenerateCSTable(Table);
  for (i=0; i<Table->noProcs; i++)
    GenerateCSProc(Table, &Table->Procs[i]);
  GenerateRepo(Table);
  //if (Table->noFields > 0)
  //  GenerateCSTableProcs(Table);
  //for (i=0; i<Table->noProcs; i++)
  //  GenerateCSProcProcs(Table, &Table->Procs[i]);
  fprintf(CSDapFile,
    "  }\n"
    "}\n"
    );
  fclose(CSDapFile);
}



