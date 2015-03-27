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

#define scc static const char
#define US Table->UnderScore ? "_" : ""

scc CrHTop[]         = "#ifndef _%s_SH_\n"
                       "#define _%s_SH_ %s\n/* %lu */\n\n"
                       "/* Please do not modify, spindle or mutilate. */\n"
                       "#include \"machine.h\"\n"
                       "#pragma pack(1)\n\n";
scc CrHTop1[]        = "#include \"xstring.h\"\n\n";
scc CrHTop2[]        = "// -------------------------------------------------------------\n"
                       "// Be aware that you must have the required includes before this\n"
                       "// -------------------------------------------------------------\n"
                       "#if !defined(__cplusplus)\n"
                       "#error This header is generated for a c++ compile.\n"
                       "#endif\n"
                       "#include \"dbportal.h\"\n"
                       "#include \"xstring.h\"\n\n";
scc CrHStruct0[]     = "%sstruct t%s%s\n{\n";
scc CrHStructT[]     = "%sstruct t%s%s%s%s\n{\n";
scc CrHStruct1[]     = "  %s\n";
scc CrHStructN[]     = "  int16  %sIsNull;\n";
scc CrHStructF[]     = "  char   filler%d[%d];\n";
scc CrHStruct2[]     = "} t%s%s, *p%s%s;\n\n";
scc CrHStruct3[]     = "} t%s%s%s%s, *p%s%s%s%s;\n\n";
scc CrHStruct4[]     = "} t%s%sKey, *p%s%sKey;\n\n";
scc CrHBottom[]      = "#ifdef M_AIX\n"
                       "#pragma pack(pop)\n"
                       "#else\n"
                       "#pragma pack()\n"
                       "#endif\n"
                       "#endif\n\n";
scc CrVBTop[]        = "'%s \n"
                       "' Please do not modify, spindle or mutilate.\n"
                       "Option Explicit\n\n";
scc CrVBStart[]      = "Type t%s%s\n";
scc CrVBStartKey[]   = "Type t%s%sKEY\n";
scc CrVBStartProc[]  = "Type t%s%s%s%s\n";
scc CrVBField[]      = "  %s\n";
scc CrVBFiller[]     = "  Filler%d As String * %d\n";
scc CrVBEnd[]        = "End Type\n\n";
scc CrVBProc0[]      = "Dim Erc As Integer\n";
scc CrVBProc1[]      = "Function %s%s (aSession As Long"; //  1 = Table Name, 2 = Action eg DeleteOne
scc CrVBProc2a[]     = ", a%s";                           // 3 = field Name
scc CrVBProc2b[]     = ", a%sRec As t%s";                 // 1 1
scc CrVBProc2c[]     = ", a%s%sRec As t%s%s";             // 1 2 1 2
scc CrVBProc3a[]     = ") As Integer\n";
scc CrVBProc3b[]     = ") As Long\n";
scc CrVBProc4a[]     = "  Dim %sKeyRec As t%sKey\n";      // 1 1
scc CrVBProc4b[]     = "  Dim %s%sRec As t%s%s\n";        // 1 2 1 2
scc CrVBProc4c[]     = "  Dim %s%sRec As Long\n";         // 1 2
scc CrVBProc5a[]     = "  %sKeyRec.%s = a%s\n";           // 1 3 3
scc CrVBProc5b[]     = "  a%sRec.%s = a%s\n";             // 1 3 3
scc CrVBProc5c[]     = "  %s%sRec.%s = a%s\n";            // 1 2 3 3
scc CrVBProc6a[]     = "  Erc = DBRunProc(aSession, \"%s%s\", %sKeyRec)\n"; // 1 2 1
scc CrVBProc6b[]     = "  Erc = DBRunProc(aSession, \"%s%s\", a%sRec)\n";   // 1 2 1
scc CrVBProc6c[]     = "  Erc = DBRunProc(aSession, \"%s%s\", a%s%sRec)\n"; // 1 2 1 2
scc CrVBProc6d[]     = "  Erc = DBRunProc(aSession, \"%s%s\", 0)\n";        // 1 2
scc CrVBProc6e[]     = "  Erc = DBRunProc(aSession, \"%s%s\", %s%sRec)\n"; // 1 2 1 2
scc CrVBProc7a[]     = "  %s%s = Erc\n";                  // 1 2
scc CrVBProc7b[]     = "  %s%s = %s%sRec.Count\n";        // 1 2 1 2
scc CrVBProc7c[]     = "  %s%s = %s%sRec\n";              // 1 2 1 2
scc CrVBProc8[]      = "  If Erc <> SQLapiOK Then %s%s = 0 - Erc\n";            // 1 2
scc CrVBProc9[]      = "  If Erc = SQLapiNoMoreRows Then Exit Function\n";
scc CrVBProc10[]     = "  DBVBErrorCheck aSession, Erc, \"%s %s\"\n"
                       "End Function\n\n"; // 1 2
scc CrPASTop[]       = "{%s%s}\n"
                       "{ Please do not modify, spindle or mutilate. }\n";
scc CrPASCode1a[]    = "unit %s%s;\n"                 // 1=shortname
                       "interface\n"
                       "uses sysutils, psqlapi, p%sapi;\n"    // 2= oc odbc
                       "type\n"
                       "  tConnect = t%sConnect;\n"
                       "  tQuery   = t%sQuery;\n";  // 1 2 2 2
scc CrPASTop2[]      = "const %s_TOKEN = '%s';\n";
scc CrPASCode2a[]    = "procedure %s%s( ";          // 1 3=Proc
scc CrPASCode2b[]    = "function %s%s( ";           // 1 3
scc CrPASCode3a[]    = "Connect : tConnect\n";
scc CrPASCode4a[]    = "%*s; var aRec : t%s%s\n";     // 4=space 1
scc CrPASCode4b[]    = "%*s; %s\n";                 // 4 5=Parm
scc CrPASCode5a[]    = "%*s);\n";                   // 4
scc CrPASCode5b[]    = "%*s) : boolean;\n";         // 4
scc CrPASCode5c[]    = "%*s) : tQuery;\n";          // 4
scc CrPASCode5d[]    = "%*s) : Longint;\n";         // 4
scc CrPASCode6a[]    = "implementation\n";
scc CrPASCode7a[]    = "var\n";
scc CrPASCode8a[]    = "  aKey : t%sKey;\n";
scc CrPASCode8b[]    = "  aRec : t%sExists;\n";
scc CrPASCode8c[]    = "  Dummy : word;\n";
scc CrPASCode8d[]    = "  aRec : longint;\n";
scc CrPASCode9a[]    = "begin\n";
scc CrPASCode10a[]   = "  Connect.RunProc('%s%s', aRec);\n";
scc CrPASCode10b[]   = "  Result := true;\n"
                       "  try\n";
scc CrPASCode10c[]   = "  Connect.RunProc('%s%s', Dummy);\n";
scc CrPASCode11a[]   = "    aRec.%s := %s;\n";
scc CrPASCode11b[]   = "    strcopy(aRec.%s, %s);\n";
scc CrPASCode11c[]   = "  aKey.%s := %s;\n";
scc CrPASCode11d[]   = "  strcopy(aKey.%s, %s);\n";
scc CrPASCode12a[]   = "    Connect.RunProc('%s%s', aRec);\n"
                       "  except\n"
                       "    on e : ePSqlAPIError do begin\n"
                       "      if e.rc <> SQLapiNoMoreRows then\n"
                       "        Raise\n"
                       "      else\n"
                       "        Result := false;\n"
                       "    end;\n"
                       "    else Raise;\n"
                       "  end;\n"
                       "end;\n\n";
scc CrPASCode12b[]   = "  Result := aRec.Count = 1;\n";
scc CrPASCode12c[]   = "  Connect.RunProc('%s%s', aKey);\n";
scc CrPASCode12d[]   = "  Result := aRec;\n";
scc CrPASCode12e[]   = "  Result := tQuery.CreateExec(Connect, '%s%s', aRec);\n";
scc CrPASCode13a[]   = "end;\n\n";
scc CrPASCode14a[]   = "end.\n";
scc CrPASStart[]     = "type t%s%s = record\n";
scc CrPASStartKey[]  = "type t%s%sKEY = record\n";
scc CrPASStartProc[] = "type t%s%s%s%s = record\n";
scc CrPASField[]     = "  %s\n";
scc CrPASFiller[]    = "  filler%-3d            : array[1..%d] of char;\n";
scc CrPASEnd[]       = "end;\n\n";
scc CrHDefault[]     = "  t%s%s%s%s()\n"
                       "  {\n"
                       "    %smemset(this, 0, sizeof(*this));\n"
                       "  }\n";
scc CrRunProc0[]     = "  void %s(tDBConnect& C";
scc CrRunProc1[]     = ", %s";
scc CrRunProc2[]     = ")\n"
                       "  {\n";
scc CrRunProc3a[]    = "    %s = a%s;\n";
scc CrRunProc3b[]    = "    strncpyz(%s%s, a%s, sizeof(%s)-1);\n";
scc CrRunProc3c[]    = "    memcpy(%s%s, a%s, sizeof(%s));\n";
scc CrRunProc4[]     = "    C.RunProc(\"%s%s%s\", this);\n"
                       "  }\n";
scc CrCount[]        = "inline int %s%sCount(tDBConnect& C)\n"
                       "{\n"
                       "  int L;\n"
                       "  C.RunProc(\"%s%sCount\", &L);\n"
                       "  return L;\n"
                       "}\n\n";
scc CrQuery[]        = "struct t%s%s%s%sQuery : public tDBQuery\n"
                       "{\n"
                       "  t%s%s%s%sQuery(tDBConnect& C, p%s%s%s%s D, bool DoExec=True)\n"
                       "  : tDBQuery(C, \"%s%s%s\", D)\n"
                       "    {\n"
                       "    if (DoExec == True)\n"
                       "      Exec();\n"
                       "    }\n"
                       "};\n";
scc CrQueryLittle[]    = "struct t%s%s%s%sQuery : public tDBQuery\n"
                       "{\n"
                       "  t%s%s%s%sQuery(tDBConnect& C, p%s%s%s%s D, bool DoExec=true)\n"
                       "  : tDBQuery(C, \"%s%s%s\", D)\n"
                       "    {\n"
                       "    if (DoExec == true)\n"
                       "      Exec();\n"
                       "    }\n"
                       "};\n";
scc CrNoFields[]     = "inline void %s%s%s(tDBConnect& C)\n"
                       "{\n"
                       "  C.RunProc(\"%s%s%s\");\n"
                       "}\n\n";

static FILE *HeaderFile   = 0;
static FILE *VBFile       = 0;
static FILE *PASFile      = 0;

char NullFile[] = "nul";

static char *Dehash(const char *Name);
static void GenerateHeadTop(PYYTable Table);
static void GenerateHeadTable(PYYTable Table);
static void GenerateHeadTableExtended(PYYTable Table);
static void GenerateHeadProc(PYYTable Table, PYYProc Proc);
static void GenerateHeadProcExtended(PYYTable Table, PYYProc Proc);
static void GenerateHeadProcOthers(PYYTable Table, PYYProc Proc);
static void GenerateHeadKey(PYYTable Table, PYYKey Key);
static void GenerateHeadKeyExtended(PYYTable Table);
static void GenerateReadOne(ushort noFields, PYYField Fields, PYYTable Table, const char *ProcName, int All);
static void GenerateHeadBottom(void);
static void GenerateHeadTableSwaps(PYYTable Table);
static void GenerateHeadKeySwaps(PYYKey Key);
static void GenerateHeadProcSwaps(PYYProc Proc);
static void GenerateHeadTableDL(PYYTable Table);
static void GenerateHeadTableXML(PYYTable Table);
static void GenerateHeadKeyDL(PYYTable Table, PYYKey Key);
static void GenerateHeadKeyXML(PYYTable Table, PYYKey Key);
static void GenerateHeadKeyPop(PYYTable Table, PYYKey Key);
static void GenerateHeadProcDL(PYYTable Table, PYYProc Proc);
static void GenerateHeadProcXML(PYYTable Table, PYYProc Proc);
static int OpenVBFile(const char *VBFileName);
static void GenerateVBTop(const char *FName);
static int OpenVB5File(const char *VBFileName);
static void GenerateVB5Top(const char *FName);
static void GenerateVBTable(PYYTable Table);
static void GenerateVB5Table(PYYTable Table);
static void GenerateVBBottom(void);
static void GenerateVBProc(PYYTable Table, PYYProc Proc);
static void GenerateVBKey(PYYTable Table, PYYKey Key);
static void GenerateVB5Proc(PYYTable Table, PYYProc Proc);
static void GenerateVB5Key(PYYTable Table, PYYKey Key);
static void GenerateVBExtendedRec(PYYTable Table, const char *ProcName);
static void GenerateVBExtendedKey(PYYTable Table, PYYKey Key, const char *ProcName);
static void GenerateVBExtendedKeyRec(PYYTable Table, PYYKey Key, const char *ProcName);
static void GenerateVBExtendedKeyExists(PYYTable Table, PYYKey Key, const char *ProcName);
static void GenerateVBExtendedCount(PYYTable Table, const char *ProcName);
static void GenerateVBExtendedDeleteAll(PYYTable Table, const char *ProcName);
static void GenerateVBExtended(PYYTable Table);
static void GenerateVBConstsDescr(PYYTable Table);

static void GeneratePASTop(PYYTable Table);
static void GeneratePASTable(PYYTable Table);
static void GeneratePASBottom(void);
static void GeneratePASProc(PYYTable Table, PYYProc Proc);
static void GeneratePASKey(PYYTable Table, PYYKey Key);
static void GeneratePASInterface(PYYTable Table);
static void GeneratePASImplementation(PYYTable Table);
static void GeneratePASIntSelectOne(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASImpSelectOne(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASIntDeleteOne(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASImpDeleteOne(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASIntExists(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASImpExists(PYYTable Table, PYYKey Key, const char *ProcName);
static void GeneratePASIntRec(PYYTable Table, const char *ProcName, int useProc);
static void GeneratePASImpRec(PYYTable Table, const char *ProcName, int useProc);
static void GeneratePASIntCount(PYYTable Table, const char *ProcName);
static void GeneratePASImpCount(PYYTable Table, const char *ProcName);
static void GeneratePASIntNoRec(PYYTable Table, const char *ProcName);
static void GeneratePASImpNoRec(PYYTable Table, const char *ProcName);
static void GeneratePASIntQuery(PYYTable Table, const char *ProcName, int useProc);
static void GeneratePASImpQuery(PYYTable Table, const char *ProcName, int useProc);
static void GeneratePASIntProc(PYYTable Table, PYYProc Proc);
static void GeneratePASImpProc(PYYTable Table, PYYProc Proc);

static char *HeadFieldType(PYYField Field, const char *table);
static char *HeadListType(PYYField Field, const char *table);
static char *ToXMLFormat(char *Work, PYYField Field);
static char *ToBuildFormat(char *Work, PYYField Field);
static char *FromXMLFormat(char *Work, PYYField Field);
static char *VBFieldType(PYYField Field);
static char *VBParmType(PYYField Field);
static char *PASFieldType(PYYField Field);
static char *PASParmType(PYYField Field);
static bool PASispchar(PYYField Field);
static int NeedsSwap(PYYField Field);
static int NeedsTrim(PYYField Field);

static char *VB5ConstrFieldType(PYYField Field, bool isArray = false);
static char *VB5GetFieldType(PYYField Field, bool isArray = false);
static char *VB5SetFieldType(PYYField Field, bool isArray = false);
static bool VB5FieldIsString(PYYField Field);
static char *NameOf(PYYTable Table);
static char *NameOf(PYYField Field);

static char VB5CurrClassName[512];
static void VB5Top(const char *Name)
{
  strcpy(VB5CurrClassName, Name);
  fprintf(VBFile, "VERSION 1.0 CLASS\n"
                  "BEGIN\n"
                  "  MultiUse = -1\n"
                  "END\n"
                  "ATTRIBUTE VB_NAME=\"T%s\"\n"
                  "Option explicit\n"
                  "' This code was generated, do not modify it, "
                  "modify it at source and regenerate it.\n\n", Name);
}

static void VB5Start()
{
  fprintf(VBFile, "Private Erc As Integer\n"
                  "Private Type tRec\n");
}

static void VB5Rec(bool isArrayed = false)
{
  fprintf(VBFile, "Private SaveRec As tRec\n\n");
  if (isArrayed)
  {
    fprintf(VBFile, "Private fRec() As tRec\n"
                    "Public RecCount As Long\n\n"
                    "Friend Property Get ZZQI(aIndex As Long) As tRec\n"
                    "  ZZQI = fRec(aIndex)\n"
                    "End Property\n\n"
                    "Friend Property Let Copy(aDest As Long, "
                                               "aSource As Long, "
                                               "aClass as T%s)\n"
                    "  fRec(aDest) = aClass.ZZQI(aSource)\n"
                    "End Property\n\n"
                    "Public Sub RecClear()\n"
                    "  ReDim Preserve fRec(0 to 0)\n"
                    "  RecCount = 0\n"
                    "End Sub\n\n"
                    "Public Sub Resize(aSize As Long)\n"
                    "  ReDim Preserve fRec(0 to aSize)\n"
                    "  RecCount = aSize\n"
                    "End Sub\n\n"
                    "Public Sub Swap(A As Long, B As Long)\n"
                    "  Dim Temp As tRec\n"
                    "  Temp   = fRec(A)\n"
                    "  fRec(A) = fRec(B)\n"
                    "  fRec(B) = Temp\n"
                    "End Sub\n\n", VB5CurrClassName);
  }
  else
  {
    fprintf(VBFile, "Private fRec As tRec\n\n"
                    "Friend Property Get ZZQI() As tRec\n"
                    "  ZZQI = fRec\n"
                    "End Property\n\n"
                    "Friend Property Let Copy(aClass as T%s)\n"
                    "  fRec = aClass.ZZQI\n"
                    "End Property\n\n", VB5CurrClassName);
  }
}

static void VB5IO(bool isArrayed = false)
{
  if (isArrayed)
  {
    fprintf(VBFile, "Public Sub Class_PutFile(OutFile As Integer, aIndex As Long)\n"
                    "  Put OutFile,, fRec(aIndex)\n"
                    "End Sub\n\n"
                    "Public Sub Class_GetFile(InFile As Integer, aIndex As Long)\n"
                    "  Get InFile,, fRec(aIndex)\n"
                    "End Sub\n\n"
                    );
  }
  else
  {
    fprintf(VBFile, "Public Sub Class_PutFile(OutFile As Integer)\n"
                    "  Put OutFile,, fRec\n"
                    "End Sub\n\n"
                    "Public Sub Class_GetFile(InFile As Integer)\n"
                    "  Get InFile,, fRec\n"
                    "End Sub\n\n"
                    );
  }
}

static void VB5GetLet(const char *GetFT, bool isString, const char *Name, const char *SetFT, bool isArray = false)
{
  char plus[] = "\0NDER";
  if (Name[0] == '_')
    plus[0] = 'U';
  const char *array = "";
  if (isArray) array = "(aIndex)";
  fprintf(VBFile, "Public Property Get %s\n", GetFT);
  if (isString)
    fprintf(VBFile, "  %s%s = RTrim$(fRec%s.%s%s)\n",plus , Name, array, plus, Name);
  else
    fprintf(VBFile, "  %s%s = fRec%s.%s%s\n",plus , Name, array,plus , Name);
  fprintf(VBFile, "End Property\n\n");
  fprintf(VBFile, "Public Property Let %s\n", SetFT);
  fprintf(VBFile, "  fRec%s.%s%s = aValue\n",array, plus, Name);
  fprintf(VBFile, "End Property\n\n");
}

static void VB5NoOutputProc(PYYTable Table, const char *ProcName, bool isArray = false)
{
  const char *array = ""; const char *param = "";
  if (isArray)
  {
     param = ", Optional aIndex As Long";
     array = "(aIndex)";
  }
  fprintf(VBFile, "Public Sub %s(Connect as TConnect%s)\n"
                  "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", fRec%s)\n"
                  "  If Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\", Connect.ErrorMsg(Erc)\n"
                  "End Sub\n\n", ProcName, param, NameOf(Table), US, ProcName, array, NameOf(Table), US, ProcName);
}

static void VB5SingleProc(PYYTable Table, const char *ProcName, bool isArray = false)
{
  const char *array = ""; const char *param = "";
  if (isArray)
  {
     param = ", Optional aIndex As Long";
     array = "(aIndex)";
  }
  fprintf(VBFile, "Public Function %s(Connect as TConnect%s) as Boolean\n"
                  "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", fRec%s)\n"
                  "  %s = Connect.CheckSingle(Erc, \"%s%s%s\")\n"
                  "End Function\n\n",
                  ProcName, param,
                  NameOf(Table), US, ProcName, array,
                  ProcName, NameOf(Table), US, ProcName);
}

static void VB5MultiProc(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, "Public Sub %s(Connect as TConnect)\n"
                  "  Set %sCursor = New TCursor\n"
                  "  Dim Handle as Long\n"
                  "  Erc = DBOpenExec(Connect.Handle, Handle, \"%s%s%s\", fRec(0))\n"
                  "  %sCursor.Handle = Handle\n"
                  "  If Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\", %sCursor.ErrorMsg(Erc)\n"
                  "End Sub\n\n",
                  ProcName, ProcName, NameOf(Table), US, ProcName, ProcName, NameOf(Table), US, ProcName, ProcName);
  fprintf(VBFile, "Public Function next%s() as Boolean\n"
                  "  if %sCursor Is Nothing then Err.Raise SQLapiCursorErr, \"%s%sNext%s\","
                  " \"Run %s%s%s first\"\n",
                  ProcName, ProcName, NameOf(Table), US, ProcName, NameOf(Table), US, ProcName);
  fprintf(VBFile, "  Erc = DBFetchData(%sCursor.Handle, fRec(0))\n"
                  "  next%s = %sCursor.CheckEOF(Erc, \"%s%s%s\")\n"
                  "End Function\n\n",
                  ProcName, ProcName, ProcName, NameOf(Table), US, ProcName);
  fprintf(VBFile, "Public Sub cancel%s()\n"
                  "' You must call call this when you terminate the fetch loop prematurly\n"
                  "' but you must call this only if you do\n"
                  "  if %sCursor.isOpen then %sCursor.Done\n"
                  "End Sub\n\n",
                  ProcName ,ProcName, ProcName);
  fprintf(VBFile, "Public Sub load%s(Connect as TConnect, Optional Max As Long)\n"
                  "  Dim AllocCount As Long\n"
                  "  AllocCount = 32\n"
                  "  RecCount = 0\n"
                  "  ReDim Preserve fRec(0 to AllocCount)\n"
                  "  %s Connect\n"
                  "  Do While next%s()\n"
                  "    RecCount = RecCount + 1\n"
                  "    if RecCount > AllocCount Then\n"
                  "      AllocCount = AllocCount * 2\n"
                  "      ReDim Preserve fRec(0 to AllocCount)\n"
                  "    End If\n"
                  "    fRec(RecCount) = fRec(0)\n"
                  "    If RecCount = Max Then\n"
                  "      cancel%s\n"
                  "      Exit Do\n"
                  "    End If\n"
                  "  Loop\n"
                  "  ReDim Preserve fRec(0 to RecCount)\n"
                  "End Sub\n\n",
                  ProcName, ProcName, ProcName, ProcName);
}

static void VB5NoIOProc(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, "Public Sub %s(Connect as TConnect)\n"
                  "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", 0)\n"
                  "  if Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\","
                  " Connect.ErrorMsg(Erc)\n"
                  "End Sub\n\n", ProcName, NameOf(Table), US, ProcName, NameOf(Table), US, ProcName);
}

static bool VB5CursorVars(const char *ProcName)
{
  fprintf(VBFile, "Private %sCursor as TCursor\n\n", ProcName);
  return true;
}

static char *NameOf(PYYField Field)
{
  return Field->Alias ? Field->Alias : Dehash(Field->Name);
}

static char *NameOf(PYYTable Table)
{
  return Table->Alias ? Table->Alias : Dehash(Table->Name);
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

static int OpenVBFile(const char *VBFileName)
{
  VBFile = OpenFile("VB File", VBFileName);
  if (VBFile == 0)
    return 1;
  char *p=FNameName((char *)VBFileName);
  GenerateVBTop(p);
  free(p);
  return 0;
}

static int OpenVB5File(const char *VBFileName)
{
  VBFile = OpenFile("VB File", VBFileName);
  if (VBFile == 0)
    return 1;
  char *p=FNameName((char *)VBFileName);
  GenerateVB5Top(p);
  free(p);
  return 0;
}

char *MakeVB5FileName(const char *path, const char *funcname)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(path, dir, fname, ext);
  if (strlen(Table->VB5Dir))
    FNameMerge(newpath, Table->VB5Dir, funcname, Table->VB5Ext);
  else
    FNameMerge(newpath, path, funcname, Table->VB5Ext);
  return strdup(newpath);
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

static void MakeVB5Name(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->VB5Dir))
    FNameMerge(newpath, Table->VB5Dir, fname, Table->VB5Ext);
  else
    FNameMerge(newpath, dir, fname, Table->VB5Ext);
  Table->VBFileName = strdup(newpath);
}

void GenerateVB5(PYYTable Table)
{
  MakeVB5Name(Table);
  ushort i;
  if (Table->noFields > 0)
  {
    char *path = MakeVB5FileName(Table->VBFileName, NameOf(Table));
    int rc = OpenVB5File(path);
    free(path);
    if (rc)
      return;
    if (strlen(NameOf(Table)))
      GenerateVB5Table(Table);
    if (Table->hasPrimaryKey && Table->hasDeleteOne)
      GenerateVB5Key(Table, &Table->Keys[Table->PrimaryKey]);
  }
  for (i=0; i<Table->noProcs; i++)
    GenerateVB5Proc(Table, &Table->Procs[i]);
  fclose(VBFile);VBFile=0;
}

static void MakeVBName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->VBDir))
    FNameMerge(newpath, Table->VBDir, fname, Table->VBExt);
  else
    FNameMerge(newpath, dir, fname, Table->VBExt);
  Table->VBFileName = strdup(newpath);
}

void GenerateVB(PYYTable Table)
{
  MakeVBName(Table);
  ushort i;
  if (OpenVBFile(Table->VBFileName))
    return;
  if (strlen(NameOf(Table)))
    GenerateVBTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateVBKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateVBProc(Table, &Table->Procs[i]);
  if (Table->ExtendedVB == 1 || Table->MarkedExtVB == 1)
    GenerateVBExtended(Table);
  GenerateVBConstsDescr(Table);
  fclose(VBFile);VBFile=0;
}

static void GenerateVBTop(const char *FName)
{
  fprintf(VBFile, CrVBTop, FName);
}

static void GenerateVB5Top(const char *FName)
{
  VB5Top(FName);
}

static void GenerateVBConstsDescr(PYYTable Table)
{
  int i;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(VBFile, "Function Get%s%sDescr(No As Integer) As String\n"
                      "  Select Case No\n"
                    , NameOf(Table)
                    , NameOf(Field));
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(VBFile, "    Case %s%s%s\n"
                        "       Get%s%sDescr = \"%s\"\n"
                      , NameOf(Table)
                      , NameOf(Field)
                      , Const->Name
                      , NameOf(Table)
                      , NameOf(Field)
                      , Const->Name);
      }
      fprintf(VBFile, "  End Select\n"
                      "End Function\n\n");
    }
  }
}

static void GenerateVBTable(PYYTable Table)
{
  ushort i;
  if (Table->ExtendedVB == 1 || Table->MarkedExtVB == 1)
    fprintf(VBFile, CrVBProc0);
  if (Table->noFields == 0) return;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        if (Const->Type == 0)
        {
          fprintf(VBFile, "Public Const %s%s%s = %d\n"
                    , NameOf(Table)
                    , NameOf(Field)
                    , Const->Name
                    , Const->Value
                    );
        }
        else
        {
          fprintf(VBFile, "Public Const %s%s%s = Asc(\"%c\")\n"
                    , NameOf(Table)
                    , NameOf(Field)
                    , Const->Name
                    , Const->Value
                    );
        }
      }
      fprintf(VBFile, "\n");
    }
  }
  fprintf(VBFile, CrVBStart, US, NameOf(Table));
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBFile, "  %sIsNull As Integer\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Table->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  Table->noBuffers++;
}

static void GenerateVB5Table(PYYTable Table)
{
  ushort i;
  if (Table->noFields == 0) return;
  VB5Start();
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBFile, "  %sIsNull As Integer\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Table->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  bool isArrayed = false;
  if (hasStdRtns(Table))
  {
    if (Table->hasSelectAll || (Table->extSelectAll & doVB5))
      isArrayed = VB5CursorVars("SelectAll");
    if (Table->hasSelectAllUpd || (Table->extSelectAllUpd & doVB5))
      isArrayed = VB5CursorVars("SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isFetch)
        continue;
      if (Proc->useStd)
        isArrayed = VB5CursorVars(Proc->Name);
    }
    VB5Rec(isArrayed);
    if (Table->hasVB5IO)
      VB5IO(isArrayed);
    fprintf(VBFile, "Private Sub Class_Initialize()\n");
    if (isArrayed)
      fprintf(VBFile, "  RecClear\n");
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      fprintf(VBFile, "  %s\n", VB5ConstrFieldType(Field, isArrayed));
    }
    fprintf(VBFile, "End Sub\n\n");
    if (isArrayed)
      fprintf(VBFile, "Private Sub Class_Terminate()\n"
                      "  Erase fRec\n"
                      "End Sub\n\n");
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      VB5GetLet(VB5GetFieldType(Field, isArrayed), VB5FieldIsString(Field), NameOf(Field),
                VB5SetFieldType(Field, isArrayed), isArrayed);
    }
    if (Table->hasInsert || (Table->extInsert & doVB5))
      VB5NoOutputProc(Table, "Insert", isArrayed);
    if (Table->hasUpdate || (Table->extUpdate & doVB5))
      VB5NoOutputProc(Table, "Update", isArrayed);
    if (Table->hasDeleteAll || (Table->extDeleteAll & doVB5))
      VB5NoIOProc(Table, "DeleteAll");
    if (Table->hasSelectOne || (Table->extSelectOne & doVB5))
      VB5SingleProc(Table, "SelectOne", isArrayed);
    if (Table->hasSelectOneUpd || (Table->extSelectOneUpd & doVB5))
      VB5SingleProc(Table, "SelectOneUpd", isArrayed);
    if (Table->hasSelectAll || (Table->extSelectAll & doVB5))
      VB5MultiProc(Table, "SelectAll");
    if (Table->hasSelectAllUpd || (Table->extSelectAllUpd & doVB5))
      VB5MultiProc(Table, "SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd == 1)
        continue;
      if (Proc->noFields == 0)
        VB5NoIOProc(Table, Proc->Name);
      else if (Proc->useStd)
      {
        if (Proc->isSql && Proc->isFetch)
          VB5SingleProc(Table, Proc->Name, isArrayed);
        else if (Proc->isSql && Proc->noOutputs > 0)
          VB5MultiProc(Table, Proc->Name);
        else
          VB5NoOutputProc(Table, Proc->Name, isArrayed);
      }
    }
  }
  Table->noBuffers++;
}

static void GenerateVB5Key(PYYTable Table, PYYKey Key)
{
  ushort i;
  if (Key->noFields == 0)
    return;
  char Work[512];
  if (VBFile)
  {
    fclose(VBFile);
    VBFile=0;
  }
  sprintf(Work, "%s%sDeleteOne", US, NameOf(Table));
  char *path = MakeVB5FileName(Table->VBFileName, Work);
  int rc = OpenVB5File(path);
  free(path);
  if (rc)
    return;
  VB5Start();
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Key->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  VB5Rec();
  fprintf(VBFile, "Private Sub Class_Initialize()\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, "  %s\n", VB5ConstrFieldType(Field));
  }
  fprintf(VBFile, "End Sub\n\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    VB5GetLet(VB5GetFieldType(Field), VB5FieldIsString(Field), NameOf(Field),
              VB5SetFieldType(Field));
  }
  VB5NoOutputProc(Table, "DeleteOne");
  Table->noBuffers++;
}

static void GenerateVBKey(PYYTable Table, PYYKey Key)
{
  ushort i;
  if (Key->noFields == 0)
    return;
  fprintf(VBFile, CrVBStartKey, US, NameOf(Table));
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Key->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  Table->noBuffers++;
}

static void GenerateVBProc(PYYTable Table, PYYProc Proc)
{
  ushort i;
  bool GenIf = false;
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if ((Proc->extProc & noIDL) == noIDL)
  {
    fprintf(VBFile, "#If Not DropNoIDL Then\n\n");
    GenIf = true;
  }
  fprintf(VBFile, CrVBStartProc, US, NameOf(Table), US, Proc->Name);
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBFile, "  %sIsNull As Integer\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Proc->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  if (GenIf == true)
    fprintf(VBFile, "#End If\n\n");
  Table->noBuffers++;
}

static void GenerateVB5Proc(PYYTable Table, PYYProc Proc)
{
  ushort i;
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  char Work[512];
  if (VBFile)
  {
    fclose(VBFile);
    VBFile=0;
  }
  sprintf(Work, "%s%s%s", NameOf(Table), US, Proc->Name);
  char *path = MakeVB5FileName(Table->VBFileName, Work);
  int rc = OpenVB5File(path);
  free(path);
  if (rc)
    return;
  VB5Start();
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
       fprintf(VBFile, CrVBFiller, i, Field->Filler);
    fprintf(VBFile, CrVBField, VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBFile, "  %sIsNull As Integer\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Proc->Length % padvalue)
    fprintf(VBFile, CrVBFiller, i, padvalue - pad);
  fprintf(VBFile, CrVBEnd);
  bool isArrayed = false;
  if (Proc->isSql && Proc->noOutputs > 0 && !Proc->isFetch)
    isArrayed = VB5CursorVars(Proc->Name);
  VB5Rec(isArrayed);
  fprintf(VBFile, "Private Sub Class_Initialize()\n");
  if (isArrayed)
    fprintf(VBFile, "  RecClear\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(VBFile, "  %s\n", VB5ConstrFieldType(Field, isArrayed));
  }
  fprintf(VBFile, "End Sub\n\n");
  if (isArrayed)
    fprintf(VBFile, "Private Sub Class_Terminate()\n"
                    "  Erase fRec\n"
                    "End Sub\n\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    VB5GetLet(VB5GetFieldType(Field, isArrayed), VB5FieldIsString(Field),
              NameOf(Field), VB5SetFieldType(Field, isArrayed), isArrayed);
  }
  if (Proc->isSql && Proc->isFetch)
    VB5SingleProc(Table, Proc->Name, isArrayed);
  else if (Proc->isSql && Proc->noOutputs > 0)
    VB5MultiProc(Table, Proc->Name);
  else
    VB5NoOutputProc(Table, Proc->Name, isArrayed);
  Table->noBuffers++;
}

static void GenerateVBExtendedProcRec(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc2c, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc6c, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc9);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedProcNoRec(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc6d, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isSql && !Proc->isFetch && Proc->noOutputs > 0)
    return;
  if (Proc->isData)
    return;
  if (Proc->isStd)
    return;
  if (Table->MarkedExtVB && (Proc->extProc && doEXTVB) == 0)
    return;
  if (Proc->noFields > 0)
    GenerateVBExtendedProcRec(Table, Proc->Name);
  else
    GenerateVBExtendedProcNoRec(Table, Proc->Name);
}

static void GenerateVBExtendedRec(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc2b, NameOf(Table), NameOf(Table));
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc6b, NameOf(Table), ProcName, NameOf(Table));
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

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

static void GenerateVBExtendedKey(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc2a, VBParmType(Field));
  }
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc4a, NameOf(Table), NameOf(Table));
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc5a, NameOf(Table), NameOf(Field), NameOf(Field));
  }
  fprintf(VBFile, CrVBProc6a, NameOf(Table), ProcName, NameOf(Table));
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedKeyRec(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc2a, VBParmType(Field));
  }
  fprintf(VBFile, CrVBProc2b, NameOf(Table), NameOf(Table));
  fprintf(VBFile, CrVBProc3a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc5b, NameOf(Table), NameOf(Field), NameOf(Field));
  }
  fprintf(VBFile, CrVBProc6b, NameOf(Table), ProcName, NameOf(Table));
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc9);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedKeyExists(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc2a, VBParmType(Field));
  }
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc4b, NameOf(Table), ProcName, NameOf(Table), ProcName);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBFile, CrVBProc5c, NameOf(Table), ProcName, NameOf(Field), NameOf(Field));
  }
  fprintf(VBFile, CrVBProc6e, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc7b, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc8, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedCount(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc3b);
  fprintf(VBFile, CrVBProc4c, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc6e, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc7c, NameOf(Table), ProcName, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc8, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtendedDeleteAll(PYYTable Table, const char *ProcName)
{
  fprintf(VBFile, CrVBProc1, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc3a);
  fprintf(VBFile, CrVBProc6d, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc7a, NameOf(Table), ProcName);
  fprintf(VBFile, CrVBProc10, NameOf(Table), ProcName);
}

static void GenerateVBExtended(PYYTable Table)
{
  ushort i;
  fprintf(VBFile, "#If Not NoVBExtended Then\n\n");
  if (Table->hasSelectOne && Table->hasPrimaryKey
  && Table->ExtendedVB || (Table->extSelectOne & doEXTVB))
    GenerateVBExtendedKeyRec(Table, &Table->Keys[Table->PrimaryKey], "SelectOne");
  if (Table->hasSelectOneUpd && Table->hasPrimaryKey
  && Table->ExtendedVB || (Table->extSelectOneUpd & doEXTVB))
    GenerateVBExtendedKeyRec(Table, &Table->Keys[Table->PrimaryKey], "SelectOneUpd");
  if (Table->hasDeleteOne && Table->hasPrimaryKey
  && Table->ExtendedVB || (Table->extDeleteOne & doEXTVB))
    GenerateVBExtendedKey(Table, &Table->Keys[Table->PrimaryKey], "DeleteOne");
  if (Table->hasUpdate
  && Table->ExtendedVB || (Table->extUpdate & doEXTVB))
    GenerateVBExtendedRec(Table, "Update");
  if (Table->hasInsert
  && Table->ExtendedVB || (Table->extInsert & doEXTVB))
    GenerateVBExtendedRec(Table, "Insert");
  if (Table->hasExists && Table->hasPrimaryKey
  && Table->ExtendedVB || (Table->extExists & doEXTVB))
    GenerateVBExtendedKeyExists(Table, &Table->Keys[Table->PrimaryKey], "Exists");
  if (Table->hasCount
  && Table->ExtendedVB || (Table->extCount & doEXTVB))
    GenerateVBExtendedCount(Table, "Count");
  if (Table->hasDeleteAll
  && Table->ExtendedVB || (Table->extDeleteAll & doEXTVB))
    GenerateVBExtendedDeleteAll(Table, "DeleteAll");
  for (i=0; i<Table->noProcs; i++)
    GenerateVBExtendedProc(Table, &Table->Procs[i]);
  fprintf(VBFile, "#End If\n\n");
}

const char GenAbbr[] = "oci";

static void MakePasName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->PASDir))
    FNameMerge(newpath, Table->PASDir, fname, Table->PASExt);
  else
    FNameMerge(newpath, dir, fname, Table->PASExt);
  Table->PASFileName = strdup(newpath);
}

void GeneratePAS(PYYTable Table)
{
  ushort i;
  MakePasName(Table);
  PASFile = OpenFile("PAS File", Table->PASFileName);
  if (PASFile == 0)
    return;
  GeneratePASTop(Table);
  if (strlen(NameOf(Table)))
    GeneratePASTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GeneratePASKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GeneratePASProc(Table, &Table->Procs[i]);
  if (Table->ExtendedPAS == 1 || Table->MarkedExtPAS == 1)
  {
    GeneratePASInterface(Table);
    fprintf(PASFile, CrPASCode6a);
    GeneratePASImplementation(Table);
    fprintf(PASFile, CrPASCode14a);
  }
  fclose(PASFile);
}

static void GeneratePASTop(PYYTable Table)
{
  char *p=FNameName((char *)Table->PASFileName);
  fprintf(PASFile, CrPASTop, p, GenAbbr);
  if (Table->ExtendedPAS == 1 || Table->MarkedExtPAS == 1)
    fprintf(PASFile, CrPASCode1a, Table->ShortName, "", GenAbbr, GenAbbr, GenAbbr);
  fprintf(PASFile, CrPASTop2, p, TableToken(Table));
  free(p);
}

static void GeneratePASTable(PYYTable Table)
{
  ushort i;

  if (Table->noFields == 0) return;
  fprintf(PASFile, CrPASStart,
          US, NameOf(Table));
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Filler)
       fprintf(PASFile, CrPASFiller, i, Field->Filler);
    fprintf(PASFile, CrPASField, PASFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(PASFile, "  %sIsNull : ShortInt;\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Table->Length % padvalue)
    fprintf(PASFile, CrPASFiller, i, padvalue - pad);
  fprintf(PASFile, CrPASEnd);
  Table->noBuffers++;
}

static void GeneratePASKey(PYYTable Table, PYYKey Key)
{
  ushort i;

  if (Key->noFields == 0)
    return;
  fprintf(PASFile, CrPASStartKey,
          US, NameOf(Table));
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
       fprintf(PASFile, CrPASFiller, i, Field->Filler);
    fprintf(PASFile, CrPASField, PASFieldType(Field));
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Key->Length % padvalue)
    fprintf(PASFile, CrPASFiller, i, padvalue - pad);
  fprintf(PASFile, CrPASEnd);
  Table->noBuffers++;
}

static void GeneratePASProc(PYYTable Table, PYYProc Proc)
{
  ushort i;

  if (Proc->noFields == 0 || Proc->useStd)
    return;
  fprintf(PASFile, CrPASStartProc,
          US, NameOf(Table),
          US, Proc->Name);
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
       fprintf(PASFile, CrPASFiller, i, Field->Filler);
    fprintf(PASFile, CrPASField, PASFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(PASFile, "  %sIsNull : ShortInt;\n", Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Proc->Length % padvalue)
    fprintf(PASFile, CrPASFiller, i, padvalue - pad);
  fprintf(PASFile, CrPASEnd);
  Table->noBuffers++;
}

static void GeneratePASIntSelectOne(PYYTable Table, PYYKey Key, const char *ProcName)
{
  int i, len;
  len = fprintf(PASFile, CrPASCode2b, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, CrPASCode4b, len, "", PASParmType(Field));
  }
  fprintf(PASFile, CrPASCode4a, len, "", NameOf(Table), "");
  fprintf(PASFile, CrPASCode5b, len, "");
}

static void GeneratePASImpSelectOne(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  GeneratePASIntSelectOne(Table, Key, ProcName);
  fprintf(PASFile, CrPASCode9a);
  fprintf(PASFile, CrPASCode10b);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, PASispchar(Field) ? CrPASCode11b : CrPASCode11a,
            NameOf(Field), NameOf(Field));
  }
  fprintf(PASFile, CrPASCode12a, NameOf(Table), ProcName);
}

static void GeneratePASIntDeleteOne(PYYTable Table, PYYKey Key, const char *ProcName)
{
  int i, len;
  len = fprintf(PASFile, CrPASCode2a, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, CrPASCode4b, len, "", PASParmType(Field));
  }
  fprintf(PASFile, CrPASCode5a, len, "");
}

static void GeneratePASImpDeleteOne(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  GeneratePASIntDeleteOne(Table, Key, ProcName);
  fprintf(PASFile, CrPASCode7a);
  fprintf(PASFile, CrPASCode8a, NameOf(Table));
  fprintf(PASFile, CrPASCode9a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, PASispchar(Field) ? CrPASCode11d : CrPASCode11c,
                     NameOf(Field), NameOf(Field));
  }
  fprintf(PASFile, CrPASCode12c, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntExists(PYYTable Table, PYYKey Key, const char *ProcName)
{
  int i, len;
  len = fprintf(PASFile, CrPASCode2b, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, CrPASCode4b, len, "", PASParmType(Field));
  }
  fprintf(PASFile, CrPASCode5b, len, "");
}

static void GeneratePASImpExists(PYYTable Table, PYYKey Key, const char *ProcName)
{
  ushort i;
  GeneratePASIntExists(Table, Key, ProcName);
  fprintf(PASFile, CrPASCode7a);
  fprintf(PASFile, CrPASCode8b, NameOf(Table));
  fprintf(PASFile, CrPASCode9a);
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(PASFile, PASispchar(Field) ? CrPASCode11b : CrPASCode11a,
            NameOf(Field), NameOf(Field));
  }
  fprintf(PASFile, CrPASCode10a, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode12b);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntRec(PYYTable Table, const char *ProcName, int useProc)
{
  int len;
  len = fprintf(PASFile, CrPASCode2a, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  fprintf(PASFile, CrPASCode4a, len, "", NameOf(Table), useProc ? ProcName : "");
  fprintf(PASFile, CrPASCode5a, len, "");
}

static void GeneratePASImpRec(PYYTable Table, const char *ProcName, int useProc)
{
  GeneratePASIntRec(Table, ProcName, useProc);
  fprintf(PASFile, CrPASCode9a);
  fprintf(PASFile, CrPASCode10a, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntCount(PYYTable Table, const char *ProcName)
{
  int len;
  len = fprintf(PASFile, CrPASCode2b, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  fprintf(PASFile, CrPASCode5d, len, "");
}

static void GeneratePASImpCount(PYYTable Table, const char *ProcName)
{
  GeneratePASIntCount(Table, ProcName);
  fprintf(PASFile, CrPASCode7a);
  fprintf(PASFile, CrPASCode8d);
  fprintf(PASFile, CrPASCode9a);
  fprintf(PASFile, CrPASCode10a, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode12d);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntNoRec(PYYTable Table, const char *ProcName)
{
  int len;
  len = fprintf(PASFile, CrPASCode2a, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  fprintf(PASFile, CrPASCode5a, len, "");
}

static void GeneratePASImpNoRec(PYYTable Table, const char *ProcName)
{
  GeneratePASIntNoRec(Table, ProcName);
  fprintf(PASFile, CrPASCode7a);
  fprintf(PASFile, CrPASCode8c);
  fprintf(PASFile, CrPASCode9a);
  fprintf(PASFile, CrPASCode10c, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntQuery(PYYTable Table, const char *ProcName, int useProc)
{
  int len;
  len = fprintf(PASFile, CrPASCode2b, NameOf(Table), ProcName) - 2;
  fprintf(PASFile, CrPASCode3a);
  fprintf(PASFile, CrPASCode4a, len, "", NameOf(Table), useProc?ProcName:"");
  fprintf(PASFile, CrPASCode5c, len, "");
}

static void GeneratePASImpQuery(PYYTable Table, const char *ProcName, int useProc)
{
  GeneratePASIntQuery(Table, ProcName, useProc);
  fprintf(PASFile, CrPASCode9a);
  fprintf(PASFile, CrPASCode12e, NameOf(Table), ProcName);
  fprintf(PASFile, CrPASCode13a);
}

static void GeneratePASIntProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isData)
    return;
  if (Proc->isStd)
    return;
  if (Proc->isSql && Proc->isFetch == 0 && Proc->noOutputs > 0)
    GeneratePASIntQuery(Table, Proc->Name, Proc->useStd ? 0 : 1);
  else if (Proc->noFields > 0)
    GeneratePASIntRec(Table, Proc->Name, Proc->useStd ? 0 : 1);
  else
    GeneratePASIntNoRec(Table, Proc->Name);
}

static void GeneratePASImpProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->isData)
    return;
  if (Proc->isStd)
    return;
  if (Proc->isSql && Proc->isFetch == 0 && Proc->noOutputs > 0)
    GeneratePASImpQuery(Table, Proc->Name, Proc->useStd ? 0 : 1);
  else if (Proc->noFields > 0)
    GeneratePASImpRec(Table, Proc->Name, Proc->useStd ? 0 : 1);
  else
    GeneratePASImpNoRec(Table, Proc->Name);
}

static void GeneratePASInterface(PYYTable Table)
{
  ushort i;
  if (Table->hasSelectOne && Table->hasPrimaryKey)
    GeneratePASIntSelectOne(Table, &Table->Keys[Table->PrimaryKey], "SelectOne");
  if (Table->hasSelectOneUpd && Table->hasPrimaryKey)
    GeneratePASIntSelectOne(Table, &Table->Keys[Table->PrimaryKey], "SelectOneUpd");
  if (Table->hasDeleteOne && Table->hasPrimaryKey)
    GeneratePASIntDeleteOne(Table, &Table->Keys[Table->PrimaryKey], "DeleteOne");
  if (Table->hasUpdate)
    GeneratePASIntRec(Table, "Update", 0);
  if (Table->hasInsert)
    GeneratePASIntRec(Table, "Insert", 0);
  if (Table->hasExists && Table->hasPrimaryKey)
    GeneratePASIntExists(Table, &Table->Keys[Table->PrimaryKey], "Exists");
  if (Table->hasCount)
    GeneratePASIntCount(Table, "Count");
  if (Table->hasDeleteAll)
    GeneratePASIntNoRec(Table, "DeleteAll");
  if (Table->hasSelectAll)
    GeneratePASIntQuery(Table, "SelectAll", 0);
  if (Table->hasSelectAllUpd)
    GeneratePASIntQuery(Table, "SelectAllUpd", 0);
  for (i=0; i<Table->noProcs; i++)
    GeneratePASIntProc(Table, &Table->Procs[i]);
}

static void GeneratePASImplementation(PYYTable Table)
{
  ushort i;
  if (Table->hasSelectOne && Table->hasPrimaryKey)
    GeneratePASImpSelectOne(Table, &Table->Keys[Table->PrimaryKey], "SelectOne");
  if (Table->hasSelectOneUpd && Table->hasPrimaryKey)
    GeneratePASImpSelectOne(Table, &Table->Keys[Table->PrimaryKey], "SelectOneUpd");
  if (Table->hasDeleteOne && Table->hasPrimaryKey)
    GeneratePASImpDeleteOne(Table, &Table->Keys[Table->PrimaryKey], "DeleteOne");
  if (Table->hasUpdate)
    GeneratePASImpRec(Table, "Update", 0);
  if (Table->hasInsert)
    GeneratePASImpRec(Table, "Insert", 0);
  if (Table->hasExists && Table->hasPrimaryKey)
    GeneratePASImpExists(Table, &Table->Keys[Table->PrimaryKey], "Exists");
  if (Table->hasCount)
    GeneratePASImpCount(Table, "Count");
  if (Table->hasDeleteAll)
    GeneratePASImpNoRec(Table, "DeleteAll");
  if (Table->hasSelectAll)
    GeneratePASImpQuery(Table, "SelectAll", 0);
  if (Table->hasSelectAllUpd)
    GeneratePASImpQuery(Table, "SelectAllUpd", 0);
  for (i=0; i<Table->noProcs; i++)
    GeneratePASImpProc(Table, &Table->Procs[i]);
}

static void MakeHeaderName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->CDir))
    FNameMerge(newpath, Table->CDir, fname, Table->CExt);
  else
    FNameMerge(newpath, dir, fname, Table->CExt);
  Table->HeaderFileName = strdup(newpath);
}

void GenerateHeader(PYYTable Table)
{
  ushort i;
  MakeHeaderName(Table);
  HeaderFile = OpenFile("Header File", Table->HeaderFileName);
  if (HeaderFile == 0)
    return;
  GenerateHeadTop(Table);
  if (strlen(NameOf(Table)))
    GenerateHeadTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateHeadKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateHeadProc(Table, &Table->Procs[i]);
  GenerateHeadBottom();
  fclose(HeaderFile);
}

static void GenerateHeadTop(PYYTable Table)
{
  char *p=FNameName((char *)Table->HeaderFileName);
  fprintf(HeaderFile, CrHTop, p, p, TableToken(Table), (unsigned long) Table->Token);
  if (Table->CppHeader == 1)
    fprintf(HeaderFile, CrHTop2);
  else
    fprintf(HeaderFile, CrHTop1);
  free(p);
}

static int AmpType(PYYField Field)
{
  if (   Field->Type == ftypeImage
     ||  Field->Type == ftypeImagePtr
     ||  Field->Type == ftypeBLOB
     ||  Field->Type == ftypeCLOB
     ||  Field->Type == ftypeZLOB
     ||  Field->Type == ftypeXMLTYPE
  )
    return 1;
  return 0;
}

static void GenerateHeadTableExtended(PYYTable Table)
{
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #if defined(_DBPORTAL_H_)\n");
  fprintf(HeaderFile, CrHDefault, US, NameOf(Table), "", "", "");
  if (Table->hasInsert && Table->Internal == 0)
  {
    fprintf(HeaderFile, CrRunProc0, "Insert");
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, "Insert");
  }
  if (Table->hasPrimaryKey && Table->hasSelectOne && Table->Internal == 0)
  {
    GenerateReadOne(Table->Keys[Table->PrimaryKey].noFields,
                    Table->Keys[Table->PrimaryKey].Fields,
                    Table, "SelectOne", 1);
    fprintf(HeaderFile, CrRunProc0, "SelectOne");
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, "SelectOne");
  }
  if (Table->hasPrimaryKey && Table->hasSelectOneUpd && Table->Internal == 0)
  {
    GenerateReadOne(Table->Keys[Table->PrimaryKey].noFields,
                    Table->Keys[Table->PrimaryKey].Fields,
                    Table, "SelectOneUpd", 1);
    fprintf(HeaderFile, CrRunProc0, "SelectOneUpd");
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, "SelectOneUpd");
  }
  if (Table->hasUpdate && Table->Internal == 0)
  {
    fprintf(HeaderFile, CrRunProc0, "Update");
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, "Update");
  }
  int i;
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData)
      continue;
    if (Proc->useStd)
    {
      if (Proc->isFetch != 0)
      {
        GenerateReadOne(Proc->noFields, Proc->Fields,
                        Table, Proc->Name, 0);
        fprintf(HeaderFile, CrRunProc0, Proc->Name);
        fprintf(HeaderFile, CrRunProc2);
        fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
      }
      else if (Proc->noFields == 0)
        fprintf(HeaderFile, CrNoFields, NameOf(Table), US, Proc->Name,
                                        NameOf(Table), US, Proc->Name);
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        fprintf(HeaderFile, CrRunProc0, Proc->Name);
        fprintf(HeaderFile, CrRunProc2);
        fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
        int size=0;
        size += fprintf(HeaderFile, CrRunProc0, Proc->Name);
        int j;
        for (j=0; j<Proc->noFields; j++)
        {
          PYYField Field = &Proc->Fields[j];
          size += fprintf(HeaderFile, CrRunProc1, HeadListType(Field, NameOf(Table)));
          if (size > 74)
            size = fprintf(HeaderFile, "\n        ");
        }
        fprintf(HeaderFile, CrRunProc2);
        for (j=0; j<Proc->noFields; j++)
        {
          PYYField Field = &Proc->Fields[j];
          if (Field->Type == ftypeFloat
          ||  Field->Type == ftypeInt
          ||  Field->Type == ftypeLong
          ||  Field->Type == ftypeSmallint
          ||  Field->Type == ftypeTinyint
          ||  Field->Type == ftypeBoolean
          ||  Field->Type == ftypeHugeCHAR)
            fprintf(HeaderFile, CrRunProc3a, NameOf(Field), NameOf(Field));
          else
            fprintf(HeaderFile, AmpType(Field) == 1 ? CrRunProc3c : CrRunProc3b
                              , AmpType(Field) == 1 ? "&" : ""
                              , NameOf(Field), NameOf(Field), NameOf(Field));
        }
        fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
      }
    }
  }
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKeyExtended(PYYTable Table)
{
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #if defined(_DBPORTAL_H_)\n");
  fprintf(HeaderFile, CrHDefault, US, NameOf(Table), US, "Key", "");
  if (Table->hasDeleteOne)
  {
    fprintf(HeaderFile, CrRunProc0, "DeleteOne");
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, "DeleteOne");
  }
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadTableExtendedOthers(PYYTable Table)
{
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "#if defined(_DBPORTAL_H_)\n");
  if (Table->hasDeleteAll && Table->Internal == 0)
    fprintf(HeaderFile, CrNoFields, NameOf(Table), US, "DeleteAll",
                                    NameOf(Table), US, "DeleteAll");
  if (Table->hasCount && Table->Internal == 0)
    fprintf(HeaderFile, CrCount,     NameOf(Table), US,
                                     NameOf(Table), US);
  if (Table->hasSelectAll && Table->Internal == 0)
    fprintf(HeaderFile, Table->LittleTrue ? CrQueryLittle : CrQuery,
                                 US, NameOf(Table), US, "SelectAll",
                                 US, NameOf(Table), US, "SelectAll",
                                 US, NameOf(Table), "", "",
                                 NameOf(Table), US, "SelectAll");
  if (Table->hasSelectAllUpd && Table->Internal == 0)
    fprintf(HeaderFile, Table->LittleTrue ? CrQueryLittle : CrQuery,
                                 US, NameOf(Table), US, "SelectAllUpd",
                                 US, NameOf(Table), US, "SelectAllUpd",
                                 US, NameOf(Table),  "", "",
                                 NameOf(Table), US, "SelectAllUpd");
  int i;
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData)
      continue;
    if (Proc->useStd)
    {
      if (!Proc->isFetch && Proc->noOutputs)
      {
        fprintf(HeaderFile, Table->LittleTrue ? CrQueryLittle : CrQuery,
                                     US, NameOf(Table), US, Proc->Name,
                                     US, NameOf(Table), US, Proc->Name,
                                     US, NameOf(Table), "", "",
                                     NameOf(Table), US, Proc->Name);
      }
    }
    else if (Proc->noFields == 0)
      fprintf(HeaderFile, CrNoFields, NameOf(Table), US, Proc->Name,
                                      NameOf(Table), US, Proc->Name);
  }
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "#endif\n\n");
}

static void GenerateHeadProcExtended(PYYTable Table, PYYProc Proc)
{
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #if defined(_DBPORTAL_H_)\n");
  fprintf(HeaderFile, CrHDefault, US, NameOf(Table), US, Proc->Name, "");
  if (Proc->isFetch)
  {
    GenerateReadOne(Proc->noFields, Proc->Fields,
                    Table, Proc->Name, 0);
    fprintf(HeaderFile, CrRunProc0, Proc->Name);
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
  }
  else if (Proc->noFields == 0)
    fprintf(HeaderFile, CrNoFields, NameOf(Table), US, Proc->Name,
                                    NameOf(Table), US, Proc->Name);
  else if (Proc->isSql == 0 || Proc->noOutputs == 0)
  {
    fprintf(HeaderFile, CrRunProc0, Proc->Name);
    fprintf(HeaderFile, CrRunProc2);
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
    int size = fprintf(HeaderFile, CrRunProc0, Proc->Name);
    int j;
    for (j=0; j<Proc->noFields; j++)
    {
      PYYField Field = &Proc->Fields[j];
      size += fprintf(HeaderFile, CrRunProc1, HeadListType(Field, NameOf(Table)));
      if (size > 74)
        size = fprintf(HeaderFile, "\n        ");
    }
    fprintf(HeaderFile, CrRunProc2);
    for (j=0; j<Proc->noFields; j++)
    {
      PYYField Field = &Proc->Fields[j];
      if (Field->Type == ftypeFloat
      ||  Field->Type == ftypeInt
      ||  Field->Type == ftypeLong
      ||  Field->Type == ftypeSmallint
      ||  Field->Type == ftypeTinyint
      ||  Field->Type == ftypeBoolean)
        fprintf(HeaderFile, CrRunProc3a, NameOf(Field), NameOf(Field));
      else
        fprintf(HeaderFile, AmpType(Field) == 1 ? CrRunProc3c : CrRunProc3b
                          , AmpType(Field) == 1 ? "&" : ""
                          , NameOf(Field), NameOf(Field), NameOf(Field));
    }
    fprintf(HeaderFile, CrRunProc4, NameOf(Table), US, Proc->Name);
  }
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadProcOthers(PYYTable Table, PYYProc Proc)
{
  if (Proc->isSql && !Proc->isFetch && Proc->noOutputs)
  {
    if (Table->CppHeader == 0)
      fprintf(HeaderFile, "#if defined(_DBPORTAL_H_)\n");
    fprintf(HeaderFile, Table->LittleTrue ? CrQueryLittle : CrQuery,
                                 US, NameOf(Table), US, Proc->Name,
                                 US, NameOf(Table), US, Proc->Name,
                                 US, NameOf(Table), US, Proc->Name,
                                 NameOf(Table), US, Proc->Name);
    if (Table->CppHeader == 0)
      fprintf(HeaderFile, "#endif\n\n");
  }
}

static void GenerateHeadTable(PYYTable Table)
{
  ushort i;
  int f;
  if (Table->noFields)
  {
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      if (Field->noConsts > 0)
      {
        fprintf(HeaderFile, "enum e%s%s\n", NameOf(Table), NameOf(Field));
        for (f=0; f<Field->noConsts; f++)
        {
          PYYConst Const = &Field->Consts[f];
          if (Const->Type == 0)
          {
            fprintf(HeaderFile, "%s %s%s%s = %d\n"
                      , f == 0 ? "{" : ","
                      , NameOf(Table)
                      , NameOf(Field)
                      , Const->Name
                      , Const->Value
                      );
          }
          else
          {
            fprintf(HeaderFile, "%s %s%s%s = '%c'\n"
                      , f == 0 ? "{" : ","
                      , NameOf(Table)
                      , NameOf(Field)
                      , Const->Name
                      , Const->Value
                      );
          }
        }
        fprintf(HeaderFile, "};\n\n");
        fprintf(HeaderFile, "inline char *%s%sLookup(int no)\n"
                            "{\n"
                            "  switch(no)\n"
                            "  {\n",
                            NameOf(Table), NameOf(Field));
        for (f=0; f<Field->noConsts; f++)
        {
          PYYConst Const = &Field->Consts[f];
          fprintf(HeaderFile, "  case %s%s%s: return \"%s\";\n"
                            , NameOf(Table)
                            , NameOf(Field)
                            , Const->Name
                            , Const->Name);
        }
        fprintf(HeaderFile, "  default: return \"<n/a>\";\n"
                            "  }\n"
                            "}\n\n");

      }
    }
    fprintf(HeaderFile, CrHStruct0, "typedef ", US, NameOf(Table));
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      if (Field->Filler)
         fprintf(HeaderFile, CrHStructF, i, Field->Filler);
      fprintf(HeaderFile, CrHStruct1, HeadFieldType(Field, NameOf(Table)));
      if (Table->isNullEnabled && Field->isNull && NullableField(Field))
         fprintf(HeaderFile, CrHStructN, Field->Name);
    }
    int padvalue = 4, pad;
    if (Table->EightByte)
      padvalue = 8;
    if (pad = Table->Length % padvalue)
      fprintf(HeaderFile, CrHStructF, i, padvalue - pad);
  }
  if (Table->noFields)
  {
    GenerateHeadTableSwaps(Table);
    GenerateHeadTableXML(Table);
    GenerateHeadTableDL(Table);
    if (Table->ExtendedC == 1 || Table->MarkedExtC == 1)
      GenerateHeadTableExtended(Table);
    fprintf(HeaderFile, CrHStruct2,
            US, NameOf(Table),
            US, NameOf(Table));
    Table->noBuffers++;
  }
  if (Table->ExtendedC == 1 || Table->MarkedExtC == 1)
    GenerateHeadTableExtendedOthers(Table);
}

static void genDBAddPad(int Length)
{
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Length % padvalue)
    fprintf(HeaderFile, "    dBuild.fill(%d);\n", padvalue - pad);
}

static void genDBAdd(PYYField Field)
{
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  if (Field->Filler > 0)
    fprintf(HeaderFile, "    dBuild.fill(%d);\n", Field->Filler);
  switch (Field->Type)
  {
  case ftypeImage:
  case ftypeImagePtr:
  case ftypeBLOB:
  case ftypeCLOB:
  case ftypeXMLTYPE:
    fprintf(HeaderFile, "    dBuild.add(\"%s\", (void*)&%s, sizeof(%s), \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.add(\"%s\", (void*)&%s, sizeof(%s));\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    fprintf(HeaderFile, "    dBuild.add(\"%s\", %s, sizeof(%s), \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.add(\"%s\", %s, sizeof(%s));\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  default:
    fprintf(HeaderFile, "    dBuild.add(\"%s\", %s, \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.add(\"%s\", %s);\n"
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  }
  fprintf(HeaderFile, "    #endif\n");
}

static void genDBSetPad(int Length)
{
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Length % padvalue)
    fprintf(HeaderFile, "    dBuild.skip(%d);\n", padvalue - pad);
}

static void genDBSet(PYYField Field)
{
  if (Field->Filler > 0)
    fprintf(HeaderFile, "    dBuild.skip(%d);\n", Field->Filler);
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  switch (Field->Type)
  {
  case ftypeImage:
  case ftypeImagePtr:
  case ftypeBLOB:
  case ftypeCLOB:
  case ftypeXMLTYPE:
    fprintf(HeaderFile, "    dBuild.set(\"%s\", (void*)&%s, sizeof(%s), \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.set(\"%s\", (void*)&%s, sizeof(%s));\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    fprintf(HeaderFile, "    dBuild.set(\"%s\", %s, sizeof(%s), \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.set(\"%s\", %s, sizeof(%s));\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  default:
    fprintf(HeaderFile, "    dBuild.set(\"%s\", %s, sizeof(%s), \"%s%s\");\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
                      , Field->isInput ? "i" : ""
                      , Field->isOutput ? "o" : ""
         );
    fprintf(HeaderFile, "    #else\n");
    fprintf(HeaderFile, "    dBuild.set(\"%s\", %s, sizeof(%s));\n"
                      , NameOf(Field)
                      , NameOf(Field)
                      , NameOf(Field)
         );
    break;
  }
  fprintf(HeaderFile, "    #endif\n");
}

static void GenerateHeadTableDL(PYYTable Table)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_DATABUILD_H_) || defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "  // You can supply your own DataBuilder with a guard of _DATABUILD_H_\n");
  fprintf(HeaderFile, "  // and the following methods :-\n");
  fprintf(HeaderFile, "  //  void name(char *);\n");
  fprintf(HeaderFile, "  //  void add(char *, <T> input);\n");
  fprintf(HeaderFile, "  //  void set(char *, <T> output, int size); /* remember char[n] are one size bigger */\n");
  fprintf(HeaderFile, "  void BuildData(DataBuilder &dBuild, char *name=\"%s\")\n"
                    , NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Table->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    genDBAdd(Field);
  }
  genDBAddPad(Table->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void SetData(DataBuilder &dBuild, char *name=\"%s\")\n"
                    , NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Table->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    genDBSet(Field);
  }
  genDBSetPad(Table->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadTableXML(PYYTable Table)
{
  int i, done=0;
  char Work[512];
  fprintf(HeaderFile, "  #if defined(_TBUFFER_H_)\n");
  fprintf(HeaderFile, "  void ToXML(TBAmp &XRec, char *Attr=0, char *Outer=\"%s\")\n"
                    , NameOf(Table)
         );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    XRec.clear();\n");
  fprintf(HeaderFile, "    XRec.append(\"<\");XRec.append(Outer);if (Attr) XRec.append(Attr);XRec.append(\">\\n\");\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (done == 0 && (NeedsSwap(Field) || Field->Type == ftypeTinyint))
    {
       fprintf(HeaderFile, "    char Work[32];\n");
       done = 1;
    }
    fprintf(HeaderFile, "    XRec.append(\"  <%s>\");%s;XRec.append(\"</%s>\\n\");\n"
                      , NameOf(Field)
                      , ToXMLFormat(Work, Field)
                      , NameOf(Field)
         );
  }
  fprintf(HeaderFile, "    XRec.append(\"</\");XRec.append(Outer);XRec.append(\">\\n\");\n");
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
  fprintf(HeaderFile, "  #if defined(__XMLRECORD_H__)\n");
  fprintf(HeaderFile, "  void FromXML(TBAmp &XRec)\n");
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    TXMLRecord msg;\n");
  fprintf(HeaderFile, "    TBAmp work;\n");
  fprintf(HeaderFile, "    msg.Load(XRec);\n");
  fprintf(HeaderFile, "    memset(this, 0, sizeof(*this));\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(HeaderFile, "    msg.GetValue(\"%s\", work);%s;\n"
                      , NameOf(Field)
                      , FromXMLFormat(Work, Field)
           );
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadTableSwaps(PYYTable Table)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_NEDGEN_H_)\n");
  fprintf(HeaderFile, "  void Swaps()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (NeedsSwap(Field))
      fprintf(HeaderFile, "    SwapBytes(%s);\n", NameOf(Field));
    else if (Field->Type == ftypeImage || Field->Type == ftypeImagePtr)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeBLOB
         ||  Field->Type == ftypeCLOB
         ||  Field->Type == ftypeXMLTYPE)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeZLOB)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n"
                          "    SwapBytes(%s.actual);\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void Trims()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (NeedsTrim(Field))
      fprintf(HeaderFile, "    TrimTrailingBlanks(%s, sizeof(%s));\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKeyPop(PYYTable Table, PYYKey Key)
{
  int i;
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "#if defined(_DBPORTAL_H_)\n");
  fprintf(HeaderFile, "  void Populate(t%s%s &Rec)\n"
                    , US, NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
      if (Field->Type == ftypeFloat
      ||  Field->Type == ftypeInt
      ||  Field->Type == ftypeLong
      ||  Field->Type == ftypeSmallint
      ||  Field->Type == ftypeTinyint
      ||  Field->Type == ftypeBoolean)
        fprintf(HeaderFile, "    %s = Rec.%s;\n", NameOf(Field), NameOf(Field));
      else if (AmpType(Field) == 1)
        fprintf(HeaderFile, "    memcpy(&%s, Rec.%s, sizeof(%s));\n", NameOf(Field), NameOf(Field), NameOf(Field));
      else
        fprintf(HeaderFile, "    strncpyz(%s, Rec.%s, sizeof(%s)-1);\n", NameOf(Field), NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  if (Table->CppHeader == 0)
    fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKeyDL(PYYTable Table, PYYKey Key)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_DATABUILD_H_) || defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "  void BuildData(DataBuilder &dBuild, char *name=\"%sKey\")\n"
                    , NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Key->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    genDBAdd(Field);
  }
  genDBAddPad(Key->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void SetData(DataBuilder &dBuild, char *name=\"%s\")\n"
                    , NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Key->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    genDBSet(Field);
  }
  genDBSetPad(Key->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKeyXML(PYYTable Table, PYYKey Key)
{
  int i, done = 0;
  char Work[512];
  fprintf(HeaderFile, "  #if defined(_TBUFFER_H_)\n");
  fprintf(HeaderFile, "  void ToXML(TBAmp &XRec, char *Attr=0, char *Outer=\"%sKey\")\n"
                    , NameOf(Table)
         );
  fprintf(HeaderFile, "  {\n");
  //fprintf(HeaderFile, "    char Work[32];\n");
  fprintf(HeaderFile, "    XRec.clear();\n");
  fprintf(HeaderFile, "    XRec.append(\"<\");XRec.append(Outer);if (Attr) XRec.append(Attr);XRec.append(\">\\n\");\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (done == 0 && (NeedsSwap(Field) || Field->Type == ftypeTinyint))
    {
       fprintf(HeaderFile, "    char Work[32];\n");
       done = 1;
    }
    fprintf(HeaderFile, "    XRec.append(\"  <%s>\");%s;XRec.append(\"</%s>\\n\");\n"
                      , NameOf(Field)
                      , ToXMLFormat(Work, Field)
                      , NameOf(Field)
         );
  }
  fprintf(HeaderFile, "    XRec.append(\"</\");XRec.append(Outer);XRec.append(\">\\n\");\n");
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
  fprintf(HeaderFile, "  #if defined(__XMLRECORD_H__)\n");
  fprintf(HeaderFile, "  void FromXML(TBAmp &XRec)\n");
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    TXMLRecord msg;\n");
  fprintf(HeaderFile, "    TBAmp work;\n");
  fprintf(HeaderFile, "    msg.Load(XRec);\n");
  fprintf(HeaderFile, "    memset(this, 0, sizeof(*this));\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(HeaderFile, "    msg.GetValue(\"%s\", work);%s;\n"
                      , NameOf(Field)
                      , FromXMLFormat(Work, Field)
           );
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKeySwaps(PYYKey Key)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_NEDGEN_H_)\n");
  fprintf(HeaderFile, "  void Swaps()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (NeedsSwap(Field))
      fprintf(HeaderFile, "    SwapBytes(%s);\n", NameOf(Field));
    else if (Field->Type == ftypeImage || Field->Type == ftypeImagePtr)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeBLOB
         ||  Field->Type == ftypeCLOB
         ||  Field->Type == ftypeXMLTYPE)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeZLOB)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n"
                          "    SwapBytes(%s.actual);\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void Trims()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (NeedsTrim(Field))
      fprintf(HeaderFile, "    TrimTrailingBlanks(%s, sizeof(%s));\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadProcDL(PYYTable Table, PYYProc Proc)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_DATABUILD_H_) || defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "  void BuildData(DataBuilder &dBuild, char *name=\"%s%s\")\n"
                    , NameOf(Table)
                    , Proc->Name
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Proc->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    genDBAdd(Field);
  }
  genDBAddPad(Proc->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void SetData(DataBuilder &dBuild, char *name=\"%s\")\n"
                    , NameOf(Table)
                    );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    dBuild.name(name);\n");
  fprintf(HeaderFile, "    #if defined(_DATABUILD2_H_)\n");
  fprintf(HeaderFile, "    dBuild.count(%d);\n", Proc->noFields);
  fprintf(HeaderFile, "    #endif\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    genDBSet(Field);
  }
  genDBSetPad(Proc->Length);
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadProcXML(PYYTable Table, PYYProc Proc)
{
  int i, done = 0;
  char Work[512];
  fprintf(HeaderFile, "  #if defined(_TBUFFER_H_)\n");
  fprintf(HeaderFile, "  void ToXML(TBAmp &XRec, char *Attr=0, char *Outer=\"%s%s\")\n"
                    , NameOf(Table)
                    , Proc->Name
         );
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    XRec.clear();\n");
  fprintf(HeaderFile, "    XRec.append(\"<\");XRec.append(Outer);if (Attr) XRec.append(Attr);XRec.append(\">\\n\");\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (done == 0 && (NeedsSwap(Field) || Field->Type == ftypeTinyint))
    {
       fprintf(HeaderFile, "    char Work[32];\n");
       done = 1;
    }
    fprintf(HeaderFile, "    XRec.append(\"  <%s>\");%s;XRec.append(\"</%s>\\n\");\n"
                      , NameOf(Field)
                      , ToXMLFormat(Work, Field)
                      , NameOf(Field)
         );
  }
  fprintf(HeaderFile, "    XRec.append(\"</\");XRec.append(Outer);XRec.append(\">\\n\");\n");
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
  fprintf(HeaderFile, "  #if defined(__XMLRECORD_H__)\n");
  fprintf(HeaderFile, "  void FromXML(TBAmp &XRec)\n");
  fprintf(HeaderFile, "  {\n");
  fprintf(HeaderFile, "    TXMLRecord msg;\n");
  fprintf(HeaderFile, "    TBAmp work;\n");
  fprintf(HeaderFile, "    msg.Load(XRec);\n");
  fprintf(HeaderFile, "    memset(this, 0, sizeof(*this));\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(HeaderFile, "    msg.GetValue(\"%s\", work);%s;\n"
                      , NameOf(Field)
                      , FromXMLFormat(Work, Field)
           );
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadProcSwaps(PYYProc Proc)
{
  int i;
  fprintf(HeaderFile, "  #if defined(_NEDGEN_H_)\n");
  fprintf(HeaderFile, "  void Swaps()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (NeedsSwap(Field))
      fprintf(HeaderFile, "    SwapBytes(%s);\n", NameOf(Field));
    else if (Field->Type == ftypeImage || Field->Type == ftypeImagePtr)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeBLOB
         ||  Field->Type == ftypeCLOB
         ||  Field->Type == ftypeXMLTYPE)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n", NameOf(Field));
    else if (Field->Type == ftypeZLOB)
      fprintf(HeaderFile, "    SwapBytes(%s.len);\n"
                          "    SwapBytes(%s.actual);\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  void Trims()\n");
  fprintf(HeaderFile, "  {\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (NeedsTrim(Field))
      fprintf(HeaderFile, "    TrimTrailingBlanks(%s, sizeof(%s));\n", NameOf(Field), NameOf(Field));
  }
  fprintf(HeaderFile, "  }\n");
  fprintf(HeaderFile, "  #endif\n");
}

static void GenerateHeadKey(PYYTable Table, PYYKey Key)
{
  ushort i;
  if (Key->noFields == 0)
    return;
  fprintf(HeaderFile, CrHStructT, "typedef ", US, NameOf(Table), "Key", "");
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
      fprintf(HeaderFile, CrHStructF, i, Field->Filler);
    fprintf(HeaderFile, CrHStruct1, HeadFieldType(Field, NameOf(Table)));
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Key->Length % padvalue)
    fprintf(HeaderFile, CrHStructF, i, padvalue - pad);
  GenerateHeadKeyPop(Table, Key);
  GenerateHeadKeySwaps(Key);
  GenerateHeadKeyXML(Table, Key);
  GenerateHeadKeyDL(Table, Key);
  if (Table->ExtendedC == 1 || Table->MarkedExtC == 1)
    GenerateHeadKeyExtended(Table);
  fprintf(HeaderFile, CrHStruct4,
          US, NameOf(Table),
          US, NameOf(Table));
  Table->noBuffers++;
}

static void GenerateHeadProc(PYYTable Table, PYYProc Proc)
{
  ushort i;

  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  fprintf(HeaderFile, CrHStructT, "typedef ",
                      US, NameOf(Table),
                      US, Proc->Name);
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
      fprintf(HeaderFile, CrHStructF, i, Field->Filler);
    fprintf(HeaderFile, CrHStruct1, HeadFieldType(Field, NameOf(Table)));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(HeaderFile, CrHStructN, Field->Name);
  }
  int padvalue = 4, pad;
  if (Table->EightByte)
    padvalue = 8;
  if (pad = Proc->Length % padvalue)
    fprintf(HeaderFile, CrHStructF, i, padvalue - pad);
  GenerateHeadProcSwaps(Proc);
  GenerateHeadProcXML(Table, Proc);
  GenerateHeadProcDL(Table, Proc);
  if (Table->ExtendedC == 1 || Table->MarkedExtC == 1)
    GenerateHeadProcExtended(Table, Proc);
  fprintf(HeaderFile, CrHStruct3,
          US, NameOf(Table),
          US, Proc->Name,
          US, NameOf(Table),
          US, Proc->Name);
  if (Table->ExtendedC  == 1 || Table->MarkedExtC == 1)
    GenerateHeadProcOthers(Table, Proc);
  Table->noBuffers++;
}

static void GenerateHeadBottom(void)
{
  fprintf(HeaderFile, CrHBottom);
}

static void GenerateReadOne(ushort noFields, PYYField Fields,
                            PYYTable Table, const char *ProcName, int All)
{
  int i, todo=1, size=0;
  if (!All)
  {
    todo = 0;
    for (i=0; i<noFields; i++)
    {
      PYYField Field = &Fields[i];
      if (Field->isInput)
      {
        todo = 1;
        break;
      }
    }
  }
  fprintf(HeaderFile, "  bool %sReadOne(tDBConnect& C)\n"
                      "  {\n", ProcName);
  fprintf(HeaderFile, "    return C.ReadOne(\"%s%s%s\", this);\n"
                      "  }\n", NameOf(Table), US, ProcName);
  if (!todo)
    return;
  size += fprintf(HeaderFile, "  bool %s(tDBConnect& C", ProcName);
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (All || Field->isInput)
      size += fprintf(HeaderFile, ", %s", HeadListType(Field, NameOf(Table)));
    if (size > 74)
      size = fprintf(HeaderFile, "\n        ");
  }
  fprintf(HeaderFile, ")\n"
                      "  {\n");
  for (i=0; i<noFields; i++)
  {
    PYYField Field = &Fields[i];
    if (All || Field->isInput)
    {
      if (Field->Type == ftypeFloat
      ||  Field->Type == ftypeInt
      ||  Field->Type == ftypeLong
      ||  Field->Type == ftypeSmallint
      ||  Field->Type == ftypeTinyint
      ||  Field->Type == ftypeBoolean)
        fprintf(HeaderFile, "    %s = a%s;\n", NameOf(Field), NameOf(Field));
      else if (AmpType(Field) == 1)
        fprintf(HeaderFile, "    memcpy(&%s, a%s, sizeof(%s));\n", NameOf(Field), NameOf(Field), NameOf(Field));
      else
        fprintf(HeaderFile, "    strncpyz(%s, a%s, sizeof(%s)-1);\n", NameOf(Field), NameOf(Field), NameOf(Field));
    }
  }
  fprintf(HeaderFile, "    return C.ReadOne(\"%s%s%s\", this);\n"
                       "  }\n", NameOf(Table), US, ProcName);
}

static char *ToXMLFormat(char *Work, PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    sprintf(Work, "XRec.ampappend(%s)", NameOf(Field));
    return Work;
  case ftypeHugeCHAR:
    sprintf(Work, "XRec.ampappend(%s.data)", NameOf(Field));
    return Work;
  case ftypeTinyint:
  case ftypeBoolean:
  case ftypeSmallint:
  case ftypeInt:
  case ftypeLong:
    sprintf(Work, "sprintf(Work, \"%%d\", %s);XRec.append(Work)", NameOf(Field));
    return Work;
  case ftypeFloat:
    sprintf(Work, "sprintf(Work, \"%%0.15g\", %s);XRec.append(Work)", NameOf(Field));
    return Work;
  }
  strcpy(Work, "XRec.append(\"unhandled\")");
  return Work;
}

static char *FromXMLFormat(char *Work, PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    sprintf(Work, "memcpy(%s, work.data, sizeof(%s)-1)", NameOf(Field), NameOf(Field));
    return Work;
  case ftypeHugeCHAR:
    sprintf(Work, "%s = work.data", NameOf(Field));
    return Work;
  case ftypeTinyint:
    sprintf(Work, "%s = (int8)atoi(work.data)", NameOf(Field));
    return Work;
  case ftypeBoolean:
  case ftypeSmallint:
    sprintf(Work, "%s = (int16)atoi(work.data)", NameOf(Field));
    return Work;
  case ftypeInt:
    sprintf(Work, "%s = atoi(work.data)", NameOf(Field));
    return Work;
  case ftypeLong:
    sprintf(Work, "%s = atol(work.data)", NameOf(Field));
    return Work;
  case ftypeFloat:
    sprintf(Work, "%s = atof(work.data)", NameOf(Field));
    return Work;
  }
  strcpy(Work, "/*unhandled*/");
  return Work;
}

static char *HeadFieldType(PYYField Field, const char *table)
{
  static char work[80], direction[20];
  char *name = NameOf(Field);
  sprintf(direction, "/* %s%s */",
          Field->isInput?"i":" ",
          Field->isOutput?"o":" ");
  switch (Field->Type)
  {
  case ftypeBinary:
    sprintf(work, "unsigned char %s[%hu]; %s", name, Field->Length, direction);
    return work;
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    sprintf(work, "char   %s[%hu]; %s", name, Field->Length, direction);
    return work;
  case ftypeDate:
    sprintf(work, "char   %s[9]; %s", name, direction);
    return work;
  case ftypeDateTime:
    sprintf(work, "char   %s[15]; %s", name, direction);
    return work;
  case ftypeTinyint:
    sprintf(work, "int8   %s; %s", name, direction);
    return work;
  case ftypeBoolean:
  case ftypeSmallint:
    sprintf(work, "int16  %s; %s", name, direction);
    return work;
  case ftypeInt:
    sprintf(work, "int32  %s; %s", name, direction);
    return work;
  case ftypeLong:
    sprintf(work, "int64  %s; %s", name, direction);
    return work;
  case ftypeFloat:
    sprintf(work, "double %s; %s", name, direction);
    return work;
  case ftypeVarNum:
    sprintf(work, "char   %s[22]; %s", name, direction);
    return work;
  case ftypeImage:
    sprintf(work, "struct t%s {int16 len;unsigned char data[%u];} %s; %s",
            name, Field->Length-2,
            name, direction);
    return work;
  case ftypeImagePtr:
    sprintf(work, "struct t%s {int16 len;unsigned char data[1];} %s; %s",
            name, name, direction);
    return work;
  case ftypeCLOB:
    sprintf(work, "struct t%s {int32 len; char data[%d];} %s;",
            name, Field->Length-4, name);
    return work;
  case ftypeXMLTYPE:
    sprintf(work, "struct t%s {int32 len; char data[%d];} %s;",
            name, Field->Length-4, name);
    return work;
  case ftypeBLOB:
    sprintf(work, "struct t%s {int32 len; unsigned char data[%d];} %s;",
            name, Field->Length-4, name);
    return work;
  case ftypeZLOB:
    sprintf(work, "struct t%s {int32 len; int actual; unsigned char data[%d];} %s;",
            name, Field->Length-8, name);
    return work;
  case ftypeHugeCHAR:
    sprintf(work, "tHugeCHAR %s;", name);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *HeadListType(PYYField Field, const char *table)
{
  static char work[80];
  char *name = NameOf(Field);
  switch (Field->Type)
  {
  case ftypeBinary:
    sprintf(work, "const unsigned char *a%s", name);
    return work;
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    sprintf(work, "const char *a%s", name);
    return work;
  case ftypeDate:
    sprintf(work, "const char *a%s", name);
    return work;
  case ftypeDateTime:
    sprintf(work, "const char *a%s", name);
    return work;
  case ftypeTinyint:
    sprintf(work, "int8 a%s", name);
    return work;
  case ftypeBoolean:
  case ftypeSmallint:
    sprintf(work, "int16 a%s", name);
    return work;
  case ftypeInt:
    sprintf(work, "int32 a%s", name);
    return work;
  case ftypeLong:
    sprintf(work, "int64 a%s", name);
    return work;
  case ftypeFloat:
    sprintf(work, "double a%s", name);
    return work;
  case ftypeImage:
  case ftypeImagePtr:
    sprintf(work, "void *a%s", name);
    return work;
  case ftypeHugeCHAR:
    sprintf(work, "tHugeCHAR &a%s", name);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *VBFieldType(PYYField Field)
{
  static char work[80], direction[20];
  char *name = NameOf(Field);
  char plus[] = "\0NDER";
  if (name[0] == '_')
    plus[0] = 'U';
  sprintf(direction, "' %s%s",
          Field->isInput?"i":" ",
          Field->isOutput?"o":" ");
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    sprintf(work, "%s%s As String * %hu  %s", plus, name, Field->Length, direction);
    return work;
  case ftypeDate:
    sprintf(work, "%s%s As String * 9  %s", plus, name, direction);
    return work;
  case ftypeDateTime:
    sprintf(work, "%s%s As String * 15  %s", plus, name, direction);
    return work;
  case ftypeBoolean:
    sprintf(work, "%s%s As Byte  %s", plus, name, direction);
    return work;
  case ftypeTinyint:
    sprintf(work, "%s%s As Byte  %s", plus, name, direction);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s%s As Integer  %s", plus, name, direction);
    return work;
  case ftypeInt:
    sprintf(work, "%s%s As Long  %s", plus, name, direction);
    return work;
  case ftypeFloat:
    sprintf(work, "%s%s As Double %s", plus, name, direction);
    return work;
  case ftypeImage:
    sprintf(work, "%s%s As String * %hu  %s", plus, name, Field->Length, direction);
    return work;
  }
  strcpy(work, "unhandled");
  return work;
}

static char *VB5ConstrFieldType(PYYField Field, bool isArray)
{
  static char work[80];
  char *name = NameOf(Field);
  const char *array = "";
  char plus[] = "\0NDER";
  if (name[0] == '_')
    plus[0] = 'U';
  if (isArray) array = "(0)";
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeImage:
    sprintf(work, "fRec%s.%s%s = \"\"", array, plus, name);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeInt:
  case ftypeFloat:
    sprintf(work, "fRec%s.%s%s = 0", array, plus, name);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *VB5GetFieldType(PYYField Field, bool isArray)
{
  static char work[80];
  char *name = NameOf(Field);
  const char *array = "";
  char plus[] = "\0NDER";
  if (name[0] == '_')
    plus[0] = 'U';
  if (isArray) array = "aIndex As Long";
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    sprintf(work, "%s%s(%s) As String",plus , name, array);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "%s%s(%s) As Byte",plus , name, array);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s%s(%s) As Integer",plus , name, array);
    return work;
  case ftypeInt:
    sprintf(work, "%s%s(%s) As Long",plus , name, array);
    return work;
  case ftypeFloat:
    sprintf(work, "%s%s(%s) As Double",plus , name, array);
    return work;
  case ftypeImage:
    sprintf(work, "%s%s(%s) As String",plus , name, array);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *VB5SetFieldType(PYYField Field, bool isArray)
{
  static char work[80];
  char *name = NameOf(Field);
  char plus[] = "\0NDER";
  if (name[0] == '_')
    plus[0] = 'U';
  const char *array = "";
  if (isArray) array = "aIndex As Long, ";
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    sprintf(work, "%s%s(%saValue As String)",plus , name, array);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "%s%s(%saValue As Byte)",plus , name, array);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s%s(%saValue As Integer)",plus , name, array);
    return work;
  case ftypeInt:
    sprintf(work, "%s%s(%saValue As Long)",plus , name, array);
    return work;
  case ftypeFloat:
    sprintf(work, "%s%s(%saValue As Double)",plus , name, array);
    return work;
  case ftypeImage:
    sprintf(work, "%s%s(%saValue As String)",plus , name, array);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static bool VB5FieldIsString(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeImage:
    return true;
  default:
    return false;
  }
}

static char *VBParmType(PYYField Field)
{
  static char work[80];
  char *name = NameOf(Field);
  char plus[] = "\0NDER";
  if (name[0] == '_')
    plus[0] = 'U';
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeImage:
    sprintf(work, "%s%s As String", plus, name);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "%s%s As Byte", plus, name);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s%s As Integer", plus, name);
    return work;
  case ftypeInt:
    sprintf(work, "%s%s As Long", plus, name);
    return work;
  case ftypeFloat:
    sprintf(work, "%s%s As Double",plus, name);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *PASFieldType(PYYField Field)
{
  static char work[80], direction[20];
  char *name = NameOf(Field);

  sprintf(direction, "{ %s%s }",
          Field->isInput?"i":" ",
          Field->isOutput?"o":" ");

  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
    sprintf(work, "%-20s : array[0..%hu] of char; %s", name, Field->Length-1, direction);
    return work;
  case ftypeDate:
    sprintf(work, "%-20s : array[0..8] of char; %s", name, direction);
    return work;
  case ftypeDateTime:
    sprintf(work, "%-20s : array[0..14] of char; %s", name, direction);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "%-20s : Shortint; %s", name, direction);
    return work;
  case ftypeSmallint:
    sprintf(work, "%-20s : Smallint; %s", name, direction);
    return work;
  case ftypeInt:
    sprintf(work, "%-20s : Longint; %s", name, direction);
    return work;
  case ftypeFloat:
    sprintf(work, "%-20s : Double; %s", name, direction);
    return work;
  case ftypeImage:
    sprintf(work, "%-20s : record\n"
                  "    length : word;\n"
                  "    data : array [0..%hu] of char;\n"
                  "  end; %s", name, Field->Length-3, direction);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static char *PASParmType(PYYField Field)
{
  static char work[80];
  char *name = NameOf(Field);

  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    sprintf(work, "%s : pchar", name);
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
    sprintf(work, "%s : Shortint", name);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s : Smallint", name);
    return work;
  case ftypeInt:
    sprintf(work, "%s : Longint", name);
    return work;
  case ftypeFloat:
    sprintf(work, "%s : Double", name);
    return work;
  }
  strcpy(work, "junk");
  return work;
}

static bool PASispchar(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
  case ftypeDynamic:
  case ftypeDate:
  case ftypeDateTime:
    return true;
  }
  return false;
}

int NullableField(PYYField Field)
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
  ||  Field->Type == ftypeVarNum)
    return 1;
  return 0;
}

static int NeedsSwap(PYYField Field)
{
  if (Field->Type == ftypeFloat
  ||  Field->Type == ftypeInt
  ||  Field->Type == ftypeLong
  ||  Field->Type == ftypeBoolean
  ||  Field->Type == ftypeSmallint)
    return 1;
  return 0;
}

static int NeedsTrim(PYYField Field)
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
    return 1;
  return 0;
}






