#include "machine.h"

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>
#include <ctype.h>

#include "sqldef.h"

#include "global.h"
#include "xdir.h"
#include "tbuffer.h"

#define US Table->UnderScore ? "_" : ""

static FILE *PythonFile = 0;
static FILE *CSNet7File = 0;
static TBChar I3Main;
static char I3Iffer[32];

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

enum eFirst {nameAsis, upperFirst, lowerFirst};

static char *NameOf(PYYField Field, char* Work, eFirst first=nameAsis)
{
  strcpy(Work, Field->Alias ? Field->Alias : Dehash(Field->Name));
  switch (first)
  {
  case upperFirst:
    Work[0] = toupper(Work[0]);
    break;
  case lowerFirst:
    Work[0] = tolower(Work[0]);
    break;
  }
  return Work;
}

static char *Upper(const char *Name, char *Work)
{
  strcpy(Work, Name);
  return strupr(Work);
}

static char *Lower(const char *Name, char *Work)
{
  strcpy(Work, Name);
  return strlwr(Work);
}

static const char *TypeOf(PYYField Field)
{
  if (Field->Type == ftypeChar
  ||  Field->Type == ftypeBinary
  ||  Field->Type == ftypeTimeStamp
  ||  Field->Type == ftypeUserStamp
  ||  Field->Type == ftypeStatus
  ||  Field->Type == ftypeMoney
  ||  Field->Type == ftypeDynamic
  ||  Field->Type == ftypeDate
  ||  Field->Type == ftypeDateTime)
    return "%s";
  else if (Field->Type == ftypeFloat)
    return "%f";
//  else if (Field->isSequence == true)
//    return "%s";
  return "%d";
}

static const char *InitOf(PYYField Field)
{
  if (Field->Type == ftypeChar
  ||  Field->Type == ftypeBinary
  ||  Field->Type == ftypeTimeStamp
  ||  Field->Type == ftypeUserStamp
  ||  Field->Type == ftypeStatus
  ||  Field->Type == ftypeMoney
  ||  Field->Type == ftypeDynamic
  ||  Field->Type == ftypeDate
  ||  Field->Type == ftypeDateTime)
    return "''";
  return "0";
}

static char *NameExt(const char *path)
{
  char dir[512];
  char ext[512];
  char fname[512];
  static int n = 0;
  static char Buffer[1024];
  char *Work = Buffer+(n*128);
  n = (n+1)%8;
  FNameSplit(path, dir, fname, ext);
  FNameMerge(Work, "", fname, ext);
  return Work;
}

static int NeedsStdClass(PYYTable Table)
{
  int i;
  if (Table->TargetPython != 0)
    return 1;
  if (Table->Internal == 0)
  {
    if (Table->hasInsert
    ||  Table->hasSelectAll
    ||  Table->hasSelectAllUpd
    ||  Table->hasSelectOne
    ||  Table->hasSelectOneUpd
    ||  Table->hasUpdate)
      return 1;
  }
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->useStd && (Proc->extProc & doPYTHON) != 0)
      return 1;
  }
  return 0;
}

static void GenerateTableClass(PYYTable Table)
{
  int i, f;
  char Work[256];
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(PythonFile, "%s%sConst = {\n", Table->Name, NameOf(Field, Work));
      for (f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(PythonFile, "  '%s' : %d, "
                    , Const->Name
                    , Const->Value
                    );
        fprintf(PythonFile, "%d : '%s',\n"
                    , Const->Value
                    , Const->Name
                    );
      }
      fprintf(PythonFile, "  }\n");
    }
  }
  if (NeedsStdClass(Table) == 0)
    return;
  fprintf(PythonFile, "class DB%s(Proforma):\n", Table->Name);
  fprintf(PythonFile, "  def __init__(self, connect):\n");
  fprintf(PythonFile, "    Proforma.__init__(self, ['_connect', '_query'");
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(PythonFile, ", '%s'", NameOf(Field, Work));
  }
  fprintf(PythonFile, "])\n"
                      "    self._connect = connect\n");
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(PythonFile, "    self.%s = %s\n", NameOf(Field, Work), InitOf(Field));
  }
  fprintf(PythonFile, "  def _store(self, result):\n");
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(PythonFile, "    self.%s = result[%d]\n", NameOf(Field, Work), i);
  }
  fprintf(PythonFile, "  def _data(self):\n"
                      "    return [");
  for (i=0; i<Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(PythonFile, "%sself.%s", i==0?"":", ", NameOf(Field, Work));
  }
  fprintf(PythonFile, "]\n");
}

static void GenerateProcClass(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd)
    return;
  char Work[256];
  int i;
  fprintf(PythonFile, "class DB%s%s(Proforma):\n", Table->Name, Proc->Name);
  fprintf(PythonFile, "  def __init__(self, connect):\n");
  fprintf(PythonFile, "    Proforma.__init__(self, ['_connect', '_query'");
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(PythonFile, ", '%s'", NameOf(Field, Work));
  }
  fprintf(PythonFile, "])\n"
                      "    self._connect = connect\n");
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(PythonFile, "    self.%s = %s\n", NameOf(Field, Work), InitOf(Field));
  }
  fprintf(PythonFile, "  def _store(self, result):\n");
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(PythonFile, "    self.%s = result[%d]\n", NameOf(Field, Work), i);
  }
  fprintf(PythonFile, "  def _data(self):\n"
                      "    return [");
  for (i=0; i<Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(PythonFile, "%sself.%s", i==0?"":", ", NameOf(Field, Work));
  }
  fprintf(PythonFile, "]\n");
}

static void GenerateReadOne(const char *TableName, const char *QueryName, int noFields, PYYField Fields)
{
  int i;
  char Work1[256], Work2[256];
  fprintf(PythonFile, "  def read%s(self", QueryName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
      fprintf(PythonFile, ", %s", NameOf(Field, Work1));
  }
  fprintf(PythonFile, "):\n");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
      fprintf(PythonFile, "    self.%s = %s\n", NameOf(Field, Work1), NameOf(Field, Work2));
  }
  fprintf(PythonFile, "    try:\n");
  fprintf(PythonFile, "      self.exec%s()\n", QueryName);
  fprintf(PythonFile, "      result = 1\n");
  fprintf(PythonFile, "    except DBError, x:\n");
  fprintf(PythonFile, "      if x.ociErr == 1403:\n");
  fprintf(PythonFile, "        result = 0\n");
  fprintf(PythonFile, "      else:\n");
  fprintf(PythonFile, "        raise\n");
  fprintf(PythonFile, "    return result\n");
}

static void GenerateCS7GetStream()
{
  fprintf(CSNet7File, "    public MemoryStream GetStream()\n");
  fprintf(CSNet7File, "    {\n");
  fprintf(CSNet7File, "      MemoryStream memS = new MemoryStream();\n");
  fprintf(CSNet7File, "      XmlSerializer xmlS = new XmlSerializer(this.GetType());\n");
  fprintf(CSNet7File, "      xmlS.Serialize(memS, this);\n");
  fprintf(CSNet7File, "      return memS;\n");
  fprintf(CSNet7File, "    }\n");
}

static void GenerateCS7PutStream(const char *TableName, const char *QueryName)
{
  fprintf(CSNet7File, "    public void PutStream(MemoryStream memS)\n");
  fprintf(CSNet7File, "    {\n");
  fprintf(CSNet7File, "      XmlSerializer xmlS = new XmlSerializer(this.GetType());\n");
  fprintf(CSNet7File, "      this = (%s%sOutput) xmlS.Deserialize(memS);\n"
                    , TableName
                    , QueryName
                    );
  fprintf(CSNet7File, "    }\n");
}

static void GenerateCS7Single(const char *TableName, const char *QueryName, int noFields, PYYField Fields)
{
  int i;
  char Work1[256], Work2[256];
  fprintf(CSNet7File, "  [XmlRoot] public struct %s%sInput : I3InStream\n  {\n"
                    , TableName
                    , QueryName
                    );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
    {
      fprintf(CSNet7File, "    private string m%s;\n"
                        , NameOf(Field, Work1)
                        );
      fprintf(CSNet7File, "    [XmlAttribute] public string %s {get {return m%s;} set {m%s=value;}}\n"
                        , NameOf(Field, Work1)
                        , NameOf(Field, Work2)
                        , NameOf(Field, Work2)
                        );
    }
  }
  GenerateCS7GetStream();
  fprintf(CSNet7File, "  }\n");
  fprintf(CSNet7File, "  [XmlRoot] public struct %s%sOutput : I3InStream, I3OutStream\n  {\n"
                    , TableName
                    , QueryName
                    );
  fprintf(CSNet7File, "    private int mRc;\n"
                      "    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}\n"
                      "    private int mOciErr;\n"
                      "    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}\n"
                      "    private string mErrBuff;\n"
                      "    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}\n"
                    );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput || Field->isSequence)
    {
      fprintf(CSNet7File, "    private string m%s;\n"
                        , NameOf(Field, Work1)
                        );
      fprintf(CSNet7File, "    [XmlAttribute] public string %s {get {return m%s;} set {m%s=value;}}\n"
                        , NameOf(Field, Work1)
                        , NameOf(Field, Work2)
                        , NameOf(Field, Work2)
                        );
    }
  }
  GenerateCS7PutStream(TableName, QueryName);
  GenerateCS7GetStream();
  fprintf(CSNet7File, "  }\n");
}

static void GenerateI3SingleResult(const char *TableName, const char *QueryName, int noFields, PYYField Fields)
{
  int i;
  char Work1[512];
  fprintf(PythonFile, "  return rc, '<%s%sOutput Rc=\"%%d\" OciErr=\"%%d\" ErrBuff=\"%%s\""
                    , TableName
                    , QueryName
                    );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput || Field->isSequence)
      fprintf(PythonFile, " %s=\"%s\""
                        , NameOf(Field, Work1, lowerFirst)
                        , TypeOf(Field)
                        );
  }
  fprintf(PythonFile, "/>\\r\\n' %% (rc, ociErr, xmlEnc(errbuf)");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput)
    {
      if (strcmp(TypeOf(Field), "%s") == 0)
        fprintf(PythonFile, ", xmlEnc(rec.%s)"
                             , NameOf(Field, Work1)
                             );
      else if (strcmp(TypeOf(Field), "%d") == 0)
        fprintf(PythonFile, ", int(rec.%s)"
                          , NameOf(Field, Work1)
                          );
      else if (strcmp(TypeOf(Field), "%f") == 0)
        fprintf(PythonFile, ", float(rec.%s)"
                          , NameOf(Field, Work1)
                          );
    }
    else if (Field->isSequence)
      fprintf(PythonFile, ", int(rec.%s)"
                        , NameOf(Field, Work1)
                        );
  }
  fprintf(PythonFile, ")\n");
}

static void AddToI3Main(const char *TableName, const char *QueryName, int doCommit=0)
{
  char Work1[256];
  I3Main.append("  ");
  I3Main.append(I3Iffer);
  I3Main.append(" '");
  I3Main.append(TableName);
  I3Main.append(QueryName);
  I3Main.append("Input' in msg.keys():\n");
  I3Main.append("    rc, resp = ");
  I3Main.append(Lower(TableName, Work1));
  I3Main.append(QueryName);
  I3Main.append("(msg)\n");
  I3Main.append("    reply(resp)\n");
  if (doCommit == 1)
  {
    I3Main.append("    if rc == 0:\n");
    I3Main.append("      connect.commit()\n");
    I3Main.append("    else:\n");
    I3Main.append("      connect.rollback()\n");
  }
  strcpy(I3Iffer, "elif");
}

static void GenerateI3ReadOne(const char *TableName, const char *QueryName, int noFields, PYYField Fields, int StdRec=0)
{
  int i;
  char Work1[256];
  AddToI3Main(TableName, QueryName);
  fprintf(PythonFile, "def %s%s(msg):\n", Lower(TableName, Work1), QueryName);
  fprintf(PythonFile, "  try:\n");
  fprintf(PythonFile, "    rec = dbc.DB%s%s(connect)\n", TableName, StdRec==1?"":QueryName);
  fprintf(PythonFile, "    has = rec.read%s(\n", QueryName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
      fprintf(PythonFile, "      %s msg['%s%sInput/%s']\n"
                        , i==0?" ":","
                        , TableName
                        , QueryName
                        , NameOf(Field, Work1)
                        );
  }
  fprintf(PythonFile, "      )\n"
                      "    ociErr = 0\n"
                      "    errbuf = ''\n"
                      "    rc = has == 0\n"
                      "  except DBError, x:\n"
                      "    rc = x.rc\n"
                      "    ociErr = x.ociErr\n"
                      "    errbuf = x.value\n"
                    );
  GenerateI3SingleResult(TableName, QueryName, noFields, Fields);
}

static void GenerateRunParms(const char *TableName, const char *QueryName, int noFields, PYYField Fields)
{
  int i;
  char Work1[256], Work2[256];
  fprintf(PythonFile, "  def run%s(self", QueryName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(PythonFile, ", %s", NameOf(Field, Work1));
  }
  fprintf(PythonFile, "):\n");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(PythonFile, "    self.%s = %s\n", NameOf(Field, Work1), NameOf(Field, Work2));
  }
  fprintf(PythonFile, "    self.exec%s()\n", QueryName);
}

static void GenerateI3RunParms(const char *TableName, const char *QueryName, int noFields, PYYField Fields, int StdRec=0)
{
  int i;
  char Work1[256];
  AddToI3Main(TableName, QueryName, 1);
  fprintf(PythonFile, "def %s%s(msg):\n", Lower(TableName, Work1), QueryName);
  fprintf(PythonFile, "  try:\n");
  fprintf(PythonFile, "    rec = dbc.DB%s%s(connect)\n", TableName, StdRec==1?"":QueryName);
  fprintf(PythonFile, "    rec.run%s(\n", QueryName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(PythonFile, "      %s msg['%s%sInput/%s']\n"
                      , i==0?" ":","
                      , TableName
                      , QueryName
                      , NameOf(Field, Work1)
                      );
  }
  fprintf(PythonFile, "      )\n"
                      "    rc = 0\n"
                      "    ociErr = 0\n"
                      "    errbuf = ''\n"
                      "  except DBError, x:\n"
                      "    rc = x.rc\n"
                      "    ociErr = x.ociErr\n"
                      "    errbuf = x.value\n"
                    );
  GenerateI3SingleResult(TableName, QueryName, noFields, Fields);
}

static void GenerateRunStore(const char *TableName, const char *QueryName)
{
  fprintf(PythonFile, "  def exec%s(self):\n", QueryName);
  fprintf(PythonFile, "    result = self._connect.action('%s%s', self._data())\n", TableName, QueryName);
  fprintf(PythonFile, "    self._store(result)\n");
}

static void GenerateRun(const char *TableName, const char *QueryName, int noData=0)
{
  fprintf(PythonFile, "  def run%s(self):\n", QueryName);
  if (noData == 1)
    fprintf(PythonFile, "    self._connect.action('%s%s', [])\n", TableName, QueryName);
  else
    fprintf(PythonFile, "    self._connect.action('%s%s', self._data())\n", TableName, QueryName);
}

static void GenerateCS7Run(const char *TableName, const char *QueryName, int noData=0)
{
}

static void GenerateI3Run(const char *TableName, const char *QueryName, int noData=0, int StdRec=0)
{
  int i;
  char Work1[256];
  AddToI3Main(TableName, QueryName, 1);
  fprintf(PythonFile, "def %s%s(msg):\n", Lower(TableName, Work1), QueryName);
  fprintf(PythonFile, "  try:\n");
  fprintf(PythonFile, "    rec = dbc.DB%s%s(connect)\n", TableName, StdRec==1?"":QueryName);
  fprintf(PythonFile, "    rec.run%s()\n"
                      "    rc = 0\n"
                      "    ociErr = 0\n"
                      "    errbuf = ''\n"
                      "  except DBError, x:\n"
                      "    rc = x.rc\n"
                      "    ociErr = x.ociErr\n"
                      "    errbuf = x.value\n"
                    , QueryName);
  GenerateI3SingleResult(TableName, QueryName, 0, 0);
}

static void GenerateRunFetch(const char *TableName, const char *QueryName, int useQN=0)
{
  fprintf(PythonFile, "  def query%s(self):\n", QueryName);
  fprintf(PythonFile, "    self._query = self._connect.query('%s%s', self._data())\n", TableName, QueryName);
  fprintf(PythonFile, "  def fetch%s(self):\n", QueryName);
  fprintf(PythonFile, "    rc, result = self._connect.fetch(self._query)\n");
  fprintf(PythonFile, "    record = DB%s%s(self._connect)\n", TableName, useQN==0?"":QueryName);
  fprintf(PythonFile, "    if rc == 1:\n");
  fprintf(PythonFile, "      record._store(result)\n");
  fprintf(PythonFile, "    return rc, record\n");
  fprintf(PythonFile, "  def cancel%s(self):\n", QueryName);
  fprintf(PythonFile, "    self._connect.cancel(self._query)\n");
  fprintf(PythonFile, "  def load%s(self):\n", QueryName);
  fprintf(PythonFile, "    self.query%s()\n", QueryName);
  fprintf(PythonFile, "    result = []\n");
  fprintf(PythonFile, "    while 1:\n");
  fprintf(PythonFile, "      rc, rec = self.fetch%s()\n", QueryName);
  fprintf(PythonFile, "      if rc == 0:\n");
  fprintf(PythonFile, "        break\n");
  fprintf(PythonFile, "      result.append(rec)\n");
  fprintf(PythonFile, "    return result\n");
}

static void GenerateCS7RunFetch(const char *TableName, const char *QueryName, int noFields, PYYField Fields)
{
  int i, n;
  char Work1[256], Work2[256];
  fprintf(CSNet7File, "  [XmlRoot] public struct %s%sInput : I3InStream\n  {\n"
                    , TableName
                    , QueryName
                    );
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
    {
      fprintf(CSNet7File, "    private string m%s;\n"
                        , NameOf(Field, Work1)
                        );
      fprintf(CSNet7File, "    [XmlAttribute] public string %s {get {return m%s;} set {m%s=value;}}\n"
                        , NameOf(Field, Work1)
                        , NameOf(Field, Work2)
                        , NameOf(Field, Work2)
                        );
    }
  }
  GenerateCS7GetStream();
  fprintf(CSNet7File, "  }\n");
  fprintf(CSNet7File, "  [XmlRoot] public struct %s%sOutput : I3InStream, I3OutStream\n  {\n"
                    , TableName
                    , QueryName
                    );
  fprintf(CSNet7File, "    public struct ResultList\n");
  fprintf(CSNet7File, "    {\n");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput)
    {
      fprintf(CSNet7File, "      private string m%s;\n"
                        , NameOf(Field, Work1)
                        );
      fprintf(CSNet7File, "      [XmlAttribute] public string %s {get {return m%s;} set {m%s=value;}}\n"
                        , NameOf(Field, Work1)
                        , NameOf(Field, Work2)
                        , NameOf(Field, Work2)
                        );
    }
  }
  fprintf(CSNet7File, "    }\n");
  fprintf(CSNet7File, "    private int mRc;\n"
                      "    [XmlAttribute] public int Rc {get {return mRc;} set {mRc=value;}}\n"
                      "    private int mOciErr;\n"
                      "    [XmlAttribute] public int OciErr {get {return mOciErr;} set {mOciErr=value;}}\n"
                      "    private string mErrBuff;\n"
                      "    [XmlAttribute] public string ErrBuff {get {return mErrBuff;} set {mErrBuff=value;}}\n"
                    );
  fprintf(CSNet7File, "    private int mCount;\n"
                      "    [XmlAttribute] public int Count {get {return mCount;} set {mCount=value;}}\n"
                    );
  fprintf(CSNet7File, "    [XmlElement] public ResultList[] List;\n");
  fprintf(CSNet7File, "    public DataTable GetDataTable()\n");
  fprintf(CSNet7File, "    {\n");
  fprintf(CSNet7File, "      DataTable result = new DataTable();\n");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput)
    {
      fprintf(CSNet7File, "      result.Columns.Add(new DataColumn(\"%s\", typeof(String)));\n"
                        , NameOf(Field, Work1)
                        );
    }
  }
  fprintf(CSNet7File, "      foreach (ResultList L in List)\n");
  fprintf(CSNet7File, "      {\n");
  fprintf(CSNet7File, "        DataRow dr = result.NewRow();\n");
  for (i=0, n=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput)
    {
      fprintf(CSNet7File, "        dr[%d] = L.%s;\n"
                        , n++
                        , NameOf(Field, Work1)
                        );
    }
  }
  fprintf(CSNet7File, "        result.Rows.Add(dr);\n");
  fprintf(CSNet7File, "      }\n");
  fprintf(CSNet7File, "      return result;\n");
  fprintf(CSNet7File, "    }\n");
  GenerateCS7PutStream(TableName, QueryName);
  GenerateCS7GetStream();
  fprintf(CSNet7File, "  }\n");
}

static void GenerateI3RunFetch(const char *TableName, const char *QueryName, int noFields, PYYField Fields, int StdRec=0)
{
  int i;
  char Work1[256], Work2[256];
  AddToI3Main(TableName, QueryName);
  fprintf(PythonFile, "def %s%s(msg):\n", Lower(TableName, Work1), QueryName);
  fprintf(PythonFile, "  try:\n");
  fprintf(PythonFile, "    rec = dbc.DB%s%s(connect)\n", TableName, StdRec==1?"":QueryName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isInput || Field->Type == ftypeDynamic)
      fprintf(PythonFile, "    rec.%s  = msg['%s%sInput/%s']\n"
                        , NameOf(Field, Work1)
                        , TableName
                        , QueryName
                        , NameOf(Field, Work2)
                        );
  }
  fprintf(PythonFile, "    list = rec.load%s()\n"
                      "    ociErr = 0\n"
                      "    errbuf = ''\n"
                      "    rc = len(list)==0\n"
                      "  except DBError, x:\n"
                      "    list = []\n"
                      "    rc = x.rc\n"
                      "    ociErr = x.ociErr\n"
                      "    errbuf = x.value\n"
                    , QueryName);
  fprintf(PythonFile, "  result = '<%s%sOutput Rc=\"%%d\" OciErr=\"%%d\" ErrBuff=\"%%s\" "
                      "Count=\"%%d\">\\r\\n' %% (rc, ociErr, xmlEnc(errbuf), len(list))\n"
                        , TableName
                        , QueryName
                        );
  fprintf(PythonFile, "  for entry in list:\n");
  fprintf(PythonFile, "    result += '<List");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput || StdRec == 1)
      fprintf(PythonFile, " %s=\"%s\""
                        , NameOf(Field, Work1)
                        , TypeOf(Field)
                        );
  }
  fprintf(PythonFile, "/>\\r\\n' %% (");
  int j=0;
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (Field->isOutput || StdRec == 1)
    {
      if (strcmp(TypeOf(Field), "%d") == 0)
        fprintf(PythonFile, "%sint(entry.%s)"
                          , j==0?"":", "
                          , NameOf(Field, Work1)
                          );
      else if (strcmp(TypeOf(Field), "%f") == 0)
        fprintf(PythonFile, "%sfloat(entry.%s)"
                          , j==0?"":", "
                          , NameOf(Field, Work1)
                          );
      else
        fprintf(PythonFile, "%sxmlEnc(entry.%s)"
                          , j==0?"":", "
                          , NameOf(Field, Work1)
                          );
      j++;
    }
  }
  fprintf(PythonFile, ")\n");
  fprintf(PythonFile, "  return rc, result + '</%s%sOutput>\\r\\n'\n"
                        , TableName
                        , QueryName
                        );
}

static void GenerateNonInternalClasses(PYYTable Table)
{
  if (Table->hasDeleteAll)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteAll & doPYTHON)
    {
      fprintf(PythonFile, "class DB%sDeleteAllExec(Proforma):\n", Table->Name);
      fprintf(PythonFile, "  def __init__(self, connect):\n");
      fprintf(PythonFile, "    Proforma.__init__(self, ['_connect'])\n");
      fprintf(PythonFile, "    self._connect = connect\n");
      GenerateRun(Table->Name, "DeleteAll", 1);
      fprintf(PythonFile, "\n");
    }
  }
  if (Table->hasCount)
  {
    if (Table->MarkedPython == 0 || Table->extCount & doPYTHON)
    {
      fprintf(PythonFile, "class DB%sCount(Proforma):\n", Table->Name);
      fprintf(PythonFile, "  def __init__(self, connect):\n");
      fprintf(PythonFile, "    Proforma.__init__(self, ['_connect', 'NoOf'])\n");
      fprintf(PythonFile, "    self._connect = connect\n");
      fprintf(PythonFile, "    self.NoOf = 0\n");
      fprintf(PythonFile, "  def _store(self, result):\n");
      fprintf(PythonFile, "    self.NoOf = result[0]\n");
      fprintf(PythonFile, "  def _data(self):\n");
      fprintf(PythonFile, "    return [self.NoOf]\n");
      GenerateRunStore(Table->Name, "Count");
      fprintf(PythonFile, "\n");
    }
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  PYYKey Key = &Table->Keys[Table->PrimaryKey];
  if (Table->hasDeleteOne)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteOne & doPYTHON)
    {
      fprintf(PythonFile, "class DB%sDeleteOne(Proforma):\n", Table->Name);
      fprintf(PythonFile, "  def __init__(self, connect):\n");
      fprintf(PythonFile, "    Proforma.__init__(self, ['_connect'");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, ", '%s'", NameOf(Field, Work));
      }
      fprintf(PythonFile, "])\n"
                          "    self._connect = connect\n");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, "    self.%s = %s", NameOf(Field, Work), InitOf(Field));
      }
      fprintf(PythonFile, "\n");
      fprintf(PythonFile, "  def _data(self):\n");
      fprintf(PythonFile, "    return [");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, "%sself.%s", i==0?"":", ", NameOf(Field, Work));
      }
      fprintf(PythonFile, "]\n");
      GenerateRun(Table->Name, "DeleteOne");
      fprintf(PythonFile, "\n");
    }
  }
  if (Table->hasExists)
  {
    if (Table->MarkedPython == 0 || Table->extExists & doPYTHON)
    {
      fprintf(PythonFile, "class DB%sExists(Proforma):\n", Table->Name);
      fprintf(PythonFile, "  def __init__(self, connect):\n");
      fprintf(PythonFile, "    Proforma.__init__(self, ['_connect', 'NoOf'");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, ", '%s'", NameOf(Field, Work));
      }
      fprintf(PythonFile, "])\n"
                          "    self._connect = connect\n");
      fprintf(PythonFile, "    self.NoOf = 0\n");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, "    self.%s = %s", NameOf(Field, Work), InitOf(Field));
      }
      fprintf(PythonFile, "\n");
      fprintf(PythonFile, "  def _store(self, result):\n");
      fprintf(PythonFile, "    self.NoOf = result[0]\n");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, "    self.%s = result[%d]", NameOf(Field, Work), i+1);
      }
      fprintf(PythonFile, "\n");
      fprintf(PythonFile, "  def _data(self):\n");
      fprintf(PythonFile, "    return [self.NoOf");
      for (i=0; i < Key->noFields; i++)
      {
        PYYField Field = &Key->Fields[i];
        fprintf(PythonFile, ", self.%s", NameOf(Field, Work));
      }
      fprintf(PythonFile, "]\n");
      GenerateRunStore(Table->Name, "Exists");
      GenerateReadOne(Table->Name, "Exists", Key->noFields, Key->Fields);
      fprintf(PythonFile, "\n");
    }
  }
}

static void GenerateCS7NonInternalClasses(PYYTable Table)
{
  if (Table->hasDeleteAll)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteAll & doPYTHON)
      GenerateCS7Run(Table->Name, "DeleteAll", 1);
  }
  if (Table->hasCount)
  {
    if (Table->MarkedPython == 0 || Table->extCount & doPYTHON)
      GenerateCS7Run(Table->Name, "Count", 1);
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  PYYKey Key = &Table->Keys[Table->PrimaryKey];
  if (Table->hasDeleteOne)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteOne & doPYTHON)
      GenerateCS7Run(Table->Name, "DeleteOne");
  }
  if (Table->hasExists)
  {
    if (Table->MarkedPython == 0 || Table->extExists & doPYTHON)
      GenerateCS7Single(Table->Name, "Exists", Key->noFields, Key->Fields);
  }
}

static void GenerateI3NonInternalClasses(PYYTable Table)
{
  if (Table->hasDeleteAll)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteAll & doPYTHON)
      GenerateI3Run(Table->Name, "DeleteAll", 1, 0);
  }
  if (Table->hasCount)
  {
    if (Table->MarkedPython == 0 || Table->extCount & doPYTHON)
      GenerateI3Run(Table->Name, "Count", 1, 0);
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  PYYKey Key = &Table->Keys[Table->PrimaryKey];
  if (Table->hasDeleteOne)
  {
    if (Table->MarkedPython == 0 || Table->extDeleteOne & doPYTHON)
      GenerateI3Run(Table->Name, "DeleteOne", 0);
  }
  if (Table->hasExists)
  {
    if (Table->MarkedPython == 0 || Table->extExists & doPYTHON)
      GenerateI3ReadOne(Table->Name, "Exists", Key->noFields, Key->Fields);
  }
}

static void GenerateNonInternalStd(PYYTable Table)
{
  if (Table->hasInsert)
  {
    if (Table->MarkedPython == 0 || Table->extInsert & doPYTHON)
    {
      GenerateRunStore(Table->Name, "Insert");
      GenerateRunParms(Table->Name, "Insert", Table->noFields, Table->Fields);
    }
  }
  if (Table->hasSelectAll)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAll & doPYTHON)
      GenerateRunFetch(Table->Name, "SelectAll");
  }
  if (Table->hasSelectAllUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAllUpd & doPYTHON)
      GenerateRunFetch(Table->Name, "SelectAllUpd");
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  if (Table->hasSelectOne)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOne & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateRunStore(Table->Name, "SelectOne");
      GenerateReadOne(Table->Name, "SelectOne", Key->noFields, Key->Fields);
    }
  }
  if (Table->hasSelectOneUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOneUpd & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateRunStore(Table->Name, "SelectOneUpd");
      GenerateReadOne(Table->Name, "SelectOneUpd", Key->noFields, Key->Fields);
    }
  }
  if (Table->hasUpdate)
  {
    if (Table->MarkedPython == 0 || Table->extUpdate & doPYTHON)
    {
      GenerateRunStore(Table->Name, "Update");
      GenerateRunParms(Table->Name, "Update", Table->noFields, Table->Fields);
    }
  }
}

static void GenerateCS7NonInternalStd(PYYTable Table)
{
  if (Table->hasInsert)
  {
    if (Table->MarkedPython == 0 || Table->extInsert & doPYTHON)
      GenerateCS7Single(Table->Name, "Insert", Table->noFields, Table->Fields);
  }
  if (Table->hasSelectAll)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAll & doPYTHON)
      GenerateCS7RunFetch(Table->Name, "SelectAll", Table->noFields, Table->Fields);
  }
  if (Table->hasSelectAllUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAllUpd & doPYTHON)
      GenerateCS7RunFetch(Table->Name, "SelectAllUpd", Table->noFields, Table->Fields);
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  if (Table->hasSelectOne)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOne & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateCS7Single(Table->Name, "SelectOne", Key->noFields, Key->Fields);
    }
  }
  if (Table->hasSelectOneUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOneUpd & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateCS7Single(Table->Name, "SelectOneUpd", Key->noFields, Key->Fields);
    }
  }
  if (Table->hasUpdate)
  {
    if (Table->MarkedPython == 0 || Table->extUpdate & doPYTHON)
      GenerateCS7Single(Table->Name, "Update", Table->noFields, Table->Fields);
  }
}

static void GenerateI3NonInternalStd(PYYTable Table)
{
  if (Table->hasInsert)
  {
    if (Table->MarkedPython == 0 || Table->extInsert & doPYTHON)
      GenerateI3RunParms(Table->Name, "Insert", Table->noFields, Table->Fields, 1);
  }
  if (Table->hasSelectAll)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAll & doPYTHON)
      GenerateI3RunFetch(Table->Name, "SelectAll", Table->noFields, Table->Fields, 1);
  }
  if (Table->hasSelectAllUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectAllUpd & doPYTHON)
      GenerateI3RunFetch(Table->Name, "SelectAllUpd", Table->noFields, Table->Fields, 1);
  }
  if (Table->hasPrimaryKey == 0)
    return;
  int i;
  char Work[256];
  if (Table->hasSelectOne)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOne & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateI3ReadOne(Table->Name, "SelectOne", Key->noFields, Key->Fields, 1);
    }
  }
  if (Table->hasSelectOneUpd)
  {
    if (Table->MarkedPython == 0 || Table->extSelectOneUpd & doPYTHON)
    {
      PYYKey Key = &Table->Keys[Table->PrimaryKey];
      GenerateI3ReadOne(Table->Name, "SelectOneUpd", Key->noFields, Key->Fields, 1);
    }
  }
  if (Table->hasUpdate)
  {
    if (Table->MarkedPython == 0 || Table->extUpdate & doPYTHON)
      GenerateI3RunParms(Table->Name, "Update", Table->noFields, Table->Fields, 1);
  }
}

static void GenerateProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isSql && Proc->isSingle == 0 && Proc->noOutputs > 0)
  {
    GenerateRunFetch(Table->Name, Proc->Name, Proc->useStd ? 0 : 1);
  }
  else if (Proc->noFields > 0)
  {
    GenerateRunStore(Table->Name, Proc->Name);
    if (Proc->noOutputs == 0)
      GenerateRunParms(Table->Name, Proc->Name, Proc->noFields, Proc->Fields);
    else
      GenerateReadOne(Table->Name, Proc->Name, Proc->noFields, Proc->Fields);
  }
  else
    GenerateRun(Table->Name, Proc->Name, 1);
}

static void GenerateCS7Proc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isSql && Proc->isSingle == 0 && Proc->noOutputs > 0)
    GenerateCS7RunFetch(Table->Name, Proc->Name, Proc->useStd ? Table->noFields : Proc->noFields, Proc->useStd ? Table->Fields : Proc->Fields);
  else if (Proc->noFields > 0)
    GenerateCS7Single(Table->Name, Proc->Name, Proc->noFields, Proc->Fields);
  else
    GenerateCS7Run(Table->Name, Proc->Name, 1);
}

static void GenerateI3Proc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isSql && Proc->isSingle == 0 && Proc->noOutputs > 0)
    GenerateI3RunFetch(Table->Name, Proc->Name, Proc->useStd ? Table->noFields : Proc->noFields, Proc->useStd?Table->Fields:Proc->Fields, Proc->useStd?1:0);
  else if (Proc->noFields > 0)
  {
    if (Proc->noOutputs == 0)
      GenerateI3RunParms(Table->Name, Proc->Name, Proc->noFields, Proc->Fields, Proc->useStd?1:0);
    else
      GenerateI3ReadOne(Table->Name, Proc->Name, Proc->noFields, Proc->Fields, Proc->useStd?1:0);
    fprintf(PythonFile, "\n");
  }
  else
  {
    GenerateI3Run(Table->Name, Proc->Name, 1, Proc->useStd?1:0);
    fprintf(PythonFile, "\n");
  }
}

static void GeneratePythonCode(PYYTable Table)
{
  int i;
  if (Table->noFields > 0)
  {
    GenerateTableClass(Table);
    if (Table->Internal == 0)
    {
      GenerateNonInternalStd(Table);
      GenerateNonInternalClasses(Table);
    }
    else
    {
      for (i=0; i<Table->noProcs; i++)
      {
        PYYProc Proc = &Table->Procs[i];
        if (Proc->isData || !Proc->useStd && Proc->noFields > 0)
          continue;
        if (Table->TargetPython == 0
        && (Proc->extProc & doPYTHON) == 0)
          continue;
        GenerateProc(Table, Proc);
      }
    }
    fprintf(PythonFile, "\n");
  }
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData || Proc->useStd || Proc->noFields == 0)
      continue;
    if (Table->TargetPython == 0
    && (Proc->extProc & doPYTHON) == 0)
      continue;
    GenerateProcClass(Table, Proc);
    GenerateProc(Table, Proc);
    fprintf(PythonFile, "\n");
  }
}

static void GenerateI3CSNet7Code(PYYTable Table)
{
  int i;
  if (Table->noFields > 0)
  {
    if (Table->Internal == 0)
    {
      GenerateCS7NonInternalStd(Table);
      GenerateCS7NonInternalClasses(Table);
    }
    else
    {
      for (i=0; i<Table->noProcs; i++)
      {
        PYYProc Proc = &Table->Procs[i];
        if (Proc->isData || !Proc->useStd && Proc->noFields > 0)
          continue;
        if (Table->TargetPython == 0
        && (Proc->extProc & doPYTHON) == 0)
          continue;
        GenerateCS7Proc(Table, Proc);
      }
    }
  }
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData || Proc->useStd || Proc->noFields == 0)
      continue;
    if (Table->TargetPython == 0
    && (Proc->extProc & doPYTHON) == 0)
      continue;
    GenerateCS7Proc(Table, Proc);
  }
}

static void GenerateI3PythonCode(PYYTable Table)
{
  int i;
  if (Table->noFields > 0)
  {
    if (Table->Internal == 0)
    {
      GenerateI3NonInternalStd(Table);
      GenerateI3NonInternalClasses(Table);
    }
    else
    {
      for (i=0; i<Table->noProcs; i++)
      {
        PYYProc Proc = &Table->Procs[i];
        if (Proc->isData || !Proc->useStd && Proc->noFields > 0)
          continue;
        if (Table->TargetPython == 0
        && (Proc->extProc & doPYTHON) == 0)
          continue;
        GenerateI3Proc(Table, Proc);
      }
    }
    fprintf(PythonFile, "\n");
  }
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData || Proc->useStd || Proc->noFields == 0)
      continue;
    if (Table->TargetPython == 0
    && (Proc->extProc & doPYTHON) == 0)
      continue;
    GenerateI3Proc(Table, Proc);
    fprintf(PythonFile, "\n");
  }
}

static FILE* OpenFile(const char *FileFlag, const char *FileName, const char *Mode = "wt")
{
  char Work[512];
  sprintf(Work, "%-13.13s: %s\n", FileFlag, FileName);
  yyerror(Work);
  FILE* File = fopen(FileName, Mode);
  if (File == 0)
    yyerror("Cannot open %s : %s", FileFlag, FileName);
  else
    setvbuf(File, 0, _IOFBF, 32768);
  return File;
}

static char *Upper(PYYTable Table, char *Work)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  FNameSplit(Table->InputFileName, dir, Work, ext);
  Work = strupr(Work);
  return Work;
}

static void MakePythonName(PYYTable Table, const char *Prefix)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  strcpy(fname, Prefix);
  FNameSplit(Table->InputFileName, dir, fname+strlen(Prefix), ext);
  if (strlen(Table->PythonDir))
    FNameMerge(newpath, Table->PythonDir, strlwr(fname), strlwr(Table->PythonExt));
  else
    FNameMerge(newpath, dir, strlwr(fname), strlwr(Table->PythonExt));
  Table->PythonFileName = strdup(newpath);
  if (Table->HeaderFileName == 0)
  {
    if (strlen(Table->CDir))
      FNameMerge(newpath, Table->CDir, fname, Table->CExt);
    else
      FNameMerge(newpath, dir, fname, Table->CExt);
    Table->HeaderFileName = strdup(newpath);
  }
}

static void MakeCSNet7Name(PYYTable Table, const char *Prefix)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  strcpy(fname, Prefix);
  FNameSplit(Table->InputFileName, dir, fname+strlen(Prefix), ext);
  if (strlen(Table->CSNet7Dir))
    FNameMerge(newpath, Table->CSNet7Dir, strlwr(fname), strlwr(Table->CSNet7Ext));
  else
    FNameMerge(newpath, dir, strlwr(fname), strlwr(Table->CSNet7Ext));
  Table->CSNet7FileName = strdup(newpath);
}

void GeneratePython(PYYTable Table)
{
  char Work[512];
  MakePythonName(Table, "db_");
  PythonFile = OpenFile("Python File", Table->PythonFileName);
  if (PythonFile == 0)
    return;
  fprintf(PythonFile, "#use INTRINSICS\n");
  fprintf(PythonFile, "### generated code from DBPortal ###\n\n");
  GeneratePythonCode(Table);
  fclose(PythonFile);
  I3Main.set("def main():\n"
             "  msg = readmessage()\n");
  strcpy(I3Iffer, "if");
  MakePythonName(Table, "i3_");
  MakeCSNet7Name(Table, "i3_");
  PythonFile = OpenFile("Python File", Table->PythonFileName);
  if (PythonFile == 0)
    return;
  fprintf(PythonFile, "#use INTRINSICS\n");
  fprintf(PythonFile, "### generated code from DBPortal ###\n\n");
  fprintf(PythonFile, "def xmlEnc(a): return a.replace('&','&amp;').replace('\"','&quot;').replace(\"'\", '&apos;')\n\n");
  fprintf(PythonFile, "#use DB_%s=dbc\n", Upper(Table, Work));
  GenerateI3PythonCode(Table);
  fprintf(PythonFile, "%s", I3Main.data);
  fclose(PythonFile);
  CSNet7File = OpenFile("CS File", Table->CSNet7FileName);
  if (CSNet7File == 0)
    return;
  fprintf(CSNet7File, "// Generated code from DBPortal - modify at source and not here\n\n");
  fprintf(CSNet7File, "using System;\n");
  fprintf(CSNet7File, "using System.Data;\n");
  fprintf(CSNet7File, "using System.IO;\n");
  fprintf(CSNet7File, "using System.Xml;\n");
  fprintf(CSNet7File, "using System.Xml.Serialization;\n\n");
  fprintf(CSNet7File, "namespace I3Server\n");
  fprintf(CSNet7File, "{\n");
  GenerateI3CSNet7Code(Table);
  fprintf(CSNet7File, "}\n");
  fclose(CSNet7File);
}

