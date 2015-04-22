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

static FILE *VBNet7File  = 0;

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
      sprintf(work, "<VBFixedString(%hu)> Public %s As String", Field->Length, name);
    else
      sprintf(work, "Public %s As String", name);
    return work;
  case ftypeDate:
    if (StringSize)
      sprintf(work, "<VBFixedString(9)> Public %s As String", name);
    else
      sprintf(work, "Public %s As String", name);
    return work;
  case ftypeDateTime:
    if (StringSize)
      sprintf(work, "<VBFixedString(15)> Public %s As String", name);
    else
      sprintf(work, "Public %s As String", name);
    return work;
  case ftypeBoolean:
    sprintf(work, "Public %s As Byte", name);
    return work;
  case ftypeTinyint:
    sprintf(work, "Public %s As SByte", name);
    return work;
  case ftypeSmallint:
    sprintf(work, "Public %s As Short", name);
    return work;
  case ftypeInt:
    sprintf(work, "Public %s As Integer", name);
    return work;
  case ftypeLong:
    sprintf(work, "Public %s As Long", name);
    return work;
  case ftypeFloat:
    sprintf(work, "Public %s As Double", name);
    return work;
  case ftypeImage:
    if (StringSize)
      sprintf(work, "<VBFixedString(%hu)> Public %s As String", Field->Length, name);
    else
      sprintf(work, "Public %s As String", name);
    return work;
  }
  return "junk";
}

static const char *VBSpaceType(PYYField Field)
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

static const char *VBMakerType(PYYField Field)
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

static int OpenVBFile(const char *VBFileName)
{
  VBNet7File = OpenFile("VB File", VBFileName);
  if (VBNet7File == 0)
    return 1;
  return 0;
}

static void GenerateVBClassTop(const char *FName)
{
  fprintf(VBNet7File,
      "Public Class C%s\n"
      "  Private Erc As Integer\n"
      "  Private VRec() As V%s\n"
      "  Private TRec As T%s\n"
      "  Public Count As Integer\n"
      , FName, FName, FName);
}

static void GenerateVBInit(const char *FName)
{
  fprintf(VBNet7File,
      "  Declare Ansi Function DBRunProc Lib \"sqlapi32.dll\" (ByVal Magic As Integer, ByVal Query As String, ByRef aData As T%s) As Integer\n"
      "  Declare Ansi Function DBOpenExec Lib \"sqlapi32.dll\" (ByVal Magic As Integer, ByRef Cursor As Integer, ByVal Query As String, ByRef aData As T%s) As Integer\n"
      "  Declare Ansi Function DBFetchData Lib \"sqlapi32.dll\" (ByVal Cursor As Integer, ByRef aData As T%s) As Integer\n\n"
      "  Public Sub New()\n"
      "    TRec = New T%s()\n"
      "    RecClear\n"
      "  End Sub\n\n"
      "  Protected Overrides Sub Finalize()\n"
      "    MyBase.Finalize()\n"
      "    Erase VRec\n"
      "  End Sub\n\n"
      "  Public Sub RecClear()\n"
      "    ReDim Preserve VRec(0)\n"
      "    Count = 0\n"
      "  End Sub\n\n"
      "  Property Rec(Optional ByVal Index As Integer = 0) As V%s\n"
      "    Get\n"
      "      Return VRec(Index)\n"
      "    End Get\n"
      "    Set(ByVal Value As V%s)\n"
      "      VRec(Index) = Value\n"
      "    End Set\n"
      "  End Property\n\n"
      , FName, FName, FName, FName, FName, FName);
}

static void VBNetNoOutputProc(PYYTable Table, const char *ProcName, const char *RecName, bool doToVRec=true)
{
  fprintf(VBNet7File,
      "  Public Sub %s(conn As Connect)\n"
      "    VRec(0).FromV(TRec)\n"
      "    Erc = DBRunProc(conn.Handle, \"%s%s%s\", TRec)\n"
      , ProcName, Dehash(Table->Name), US, ProcName);
  if (doToVRec)
    fprintf(VBNet7File,
      "    VRec(0).ToV(TRec)\n"
      );
  fprintf(VBNet7File,
      "    If Erc <> SQLapi.OK Then Throw New DBPortalException(Erc, \"%s%s%s\", conn.ErrorMsg())\n"
      "  End Sub\n\n"
      , Dehash(Table->Name), US, ProcName
      );
}

static void VBNetSingleProc(PYYTable Table, const char *ProcName, const char *RecName)
{
  fprintf(VBNet7File,
      "  Public Function %s(conn As Connect) As Boolean\n"
      "    VRec(0).FromV(TRec)\n"
      "    Erc = DBRunProc(conn.Handle, \"%s%s%s\", TRec)\n"
      "    VRec(0).ToV(TRec)\n"
      "    Return conn.CheckSingle(Erc, \"%s%s%s\")\n"
      "  End Function\n\n"
      , ProcName
      , Dehash(Table->Name), US, ProcName
      //, ProcName
      , Dehash(Table->Name), US, ProcName
      );
}

static void VBNetMultiProc(PYYTable Table, const char *ProcName, const char *RecName)
{
  fprintf(VBNet7File,
      "  Public Sub %s(conn As Connect)\n"
      "    %sCursor = New Cursor\n"
      "    Dim Handle As Integer\n"
      "    VRec(0).FromV(TRec)\n"
      "    Erc = DBOpenExec(conn.Handle, Handle, \"%s%s%s\", TRec)\n"
      "    %sCursor.Handle = Handle\n"
      "    If Erc <> SqlApi.OK Then Throw New DBPortalException(Erc, \"%s%s%s\", %sCursor.ErrorMsg())\n"
      "  End Sub\n\n"
      , ProcName
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , ProcName
      , Dehash(Table->Name), US, ProcName, ProcName
      );
  fprintf(VBNet7File,
      "  Public Function %sFetch() As Boolean\n"
      "    if %sCursor Is Nothing Then Throw New DBPortalException(0, \"%s%sNext%s\", \"Run %s%s%s first\")\n"
      , ProcName
      , ProcName, Dehash(Table->Name), US, ProcName, Dehash(Table->Name), US, ProcName
      );
  fprintf(VBNet7File,
      "    Erc = DBFetchData(%sCursor.Handle, TRec)\n"
      "    VRec(0).ToV(TRec)\n"
      "    Return %sCursor.CheckEOF(Erc, \"%s%s%s\")\n"
      "  End Function\n\n"
      , ProcName
      , ProcName, Dehash(Table->Name), US, ProcName
      );
  fprintf(VBNet7File,
      "  Public Sub %sLoad(conn As Connect)\n"
      "    Count = 0\n"
      "    ReDim Preserve VRec(64)\n"
      "    %s(conn)\n"
      "    Do While %sFetch\n"
      "      Count = Count + 1\n"
      "      if Count > Ubound(VRec) Then ReDim Preserve VRec(Count * 2)\n"
      "      VRec(Count) = VRec(0)\n"
      "    Loop\n"
      "    ReDim Preserve VRec(Count)\n"
      "  End Sub\n\n"
      , ProcName
      , ProcName
      , ProcName
      );
}

static void VBNetNoIOProc(PYYTable Table, const char *ProcName)
{
  fprintf(VBNet7File,
      "  Public Sub %s(conn As Connect)\n"
      "    Erc = DBRunProc(conn.Handle, \"%s%s%s\", 0)\n"
      "    if Erc <> SqlApi.OK Then Throw New DBPortalException(Erc, \"%s%s%s\", conn.ErrorMsg())\n"
      "  End Sub\n\n"
      , ProcName
      , Dehash(Table->Name), US, ProcName
      , Dehash(Table->Name), US, ProcName
      );
}

static void MakeVBName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char fname[512];
  char ext[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->VBNet7Dir))
    FNameMerge(newpath, Table->VBNet7Dir, fname, Table->VBNet7Ext);
  else
    FNameMerge(newpath, dir, fname, Table->VBNet7Ext);
  Table->VBNet7FileName = strdup(newpath);
}

static void GenerateVBTableConsts(PYYTable Table)
{
  int i;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      fprintf(VBNet7File,
          "  ReadOnly Property %sConstDescr(ByVal No As Integer) As String\n"
          "    Get\n"
          "      Select Case No\n"
          , NameOf(Field));
      int f;
      for (f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(VBNet7File,
            "        Case %s%s\n"
            "          Return \"%s\"\n"
            , NameOf(Field), Const->Name
            , Const->Name);
      }
      fprintf(VBNet7File,
          "      End Select\n"
          "    End Get\n"
          "  End Property\n\n"
          );
    }
  }
}

static void GenerateVBTable(PYYTable Table)
{
  ushort i;
  fprintf(VBNet7File,
      "Imports Bbd.DBPortal\n\n"
      "Public Structure T%s%s\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    fprintf(VBNet7File,
        "  %s\n", VBFieldType(Field)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "  public %sIsNull As Short\n"
          , Field->Name);
  }
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  fprintf(VBNet7File,
      "Public Structure V%s%s\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "  public %sIsNull As Boolean\n"
          , Field->Name
          );
    fprintf(VBNet7File,
        "  %s\n"
        , VBFieldType(Field, false)
        );
  }
  fprintf(VBNet7File,
      "\n  Public Sub ToV(ByRef TRec As T%s%s)\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "    If TRec.%sIsNull = 0 Then %sIsNull = True Else %sIsNull = False\n"
          , Field->Name, Field->Name, Field->Name
          );
    if (isString(Field))
      fprintf(VBNet7File,
          "    %s = RTrim$(TRec.%s)\n"
          , Field->Name, Field->Name
          );
    else
      fprintf(VBNet7File,
          "    %s = TRec.%s\n"
          , Field->Name, Field->Name
          );
  }
  fprintf(VBNet7File,
      "  End Sub\n\n"
      );
  fprintf(VBNet7File,
      "  Public Sub FromV(ByRef TRec As T%s%s)\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "    If %sIsNull Then TRec.%sIsNull = 1 Else TRec.%sIsNull = 0\n"
          , Field->Name, Field->Name, Field->Name
          );
    fprintf(VBNet7File,
        "    TRec.%s = %s\n"
        , Field->Name, VBSpaceType(Field)
        );
  }
  fprintf(VBNet7File,
      "  End Sub\n\n"
      );
  fprintf(VBNet7File,
      "  Public Const _Make_Up As String = \""
      );
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "I2"
          );
    fprintf(VBNet7File,
        "%s"
        , VBMakerType(Field)
        );
  }
  fprintf(VBNet7File,
       "\"\n\n"
       );
  int hasConsts = 0;
  for (i=0; i < Table->noFields; i++)
  {
    PYYField Field = &Table->Fields[i];
    if (Field->noConsts > 0)
    {
      hasConsts = 1;
      int f;
      for (f=0; f<Field->noConsts; f++)
      {
        PYYConst Const = &Field->Consts[f];
        fprintf(VBNet7File,
            "  Public Const %s%s = %d\n"
            , NameOf(Field), Const->Name, Const->Value
            );
      }
      fprintf(VBNet7File, "\n");
    }
  }
  if (hasConsts == 1)
    GenerateVBTableConsts(Table);
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  Table->noBuffers++;
}

static bool VBCursorVars(const char *ProcName)
{
  fprintf(VBNet7File,
      "  Private %sCursor as Cursor\n"
      , ProcName
      );
  return true;
}

static void GenerateVBTableProcs(PYYTable Table)
{
  GenerateVBClassTop(Table->Name);
  ushort i;
  if (hasStdRtns(Table))
  {
    bool isArrayed = false;
    if (Table->hasSelectAll)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectAll & doVBNET7)
        VBCursorVars("SelectAll");
    if (Table->hasSelectAllUpd)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectAllUpd & doVBNET7)
        VBCursorVars("SelectAllUpd");
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd || Proc->isSingle)
        continue;
      if (Table->TargetVBNet7 == 0 && (Proc->extProc & doVBNET7) == 0)
        continue;
      if (Proc->useStd)
        VBCursorVars(Proc->Name);
    }
    fprintf(VBNet7File, "\n");
    GenerateVBInit(Table->Name);
    if (Table->hasInsert)
      if (Table->MarkedVBNet7 == 0 || Table->extInsert & doVBNET7)
        VBNetNoOutputProc(Table, "Insert", Dehash(Table->Name));
    if (Table->hasUpdate)
      if (Table->MarkedVBNet7 == 0 || Table->extUpdate & doVBNET7)
        VBNetNoOutputProc(Table, "Update", Dehash(Table->Name));
    if (Table->hasDeleteAll)
      if (Table->MarkedVBNet7 == 0 || Table->extDeleteAll & doVBNET7)
        VBNetNoIOProc(Table,     "DeleteAll");
    if (Table->hasSelectOne)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectOne & doVBNET7)
        VBNetSingleProc(Table,   "SelectOne", Dehash(Table->Name));
    if (Table->hasSelectOneUpd)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectOneUpd & doVBNET7)
        VBNetSingleProc(Table,   "SelectOneUpd", Dehash(Table->Name));
    if (Table->hasSelectAll)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectAll & doVBNET7)
        VBNetMultiProc(Table,    "SelectAll", Dehash(Table->Name));
    if (Table->hasSelectAllUpd)
      if (Table->MarkedVBNet7 == 0 || Table->extSelectAllUpd & doVBNET7)
        VBNetMultiProc(Table,    "SelectAllUpd", Dehash(Table->Name));
    for (i=0; i<Table->noProcs; i++)
    {
      PYYProc Proc = &Table->Procs[i];
      if (Proc->isData || Proc->isStd)
        continue;
      if (Table->TargetVBNet7 == 0 && (Proc->extProc & doVBNET7) == 0)
        continue;
      if (Proc->noFields == 0)
        VBNetNoIOProc(Table, Proc->Name);
      else if (Proc->useStd)
      {
        if (Proc->isSql && Proc->isSingle)
          VBNetSingleProc(Table, Proc->Name, Dehash(Table->Name));
        else if (Proc->isSql && Proc->noOutputs > 0)
          VBNetMultiProc(Table, Proc->Name, Dehash(Table->Name));
        else
          VBNetNoOutputProc(Table, Proc->Name, Dehash(Table->Name));
      }
    }
  }
  fprintf(VBNet7File,
      "End Class\n\n"
      );
}

static void GenerateVBKey(PYYTable Table, PYYKey Key)
{
  ushort i;
  if (Key->noFields == 0)
    return;
  fprintf(VBNet7File,
      "Public Structure T%s%sKEY\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBNet7File,
        "  %s\n", VBFieldType(Field)
        );
  }
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  fprintf(VBNet7File,
      "Public Structure V%s%sKEY\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBNet7File,
        "  %s\n",
        VBFieldType(Field, false)
        );
  }
  fprintf(VBNet7File,
      "\n  Public Sub FromV(ByRef TRec As T%s%sKEY)\n"
      , US, Dehash(Table->Name)
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    fprintf(VBNet7File,
        "    TRec.%s = %s\n"
        , Field->Name, VBSpaceType(Field)
        );
  }
  fprintf(VBNet7File,
      "  End Sub\n\n"
      );
  fprintf(VBNet7File,
      "  Public Const _Make_Up = \""
      );
  for (i=0; i < Key->noFields; i++)
  {
    PYYField Field = &Key->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "I2"
          );
    fprintf(VBNet7File,
        "%s"
        , VBMakerType(Field)
        );
  }
  fprintf(VBNet7File,
       "\"\n\n"
       );
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  Table->noBuffers++;
}

static void GenerateVBKeyProcs(PYYTable Table, PYYKey Key)
{
  char Work[256];
  if (Key->noFields == 0)
    return;
  sprintf(Work, "%s%sKEY", Dehash(Table->Name), US);
  GenerateVBClassTop(Work);
  GenerateVBInit(Work);
  VBNetNoOutputProc(Table, "DeleteOne", Work, false);
  fprintf(VBNet7File,
      "End Class\n\n"
      );
}

static void GenerateVBProc(PYYTable Table, PYYProc Proc)
{
  ushort i;
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetVBNet7 == 0 && (Proc->extProc & doVBNET7) == 0)
    return;
  fprintf(VBNet7File,
      "Public Structure T%s%s%s%s\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    fprintf(VBNet7File,
        "  %s\n", VBFieldType(Field)
        );
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "  %sIsNull As Short\n"
          , Field->Name
          );
  }
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  fprintf(VBNet7File,
      "Public Structure V%s%s%s%s\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "  %sIsNull As Boolean\n"
          , Field->Name
          );
    fprintf(VBNet7File,
        "  %s\n"
        , VBFieldType(Field, false)
        );
  }
  fprintf(VBNet7File,
      "\n  Public Sub ToV(ByRef TRec As T%s%s%s%s)\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "    If TRec.%sIsNull = 0 Then %sIsNull = True Else %sIsNull = False\n"
          , Field->Name, Field->Name, Field->Name
          );
    if (isString(Field))
      fprintf(VBNet7File,
          "    %s = RTrim$(TRec.%s)\n"
          , Field->Name, Field->Name
          );
    else
      fprintf(VBNet7File,
          "    %s = TRec.%s\n"
          , Field->Name, Field->Name
          );
  }
  fprintf(VBNet7File,
      "  End Sub\n\n"
      );
  fprintf(VBNet7File,
      "  Public Sub FromV(ByRef TRec As T%s%s%s%s)\n"
      , US, Dehash(Table->Name), US, Proc->Name
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "    If %sIsNull Then TRec.%sIsNull = 1 Else TRec.%sIsNull = 0\n"
          , Field->Name, Field->Name, Field->Name
          );
    fprintf(VBNet7File,
        "    TRec.%s = %s\n"
        , Field->Name, VBSpaceType(Field)
        );
  }
  fprintf(VBNet7File,
      "  End Sub\n\n"
      );
  fprintf(VBNet7File,
      "  Public Const _Make_Up = \""
      );
  for (i=0; i < Proc->noFields; i++)
  {
    PYYField Field = &Proc->Fields[i];
    if (Table->isNullEnabled && Field->isNull && NullableField(Field))
      fprintf(VBNet7File,
          "I2"
          );
    fprintf(VBNet7File,
        "%s"
        , VBMakerType(Field)
        );
  }
  fprintf(VBNet7File,
       "\"\n\n"
       );
  fprintf(VBNet7File,
      "End Structure\n\n"
      );
  Table->noBuffers++;
}

static void GenerateVBProcProcs(PYYTable Table, PYYProc Proc)
{
  int i;
  char Work[256];
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetVBNet7 == 0 && (Proc->extProc & doVBNET7) == 0)
    return;
  sprintf(Work, "%s%s%s", Dehash(Table->Name), US, Proc->Name);
  GenerateVBClassTop(Work);
  GenerateVBInit(Work);
  if (Proc->isSql && Proc->isSingle)
  {
    fprintf(VBNet7File, "\n");
    VBNetSingleProc(Table, Proc->Name, Work);
  }
  else if (Proc->isSql && Proc->noOutputs > 0)
  {
    VBCursorVars(Proc->Name);
    VBNetMultiProc(Table, Proc->Name, Work);
  }
  else
  {
    fprintf(VBNet7File, "\n");
    VBNetNoOutputProc(Table, Proc->Name, Work);
  }
  fprintf(VBNet7File,
      "End Class\n\n"
      );
}

void GenerateVBNet7(PYYTable Table)
{
  MakeVBName(Table);
  ushort i;
  int rc = OpenVBFile(Table->VBNet7FileName);
  if (rc)
    return;
  if (Table->noFields > 0)
    GenerateVBTable(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateVBKey(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateVBProc(Table, &Table->Procs[i]);
  if (Table->noFields > 0)
    GenerateVBTableProcs(Table);
  if (Table->hasPrimaryKey && Table->hasDeleteOne)
    GenerateVBKeyProcs(Table, &Table->Keys[Table->PrimaryKey]);
  for (i=0; i<Table->noProcs; i++)
    GenerateVBProcProcs(Table, &Table->Procs[i]);
  fclose(VBNet7File);
}


