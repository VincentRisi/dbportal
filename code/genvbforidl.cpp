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

static FILE *VBforIDLFile  = 0;

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
  VBforIDLFile = OpenFile("VB File", VBFileName);
  if (VBforIDLFile == 0)
    return 1;
  return 0;
}

static void MakeVBName(PYYTable Table, const char *Added, bool asClass=false)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  strcat(fname, Added);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->VBforIDLDir))
    FNameMerge(newpath, Table->VBforIDLDir, fname, asClass ? Table->VB5Ext : Table->VBExt);
  else
    FNameMerge(newpath, dir, fname, asClass ? Table->VB5Ext : Table->VBExt);
  Table->VBforIDLFileName = strdup(newpath);
}

static void GenerateVBTop(const char *FName, const char *Added)
{
  fprintf(VBforIDLFile, "ATTRIBUTE VB_NAME=\"M%s%s\"\n"
                        "' Please do not modify this, modify the source then regenerate\n"
                        "Option Explicit\n\n", FName, Added);
  fprintf(VBforIDLFile, "' This code has conditional compilation flags. These are as follows:-\n"
                        "'   RDS_CODE    - compile the IDL calls to process thru an RDS server\n"
                        "'   SQLAPI_CODE - compile the code to go directly to Database (2 tier)\n"
                        "'                 in this case you are responsible for commits and rollbacks\n\n");
}

static void GenerateVBClassTop(const char *FName, const char *Added)
{
  fprintf(VBforIDLFile, "' %s%s\n", FName, Added);
  fprintf(VBforIDLFile, "' This code must be copied together with similar code to form the\n"
                        "' RDS Server class\n\n");
}

static void GenerateVBforIDLCompressCode(const char *structName, const char *procName, const char *pad="")
{
  fprintf(VBforIDLFile, "%sPrivate Sub Compress%s(Rec As t%s, Buffer As String)\n", pad, procName, structName);
  fprintf(VBforIDLFile, "%s  Dim OutLen As Long\n", pad);
  fprintf(VBforIDLFile, "%s  OutLen = Len(Rec)\n", pad);
  fprintf(VBforIDLFile, "%s  Buffer = String$(OutLen, \" \")\n", pad);
  fprintf(VBforIDLFile, "%s  ZCompress Rec, Len(Rec), Buffer, OutLen\n", pad);
  fprintf(VBforIDLFile, "%s  Buffer = Left$(Buffer, OutLen)\n", pad);
  fprintf(VBforIDLFile, "%sEnd Sub\n\n", pad);
  fprintf(VBforIDLFile, "%sPrivate Sub Decompress%s(Buffer As String, Rec As t%s)\n", pad, procName, structName);
  fprintf(VBforIDLFile, "%s  ZDecompress Buffer, Len(Buffer), Rec, Len(Rec)\n", pad);
  fprintf(VBforIDLFile, "%sEnd Sub\n", pad);
}

static void GenerateVBforIDLServerAction(const char *structName, const char *procName, const char *moduleName, bool isFunc=false)
{
  GenerateVBforIDLCompressCode(structName, procName);
  fprintf(VBforIDLFile, "\n");
  if (isFunc)
    fprintf(VBforIDLFile, "Public Function Server%s(Buffer As String) As Long\n", procName);
  else
    fprintf(VBforIDLFile, "Public Sub Server%s(Buffer As String)\n", procName);
  fprintf(VBforIDLFile, "  Dim Rec As t%s\n", structName);
  fprintf(VBforIDLFile, "  Decompress%s Buffer, Rec\n", procName);
  fprintf(VBforIDLFile, "  PerformIDLLogon\n");
  if (isFunc)
    fprintf(VBforIDLFile, "  Server%s = %s%s(Rec)\n", procName, moduleName, procName);
  else
    fprintf(VBforIDLFile, "  %s%s Rec\n", moduleName, procName);
  fprintf(VBforIDLFile, "  PerformIDLLogoff\n");
  fprintf(VBforIDLFile, "  Compress%s Rec, Buffer\n", procName);
  if (isFunc)
    fprintf(VBforIDLFile, "End Function\n\n");
  else
    fprintf(VBforIDLFile, "End Sub\n\n");
}

static void GenerateVBforIDLServerReadMany(const char *structName, const char *procName, const char *moduleName, int NoInput=0)
{
  GenerateVBforIDLCompressCode(structName, procName);
  fprintf(VBforIDLFile, "\n");
  if (NoInput == 0)
    fprintf(VBforIDLFile, "Public Function Server%s(NoOf As Long, RetBuffer() As String, MaxNo As Long) As Long\n", procName);
  else
  {
    fprintf(VBforIDLFile, "Public Function Server%s(Buffer As String, NoOf As Long, RetBuffer() As String, MaxNo As Long) As Long\n", procName);
    fprintf(VBforIDLFile, "  Dim Rec As t%s\n", structName);
  }
  fprintf(VBforIDLFile, "  Dim Recs() As t%s\n", structName);
  fprintf(VBforIDLFile, "  Dim I As Long\n");
  fprintf(VBforIDLFile, "  PerformIDLLogon\n");
  if (NoInput != 0)
  {
    fprintf(VBforIDLFile, "  Decompress%s Buffer, Rec\n", procName);
    fprintf(VBforIDLFile, "  Server%s = %s%s(Rec, NoOf, Recs, MaxNo)\n", procName, moduleName, procName);
  }
  else
    fprintf(VBforIDLFile, "  Server%s = %s%s(NoOf, Recs, MaxNo)\n", procName, moduleName, procName);
  fprintf(VBforIDLFile, "  PerformIDLLogoff\n");
  fprintf(VBforIDLFile, "  ReDim RetBuffer(0 To NoOf)\n");
  fprintf(VBforIDLFile, "  For I=1 To NoOf\n");
  fprintf(VBforIDLFile, "    Compress%s Recs(I), RetBuffer(I)\n", procName);
  fprintf(VBforIDLFile, "  Next I\n");
  fprintf(VBforIDLFile, "End Function\n\n");
}

static void GenerateVBforIDLServerCode(PYYTable Table)
{
  char structName[256];
  char procName[256];
  if (Table->Internal == 0)
  {
    if (Table->hasInsert && (Table->extInsert & noIDL) == 0)
    {
      if (Table->MarkedIDL == 0 || (Table->extInsert & doIDL) == doIDL)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sInsert", US, Dehash(Table->Name), US);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule);
      }
    }
    if (Table->hasPrimaryKey)
    {
      if (Table->hasSelectOne && (Table->extSelectOne & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extSelectOne & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s", US, Dehash(Table->Name));
          sprintf(procName, "%s%s%sSelectOne", US, Dehash(Table->Name), US);
          GenerateVBforIDLServerAction(structName, procName, Table->IDLModule, true);
        }
      }
      if (Table->hasSelectAll && (Table->extSelectAll & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extSelectAll & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s", US, Dehash(Table->Name));
          sprintf(procName, "%s%s%sSelectAll", US, Dehash(Table->Name), US);
          GenerateVBforIDLServerReadMany(structName, procName, Table->IDLModule);
        }
      }
      if (Table->hasExists && (Table->extExists & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extExists & doIDL) == doIDL)
        {
          sprintf(procName, "%s%s%sExists", US, Dehash(Table->Name), US);
          GenerateVBforIDLServerAction(procName, procName, Table->IDLModule, true);
        }
      }
      if (Table->hasCount && (Table->extCount & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extCount & doIDL) == doIDL)
        {
          sprintf(procName, "%s%s%sCount", US, Dehash(Table->Name), US);
          GenerateVBforIDLServerAction(procName, procName, Table->IDLModule, true);
        }
      }
      if (Table->hasDeleteOne && (Table->extDeleteOne & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extDeleteOne & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s%sKey", US, Dehash(Table->Name), US);
          sprintf(procName, "%s%s%sDeleteOne", US, Dehash(Table->Name), US);
          GenerateVBforIDLServerAction(structName, procName, Table->IDLModule);
        }
      }
    }
    if (Table->hasUpdate && (Table->extUpdate & noIDL) == 0)
    {
      if (Table->MarkedIDL == 0 || (Table->extUpdate & doIDL) == doIDL)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sUpdate", US, Dehash(Table->Name), US);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule);
      }
    }
  }
  int i;
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData || (Proc->extProc & noIDL) == noIDL)
      continue;
    // Must be Marked if Target == 0
    if (Table->TargetIDL == 0 && (Proc->extProc & doIDL) == 0)
      continue;
    if (Proc->useStd)
    {
      if (Proc->isSingle != 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule, true);
      }
      else if (Proc->noFields == 0)
      {}
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerReadMany(structName, procName, Table->IDLModule);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule, true);
      }
    }
    else
    {
      if (Proc->isSingle != 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(procName, procName, Table->IDLModule, true);
      }
      else if (Proc->noFields == 0)
      {}
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        if (strcmp(Proc->Name, "DeleteOne")==0)
          sprintf(structName, "%s%s%sKey", US, Dehash(Table->Name), US);
        else
          sprintf(structName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(structName, procName, Table->IDLModule);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerReadMany(procName, procName, Table->IDLModule, (int)Proc->noBinds);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLServerAction(procName, procName, Table->IDLModule, true);
      }
    }
  }
}

static void GenerateVBforIDLClientTop(const char *structName, const char *procName)
{
  fprintf(VBforIDLFile, "#If RDS_CODE Then\n");
  GenerateVBforIDLCompressCode(structName, procName, "  ");
  fprintf(VBforIDLFile, "#End If\n\n");
}

static void GenerateVBforIDLClientActionRDS(const char *procName, const char *tableName, const char *moduleName, bool isFunc=false)
{
  fprintf(VBforIDLFile,   "#If SQLAPI_CODE Then\n");
  fprintf(VBforIDLFile,   "  Dim Erc As Integer\n");
  fprintf(VBforIDLFile,   "  Erc = DBRunProc(Connect.Handle, \"%s\", Rec)\n", procName);
  if (isFunc)
    fprintf(VBforIDLFile, "  Common%s = Connect.CheckSingle(Erc, \"%s\")\n", procName, procName);
  else
    fprintf(VBforIDLFile, "  If Erc <> SQLapiOK Then\n"
                          "    Connect.Rollback\n"
                          "    Err.Raise Erc, \"%s\", Connect.ErrorMsg(Erc)\n"
                          "  End If\n"
                          "  Connect.Commit\n", procName);
  fprintf(VBforIDLFile,   "#ElseIf RDS_CODE Then\n");
  fprintf(VBforIDLFile,   "  Dim ADS As New RDS.DataSpace\n");
  fprintf(VBforIDLFile,   "  Dim Conn As Object\n");
  fprintf(VBforIDLFile,   "  Dim Buffer As String\n");
  fprintf(VBforIDLFile,   "  Set Conn = ADS.CreateObject(RDSConnector, Host)\n");
  fprintf(VBforIDLFile,   "  Compress%s Rec, Buffer\n", procName);
  if (isFunc)
    fprintf(VBforIDLFile, "  Common%s = Conn.Server%s(Buffer)\n", procName, procName);
  else
    fprintf(VBforIDLFile, "  Conn.Server%s Buffer\n", procName);
  fprintf(VBforIDLFile,   "  Decompress%s Buffer, Rec\n", procName);
  fprintf(VBforIDLFile,   "#Else\n");
  if (isFunc)
    fprintf(VBforIDLFile, "  Common%s = %s%s(Rec)\n", procName, moduleName, procName);
  else
    fprintf(VBforIDLFile, "  %s%s Rec\n", moduleName, procName);
  fprintf(VBforIDLFile,   "#End If\n");
}

static void GenerateVBforIDLClientAction(const char *structName, const char *procName, const char *tableName, const char *moduleName)
{
  GenerateVBforIDLClientTop(structName, procName);
  fprintf(VBforIDLFile, "Public Sub Common%s(Rec As t%s)\n", procName, structName);
  GenerateVBforIDLClientActionRDS(procName, tableName, moduleName);
  fprintf(VBforIDLFile, "End Sub\n\n");
}

static void GenerateVBforIDLClientReadOne(const char *structName, const char *procName, const char *tableName, const char *moduleName)
{
  GenerateVBforIDLClientTop(structName, procName);
  fprintf(VBforIDLFile, "Public Function Common%s(Rec As t%s)\n",procName , structName);
  GenerateVBforIDLClientActionRDS(procName, tableName, moduleName, true);
  fprintf(VBforIDLFile, "End Function\n\n");
}

static void GenerateVBforIDLClientReadMany(const char *structName, const char *procName, const char *tableName, const char *moduleName, int NoInput=0)
{
  GenerateVBforIDLClientTop(structName, procName);
  if (NoInput == 0)
    fprintf(VBforIDLFile, "Public Function Common%s(NoOf As Long, Recs() As t%s, MaxNo As Long) As Long\n", procName, structName);
  else
    fprintf(VBforIDLFile, "Public Function Common%s(Rec As t%s, NoOf As Long, Recs() As t%s, MaxNo As Long) As Long\n", procName, structName, structName);
  fprintf(VBforIDLFile,   "#If SQLAPI_CODE Then\n");
  fprintf(VBforIDLFile,   "  Dim Erc As Integer\n");
  fprintf(VBforIDLFile,   "  Dim Cursor As New TCursor\n");
  fprintf(VBforIDLFile,   "  Dim Handle As Long\n");
  if (NoInput == 0)
    fprintf(VBforIDLFile, "  Dim Rec As t%s\n", structName);
  fprintf(VBforIDLFile,   "  NoOf = 0\n");
  fprintf(VBforIDLFile,   "  Erc = DBOpenExec(Connect.Handle, Handle, \"%s\", Rec)\n", procName);
  fprintf(VBforIDLFile,   "  Cursor.Handle = Handle\n");
  fprintf(VBforIDLFile,   "  If Erc <> SQLapiOK then Err.Raise Erc, \"%s\", Cursor.ErrorMsg(Erc)\n", procName);
  fprintf(VBforIDLFile,   "  ReDim Preserve Recs(0 to 64)\n");
  fprintf(VBforIDLFile,   "  Do\n");
  fprintf(VBforIDLFile,   "    Erc = DBFetchData(Cursor.Handle, Rec)\n");
  fprintf(VBforIDLFile,   "    If Not Cursor.CheckEOF(Erc, \"%s\") Then Exit Do\n", procName);
  fprintf(VBforIDLFile,   "    NoOf = NoOf + 1\n");
  fprintf(VBforIDLFile,   "    if NoOf > Ubound(Recs) Then ReDim Preserve Recs(0 to NoOf * 2)\n");
  fprintf(VBforIDLFile,   "    Recs(NoOf) = Rec\n");
  fprintf(VBforIDLFile,   "  Loop\n");
  fprintf(VBforIDLFile,   "  ReDim Preserve Recs(0 to NoOf)\n");
  fprintf(VBforIDLFile,   "#ElseIf RDS_CODE Then\n");
  if (NoInput != 0)
    fprintf(VBforIDLFile, "  Dim Buffer As String\n");
  fprintf(VBforIDLFile,   "  Dim Buffers() As String\n");
  fprintf(VBforIDLFile,   "  Dim I As Long\n");
  fprintf(VBforIDLFile,   "  Dim ADS As New RDS.DataSpace\n");
  fprintf(VBforIDLFile,   "  Dim Conn As Object\n");
  fprintf(VBforIDLFile,   "  Set Conn = ADS.CreateObject(RDSConnector, Host)\n");
  if (NoInput != 0)
  {
    fprintf(VBforIDLFile, "  Compress%s Rec, Buffer\n", procName);
    fprintf(VBforIDLFile, "  Common%s = Conn.Server%s(Buffer, NoOf, Buffers, MaxNo)\n", procName, procName);
  }
  else
    fprintf(VBforIDLFile, "  Common%s = Conn.Server%s(NoOf, Buffers, MaxNo)\n", procName, procName);
  fprintf(VBforIDLFile,   "  ReDim Recs(0 To NoOf)\n");
  fprintf(VBforIDLFile,   "  For I=1 To NoOf\n");
  fprintf(VBforIDLFile,   "    Decompress%s Buffers(I), Recs(I)\n", procName);
  fprintf(VBforIDLFile,   "  Next I\n");
  fprintf(VBforIDLFile,   "#Else\n");
  if (NoInput == 0)
    fprintf(VBforIDLFile, "  Common%s = %s%s(NoOf, Recs, MaxNo)\n", procName, moduleName, procName);
  else
    fprintf(VBforIDLFile, "  Common%s = %s%s(Rec, NoOf, Recs, MaxNo)\n", procName, moduleName, procName);
  fprintf(VBforIDLFile,   "#End If\n");
  fprintf(VBforIDLFile,   "End Function\n\n");
}

static void GenerateVBforIDLClientCode(PYYTable Table)
{
  char structName[256];
  char procName[256];
  if (Table->Internal == 0)
  {
    if (Table->hasInsert && (Table->extInsert & noIDL) == 0)
    {
      if (Table->MarkedIDL == 0 || (Table->extInsert & doIDL) == doIDL)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sInsert", US, Dehash(Table->Name), US);
        GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
      }
    }
    if (Table->hasPrimaryKey)
    {
      if (Table->hasSelectOne && (Table->extSelectOne & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extSelectOne & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s", US, Dehash(Table->Name));
          sprintf(procName, "%s%s%sSelectOne", US, Dehash(Table->Name), US);
          GenerateVBforIDLClientReadOne(structName, procName, Table->Name, Table->IDLModule);
        }
      }
      if (Table->hasSelectAll && (Table->extSelectAll & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extSelectAll & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s", US, Dehash(Table->Name));
          sprintf(procName, "%s%s%sSelectAll", US, Dehash(Table->Name), US);
          GenerateVBforIDLClientReadMany(structName, procName, Table->Name, Table->IDLModule);
        }
      }
      if (Table->hasExists && (Table->extExists & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extExists & doIDL) == doIDL)
        {
          sprintf(procName, "%s%s%sExists", US, Dehash(Table->Name), US);
          GenerateVBforIDLClientAction(procName, procName, Table->Name, Table->IDLModule);
        }
      }
      if (Table->hasCount && (Table->extCount & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extCount & doIDL) == doIDL)
        {
          sprintf(procName, "%s%s%sCount", US, Dehash(Table->Name), US);
          GenerateVBforIDLClientAction(procName, procName, Table->Name, Table->IDLModule);
        }
      }
      if (Table->hasDeleteOne && (Table->extDeleteOne & noIDL) == 0)
      {
        if (Table->MarkedIDL == 0 || (Table->extDeleteOne & doIDL) == doIDL)
        {
          sprintf(structName, "%s%s%sKey", US, Dehash(Table->Name), US);
          sprintf(procName, "%s%s%sDeleteOne", US, Dehash(Table->Name), US);
          GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
        }
      }
    }
    if (Table->hasUpdate && (Table->extUpdate & noIDL) == 0)
    {
      if (Table->MarkedIDL == 0 || (Table->extUpdate & doIDL) == doIDL)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sUpdate", US, Dehash(Table->Name), US);
        GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
      }
    }
  }
  int i;
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData || (Proc->extProc & noIDL) == noIDL)
      continue;
    // Must be Marked if Target == 0
    if (Table->TargetIDL == 0 && (Proc->extProc & doIDL) == 0)
      continue;
    if (Proc->useStd)
    {
      if (Proc->isSingle != 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientReadOne(structName, procName, Table->Name, Table->IDLModule);
      }
      else if (Proc->noFields == 0)
      {}
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientReadMany(structName, procName, Table->Name, Table->IDLModule);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
      }
    }
    else
    {
      if (Proc->isSingle != 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientReadOne(procName, procName, Table->Name, Table->IDLModule);
      }
      else if (Proc->noFields == 0)
      {}
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        if (strcmp(Proc->Name, "DeleteOne")==0)
          sprintf(structName, "%s%s%sKey", US, Dehash(Table->Name), US);
        else
          sprintf(structName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientAction(structName, procName, Table->Name, Table->IDLModule);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientReadMany(procName, procName, Table->Name, Table->IDLModule, (int)Proc->noBinds);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateVBforIDLClientAction(procName, procName, Table->Name, Table->IDLModule);
      }
    }
  }
}

void GenerateVBforIDL(PYYTable Table)
{
  MakeVBName(Table, "Server", true);
  int rc = OpenVBFile(Table->VBforIDLFileName);
  if (rc)
    return;
  GenerateVBClassTop(Table->Name, "Server");
  GenerateVBforIDLServerCode(Table);
  fclose(VBforIDLFile);
  free(Table->VBforIDLFileName);
  MakeVBName(Table, "Common");
  rc = OpenVBFile(Table->VBforIDLFileName);
  if (rc)
    return;
  GenerateVBTop(Table->Name, "Common");
  GenerateVBforIDLClientCode(Table);
  fclose(VBforIDLFile);
}

