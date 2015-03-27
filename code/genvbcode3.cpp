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

static FILE *VBCode3File  = 0;

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

static const char *VBFieldType(PYYField Field, bool StringSize=true)
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
    if (StringSize)
      sprintf(work, "%s As String * %hu", name, Field->Length);
    else
      sprintf(work, "%s As String", name);
    return work;
  case ftypeDate:
    if (StringSize)
      sprintf(work, "%s As String * 9", name);
    else
      sprintf(work, "%s As String", name);
    return work;
  case ftypeDateTime:
    if (StringSize)
      sprintf(work, "%s As String * 15", name);
    else
      sprintf(work, "%s As String", name);
    return work;
  case ftypeBoolean:
    sprintf(work, "%s As Byte", name);
    return work;
  case ftypeTinyint:
    sprintf(work, "%s As Byte", name);
    return work;
  case ftypeSmallint:
    sprintf(work, "%s As Integer", name);
    return work;
  case ftypeInt:
  //case ftypeLong:
    sprintf(work, "%s As Long", name);
    return work;
  case ftypeFloat:
    sprintf(work, "%s As Double", name);
    return work;
  case ftypeImage:
    if (StringSize)
      sprintf(work, "%s As String * %hu", name, Field->Length);
    else
      sprintf(work, "%s As String", name);
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

static int OpenVBFile(const char *VBFileName)
{
  VBCode3File = OpenFile("VB File", VBFileName);
  if (VBCode3File == 0)
    return 1;
  return 0;
}

static void GenerateVBTop(const char *FName)
{
  fprintf(VBCode3File, "ATTRIBUTE VB_NAME=\"M%s\"\n"
                       "' Please do not modify this, modify the source then regenerate\n"
                       "Option Explicit\n\n", FName);
}

static void GenerateVBClassTop(const char *FName)
{
  fprintf(VBCode3File, "VERSION 1.0 CLASS\n"
                       "BEGIN\n"
                       "  MultiUse = -1\n"
                       "END\n"
                       "ATTRIBUTE VB_NAME=\"C%s\"\n"
                       "Option explicit\n"
                       "' This code was generated, do not modify it, "
                       "modify it at source and regenerate it.\n\n", FName);
  fprintf(VBCode3File, "Private Erc      As Integer\n"
                       "Private VRec()   As V%s\n"
                       "Private FRec     As T%s\n"
                       "Public  RecCount As Long\n",
                       FName, FName);
}

static void GenerateVBInit(const char *FName)
{
  fprintf(VBCode3File, "Private Sub Class_Initialize()\n"
                       "  RecClear\n"
                       "End Sub\n\n"
                       "Private Sub Class_Terminate()\n"
                       "  Erase VRec\n"
                       "End Sub\n\n"
                       "Public Sub RecClear()\n"
                       "  ReDim Preserve VRec(0 to 0)\n"
                       "  RecCount = 0\n"
                       "End Sub\n\n"
                       "Friend Property Get Rec(Optional Index As Long = 0) As V%s\n"
                       "  Rec = VRec(Index)\n"
                       "End Property\n\n"
                       "Friend Property Let Rec(Optional Index As Long = 0, "
                                               "Value As V%s)\n"
                       "  VRec(Index) = Value\n"
                       "End Property\n\n",
                       FName, FName);
}

static void VB5NoOutputProc(PYYTable Table, const char *ProcName, const char *RecName, bool doToVRec=true)
{
  fprintf(VBCode3File, "Public Sub %s(Connect As TConnect)\n"
                       "  FromVRec\n"
                       "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", FRec)\n",
                       ProcName,
                       Dehash(Table->Name), US, ProcName);
  if (doToVRec)
    fprintf(VBCode3File, "  ToVRec\n");
  fprintf(VBCode3File, "  If Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\", Connect.ErrorMsg(Erc)\n"
                       "End Sub\n\n",
                       Dehash(Table->Name), US, ProcName);
}

static void VB5SingleProc(PYYTable Table, const char *ProcName, const char *RecName)
{
  fprintf(VBCode3File, "Public Function %s(Connect As TConnect) As Boolean\n"
                       "  FromVRec\n"
                       "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", FRec)\n"
                       "  ToVRec\n"
                       "  %s = Connect.CheckSingle(Erc, \"%s%s%s\")\n"
                       "End Function\n\n",
                       ProcName,
                       Dehash(Table->Name), US, ProcName,
                       ProcName, Dehash(Table->Name), US, ProcName);
}

static void VB5MultiProc(PYYTable Table, const char *ProcName, const char *RecName)
{
  fprintf(VBCode3File, "Public Sub %s(Connect As TConnect)\n"
                       "  Set %sCursor = New TCursor\n"
                       "  Dim Handle As Long\n"
                       "  FromVRec\n"
                       "  Erc = DBOpenExec(Connect.Handle, Handle, \"%s%s%s\", FRec)\n"
                       "  %sCursor.Handle = Handle\n"
                       "  If Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\", %sCursor.ErrorMsg(Erc)\n"
                       "End Sub\n\n",
                       ProcName,
                       ProcName,
                       Dehash(Table->Name), US, ProcName,
                       ProcName,
                       Dehash(Table->Name), US, ProcName, ProcName);
  fprintf(VBCode3File, "Public Function %sFetch() As Boolean\n"
                       "  if %sCursor Is Nothing then Err.Raise SQLapiCursorErr, \"%s%sNext%s\","
                       " \"Run %s%s%s first\"\n",
                       ProcName,
                       ProcName, Dehash(Table->Name), US, ProcName,
                       Dehash(Table->Name), US, ProcName);
  fprintf(VBCode3File, "  Erc = DBFetchData(%sCursor.Handle, FRec)\n"
                       "  ToVRec\n"
                       "  %sFetch = %sCursor.CheckEOF(Erc, \"%s%s%s\")\n"
                       "End Function\n\n",
                       ProcName,
                       ProcName, ProcName, Dehash(Table->Name), US, ProcName);
  fprintf(VBCode3File, "Public Sub %sLoad(Connect As TConnect)\n"
                       "  RecCount = 0\n"
                       "  ReDim Preserve VRec(0 to 64)\n"
                       "  %s Connect\n"
                       "  Do While %sFetch\n"
                       "    RecCount = RecCount + 1\n"
                       "    if RecCount > Ubound(VRec) Then "
                       "ReDim Preserve VRec(0 to RecCount * 2)\n"
                       "    VRec(RecCount) = VRec(0)\n"
                       "  Loop\n"
                       "  ReDim Preserve VRec(0 to RecCount)\n"
                       "End Sub\n\n",
                       ProcName,
                       ProcName,
                       ProcName);
}

static void VB5NoIOProc(PYYTable Table, const char *ProcName)
{
  fprintf(VBCode3File, "Public Sub %s(Connect As TConnect)\n"
                       "  Erc = DBRunProc(Connect.Handle, \"%s%s%s\", 0)\n"
                       "  if Erc <> SQLapiOK then Err.Raise Erc, \"%s%s%s\","
                       " Connect.ErrorMsg(Erc)\n"
                       "End Sub\n\n",
                       ProcName,
                       Dehash(Table->Name), US, ProcName,
                       Dehash(Table->Name), US, ProcName);
}

static void MakeVBName(PYYTable Table, char *fname)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->VBCode3Dir))
    FNameMerge(newpath, Table->VBCode3Dir, fname, Table->VBCode3Ext);
  else
    FNameMerge(newpath, dir, fname, Table->VBCode3Ext);
  Table->VBCode3FileName = strdup(newpath);
}

int MakeVBClassName(PYYTable Table, const char *funcname)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (strlen(Table->VBCode3Dir))
    FNameMerge(newpath, Table->VBCode3Dir, funcname, Table->VBCode3Ext);
  else
    FNameMerge(newpath, dir, funcname, Table->VBCode3Ext);
  fclose(VBCode3File);
  free(Table->VBCode3FileName);
  Table->VBCode3FileName = strdup(newpath);
  int rc = OpenVBFile(Table->VBCode3FileName);
  if (rc)
    return rc;
  GenerateVBClassTop(funcname);
  return 0;
}

static void GenerateVBTable(PYYTable Table)
{
  ushort i;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(VBCode3File, "Public Const %s%s%s = %d\n"
                  , Table->Name
                  , NameOf(Field)
                  , Const->Name
                  , Const->Value
                  );
      }
      fprintf(VBCode3File, "\n");
    }
  }
  fprintf(VBCode3File,      "Type T%s%s\n", US, Dehash(Table->Name));
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->Filler)
       fprintf(VBCode3File, "  Filler%d As String * %d\n", i, Field->Filler);
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  %sIsNull As Integer\n", Field->Name);
  }
  if (Table->Length % 4)
    fprintf(VBCode3File,    "  Filler%d As String * %d\n", i, 4 - Table->Length % 4);
  fprintf(VBCode3File,      "End Type\n\n");
  fprintf(VBCode3File,      "Type V%s%s\n", US, Dehash(Table->Name));
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  %sIsNull As Boolean\n", Field->Name);
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field, false));
  }
  fprintf(VBCode3File,      "End Type\n\n");
  Table->noBuffers++;
}

static void GenerateVBTableConsts(PYYTable Table)
{
  int i;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(VBCode3File, "Function Get%s%sDescr(No As Integer) As String\n"
                           "  Select Case No\n"
                  , Table->Name
                  , NameOf(Field));
      for (int f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(VBCode3File, "    Case %s%s%s\n"
                             "       Get%s%sDescr = \"%s\"\n"
                  , Table->Name
                  , NameOf(Field)
                  , Const->Name
                  , Table->Name
                  , NameOf(Field)
                  , Const->Name);
      }
      fprintf(VBCode3File, "  End Select\n"
                           "End Function\n\n");
    }
  }
}

static bool VBCursorVars(const char *ProcName)
{
  fprintf(VBCode3File, "Private %sCursor as TCursor\n", ProcName);
  return true;
}

static void GenerateVBTableProcs(PYYTable Table)
{
  ushort i;
  if (hasStdRtns(Table))
  {
    bool isArrayed = false;
    if (Table->hasSelectAll)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectAll & doVBCODE3)
        VBCursorVars("SelectAll");
    if (Table->hasSelectAllUpd)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectAllUpd & doVBCODE3)
        VBCursorVars("SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isFetch)
        continue;
      // Must be MarkedVBCode3 if TargetVBCode3 == 0
      if (Table->TargetVBCode3 == 0 && (Proc->extProc & doVBCODE3) == 0)
        continue;
      if (Proc->useStd)
        VBCursorVars(Proc->Name);
    }
    fprintf(VBCode3File, "\n");
    GenerateVBInit(Table->Name);
    if (Table->hasInsert)
      if (Table->MarkedVBCode3 == 0 || Table->extInsert & doVBCODE3)
        VB5NoOutputProc(Table, "Insert", Dehash(Table->Name));
    if (Table->hasUpdate)
      if (Table->MarkedVBCode3 == 0 || Table->extUpdate & doVBCODE3)
        VB5NoOutputProc(Table, "Update", Dehash(Table->Name));
    if (Table->hasDeleteAll)
      if (Table->MarkedVBCode3 == 0 || Table->extDeleteAll & doVBCODE3)
        VB5NoIOProc(Table,     "DeleteAll");
    if (Table->hasSelectOne)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectOne & doVBCODE3)
        VB5SingleProc(Table,   "SelectOne", Dehash(Table->Name));
    if (Table->hasSelectOneUpd)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectOneUpd & doVBCODE3)
        VB5SingleProc(Table,   "SelectOneUpd", Dehash(Table->Name));
    if (Table->hasSelectAll)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectAll & doVBCODE3)
        VB5MultiProc(Table,    "SelectAll", Dehash(Table->Name));
    if (Table->hasSelectAllUpd)
      if (Table->MarkedVBCode3 == 0 || Table->extSelectAllUpd & doVBCODE3)
        VB5MultiProc(Table,    "SelectAllUpd", Dehash(Table->Name));
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd)
        continue;
      // Must be MarkedVBCode3 if TargetVBCode3 == 0
      if (Table->TargetVBCode3 == 0 && (Proc->extProc & doVBCODE3) == 0)
        continue;
      if (Proc->noFields == 0)
        VB5NoIOProc(Table, Proc->Name);
      else if (Proc->useStd)
      {
        if (Proc->isSql && Proc->isFetch)
          VB5SingleProc(Table, Proc->Name, Dehash(Table->Name));
        else if (Proc->isSql && Proc->noOutputs > 0)
          VB5MultiProc(Table, Proc->Name, Dehash(Table->Name));
        else
          VB5NoOutputProc(Table, Proc->Name, Dehash(Table->Name));
      }
    }
    fprintf(VBCode3File, "Private Sub FromVRec()\n");
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      if (Table->isNullEnabled && Field->isNull && NullableField(Field))
        fprintf(VBCode3File,  "  If FRec.%sIsNull = 0 Then VRec(0).%sIsNull = True Else VRec(0).%sIsNull = False\n"
          , Field->Name, Field->Name, Field->Name);
        fprintf(VBCode3File,  "  FRec.%s = VRec(0).%s\n", Field->Name, Field->Name);
    }
    fprintf(VBCode3File,      "End Sub\n\n");
    fprintf(VBCode3File,      "Private Sub ToVRec()\n");
    for (i=0; i < Table->noFields; i++)
    {
      PYYField Field = &Table->Fields[i];
      if (Table->isNullEnabled && Field->isNull && NullableField(Field))
        fprintf(VBCode3File,  "  If VRec(0).%sIsNull Then FRec.%sIsNull = 1 Else FRec.%sIsNull = 0\n"
          , Field->Name, Field->Name, Field->Name);
      if (isString(Field))
        fprintf(VBCode3File,  "  VRec(0).%s = RTrim$(FRec.%s)\n", Field->Name, Field->Name);
      else
        fprintf(VBCode3File,  "  VRec(0).%s = FRec.%s\n", Field->Name, Field->Name);
    }
    fprintf(VBCode3File,      "End Sub\n\n");
  }
}

static void GenerateVBKey(PYYTable Table, PYYKey Key)
{
  ushort i;
  if (Key->noFields == 0)
    return;
  fprintf(VBCode3File,      "Type T%s%sKEY\n", US, Dehash(Table->Name));
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Field->Filler)
       fprintf(VBCode3File, "  Filler%d As String * %d\n", i, Field->Filler);
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field));
  }
  if (Key->Length % 4)
    fprintf(VBCode3File,    "  Filler%d As String * %d\n", i, 4 - Key->Length % 4);
  fprintf(VBCode3File,      "End Type\n\n");
  fprintf(VBCode3File,      "Type V%s%sKEY\n", US, Dehash(Table->Name));
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field, false));
  }
  fprintf(VBCode3File,      "End Type\n\n");
  Table->noBuffers++;
}

static void GenerateVBKeyProcs(PYYTable Table, PYYKey Key)
{
  char Work[256];
  if (Key->noFields == 0)
    return;
  sprintf(Work, "%s%sKEY", Dehash(Table->Name), US);
  MakeVBClassName(Table, Work);
  GenerateVBInit(Work);
  fprintf(VBCode3File,      "Private Sub FromVRec()\n");
  for (int i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBCode3File,    "  FRec.%s = VRec(0).%s\n", Field->Name, Field->Name);
  }
  fprintf(VBCode3File,      "End Sub\n\n");
  VB5NoOutputProc(Table, "DeleteOne", Work, false);
}

static void GenerateVBProc(PYYTable Table, PYYProc Proc)
{
  ushort i;
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  // Must be MarkedVBCode3 if TargetVBCode3 == 0
  if (Table->TargetVBCode3 == 0 && (Proc->extProc & doVBCODE3) == 0)
    return;
  fprintf(VBCode3File,      "Type T%s%s%s%s\n", US, Dehash(Table->Name), US, Proc->Name);
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Field->Filler)
       fprintf(VBCode3File, "  Filler%d As String * %d\n", i, Field->Filler);
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field));
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  %sIsNull As Integer\n", Field->Name);
  }
  if (Proc->Length % 4)
    fprintf(VBCode3File,    "  Filler%d As String * %d\n", i, 4 - Proc->Length % 4);
  fprintf(VBCode3File,      "End Type\n\n");
  fprintf(VBCode3File,      "Type V%s%s%s%s\n", US, Dehash(Table->Name), US, Proc->Name);
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  %sIsNull As Boolean\n", Field->Name);
    fprintf(VBCode3File,    "  %s\n", VBFieldType(Field, false));
  }
  fprintf(VBCode3File,      "End Type\n\n");
  Table->noBuffers++;
}

static void GenerateVBProcProcs(PYYTable Table, PYYProc Proc)
{
  int i;
  char Work[256];
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  // Must be MarkedVBCode3 if TargetVBCode3 == 0
  if (Table->TargetVBCode3 == 0 && (Proc->extProc & doVBCODE3) == 0)
    return;
  sprintf(Work, "%s%s%s", Dehash(Table->Name), US, Proc->Name);
  MakeVBClassName(Table, Work);
  if (Proc->isSql && Proc->isFetch)
  {
    fprintf(VBCode3File, "\n");
    VB5SingleProc(Table, Proc->Name, Work);
  }
  else if (Proc->isSql && Proc->noOutputs > 0)
  {
    VBCursorVars(Proc->Name);
    VB5MultiProc(Table, Proc->Name, Work);
  }
  else
  {
    fprintf(VBCode3File, "\n");
    VB5NoOutputProc(Table, Proc->Name, Work);
  }
  GenerateVBInit(Work);
  fprintf(VBCode3File,      "Private Sub FromVRec()\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  If FRec.%sIsNull = 0 Then VRec(0).%sIsNull = True Else VRec(0).%sIsNull = False\n"
        , Field->Name, Field->Name, Field->Name);
    fprintf(VBCode3File,    "  FRec.%s = VRec(0).%s\n", Field->Name, Field->Name);
  }
  fprintf(VBCode3File,      "End Sub\n\n");
  fprintf(VBCode3File,      "Private Sub ToVRec()\n");
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBCode3File,  "  If VRec(0).%sIsNull Then FRec.%sIsNull = 1 Else FRec.%sIsNull = 0\n"
        , Field->Name, Field->Name, Field->Name);
    if (isString(Field))
      fprintf(VBCode3File,  "  VRec(0).%s = RTrim$(FRec.%s)\n", Field->Name, Field->Name);
    else
      fprintf(VBCode3File,  "  VRec(0).%s = FRec.%s\n", Field->Name, Field->Name);
  }
  fprintf(VBCode3File,      "End Sub\n\n");
}

void GenerateVBCode3(PYYTable Table)
{
  char fname[512];
  MakeVBName(Table, fname);
  ushort i;
  int rc = OpenVBFile(Table->VBCode3FileName);
  if (rc)
    return;
  GenerateVBTop(fname);
  if (Table->noFields > 0)
    GenerateVBTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateVBKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateVBProc(Table, &Table->Procs[i]);
  if (Table->noFields > 0)
    GenerateVBTableConsts(Table);
  if (Table->noFields > 0)
  {
    MakeVBClassName(Table, Table->Name);
    GenerateVBTableProcs(Table);
  }
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateVBKeyProcs(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateVBProcProcs(Table, &Table->Procs[i]);
  fclose(VBCode3File);
}


