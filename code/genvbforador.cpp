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

#define WORK_SIZE 1024
#define US Table->UnderScore ? "_" : ""

static FILE *VBforADORFile = 0;

static char *Dehash(const char *Name, char * Work)
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

static char *NameOf(PYYField Field, char *work)
{
  return Field->Alias ? Field->Alias : Dehash(Field->Name, work);
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
  VBforADORFile = OpenFile("VB File", VBFileName);
  if (VBforADORFile == 0)
    return 1;
  return 0;
}

static void MakeVBName(PYYTable Table, const char *Added)
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
  if (strlen(Table->VBDir))
    FNameMerge(newpath, Table->VBDir, fname, Table->VBExt);
  else
    FNameMerge(newpath, dir, fname, Table->VBExt);
  Table->VBFileName = strdup(newpath);
}

static void GenerateVBTop(const char *FName, const char *Added)
{
  fprintf(VBforADORFile, "ATTRIBUTE VB_NAME=\"M%s%s\"\n"
                        "' Please do not modify this, modify the source then regenerate\n"
                        "Option Explicit\n\n", FName, Added);
}

static const char *TypeOf(PYYField Field, char *work)
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
  case ftypeImage:
    sprintf(work, "adChar, %hu", Field->Length);
    return work;
  case ftypeDate:
    sprintf(work, "adChar, 9");
    return work;
  case ftypeDateTime:
    sprintf(work, "adChar, 15");
    return work;
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeInt:
    sprintf(work, "adInteger");
    return work;
  case ftypeFloat:
    sprintf(work, "adDouble");
    return work;
  }
  return "adJunk";
}

static void GenerateMake(const char *Name, int noFields, PYYField Fields)
{
  char nwork[WORK_SIZE], twork[WORK_SIZE];
  fprintf(VBforADORFile, "#If Recordset Then\n\n");
  fprintf(VBforADORFile, "Sub Make%sRS(RS As Recordset)\n", Name);
  fprintf(VBforADORFile, "  Set RS = New Recordset\n");
  for (int i=0; i < noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(VBforADORFile, "  RS.Fields.Append \"%s\", %s\n", NameOf(Field, nwork), TypeOf(Field, twork));
  }
  fprintf(VBforADORFile, "  OpenRecords RS\n"
                         "End Sub\n\n");
}

static void GenerateInput(const char *Name, int noFields, PYYField Fields)
{
  char nwork1[WORK_SIZE], nwork2[WORK_SIZE];
  fprintf(VBforADORFile, "Sub Input%sRS(RS As Recordset, Bytes() As Byte)\n", Name);
  fprintf(VBforADORFile, "  Dim I As Long\n"
                         "  Dim Rec As t%s\n", Name);
  fprintf(VBforADORFile, "  ReDim Bytes(0 To 0)\n"
                         "  If RS.BOF And RS.EOF Then Exit Sub\n"
                         "  I = 0\n"
                         "  RS.MoveFirst\n"
                         "  While Not RS.EOF\n");
  for (int i=0; i < noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(VBforADORFile, "    Rec.%s = RS.Fields(\"%s\")\n", NameOf(Field, nwork1), NameOf(Field, nwork2));
  }
  fprintf(VBforADORFile, "    ReDim Preserve Bytes(0 To I + Len(Rec))\n"
                         "    MemCopy Bytes(I + 1), Rec, Len(Rec)\n"
                         "    I = I + Len(Rec)\n"
                         "    RS.MoveNext\n"
                         "  Wend\n"
                         "End Sub\n\n");
}

static void GenerateOutput(const char *Name, int noFields, PYYField Fields)
{
  char nwork1[WORK_SIZE], nwork2[WORK_SIZE];
  fprintf(VBforADORFile, "Sub Output%sRS(Bytes() As Byte, RS As Recordset)\n", Name);
  fprintf(VBforADORFile, "  Dim I As Long\n"
                         "  Dim Rec As t%s\n", Name);
  fprintf(VBforADORFile, "  Make%sRS RS\n", Name);
  fprintf(VBforADORFile, "  For I = 1 To UBound(Bytes) Step Len(Rec)\n"
                         "    MemCopy Rec, Bytes(I), Len(Rec)\n"
                         "    RS.AddNew\n");
  for (int i=0; i < noFields; i++)
  {
    PYYField Field = &Fields[i];
    fprintf(VBforADORFile, "    RS.Fields(\"%s\") = Rec.%s\n", NameOf(Field, nwork1), NameOf(Field, nwork2));
  }
  fprintf(VBforADORFile, "  Next\n");
  fprintf(VBforADORFile, "End Sub\n\n");
  fprintf(VBforADORFile, "#EndIf\n\n");
}

static void GenerateRecToByte(const char *Name)
{
  fprintf(VBforADORFile, "#If Structures Then\n\n");
  fprintf(VBforADORFile, "Sub Copy%sRecToBytes(Recs() As t%s, Bytes() As Byte)\n", Name, Name);
  fprintf(VBforADORFile, "  Dim I As Long\n"
                         "  Dim J As Long\n"
                         "  Dim SizeOf As Long\n"
                         "  SizeOf = Len(Recs(0))\n"
                         "  ReDim Bytes(0 To SizeOf * UBound(Recs))\n"
                         "  J = 1\n"
                         "  For I = 1 To UBound(Recs)\n"
                         "    MemCopy Bytes(J), Recs(I), SizeOf\n"
                         "    J = J + SizeOf\n"
                         "  Next\n"
                         "End Sub\n\n");
}

static void GenerateByteToRec(const char *Name)
{
  fprintf(VBforADORFile, "Sub CopyBytesTo%sRec(Bytes() As Byte, Recs() As t%s)\n", Name, Name);
  fprintf(VBforADORFile, "  Dim I As Long\n"
                         "  Dim SizeOf As Long\n"
                         "  SizeOf = Len(Recs(0))\n"
                         "  ReDim Recs(0 To UBound(Bytes) / SizeOf)\n"
                         "  For I = 1 To UBound(Bytes) Step SizeOf\n"
                         "    MemCopy Recs(I \\ SizeOf + 1), Bytes(I), SizeOf\n"
                         "  Next\n"
                         "End Sub\n\n");
  fprintf(VBforADORFile, "#EndIf\n\n");
}

static void GenerateVBKey(PYYTable Table, PYYKey Key)
{
  if (Key->noFields == 0)
    return;
  char Name[WORK_SIZE], nwork[WORK_SIZE];
  sprintf(Name, "%sKey", Dehash(Table->Name, nwork));
  GenerateMake(Name, Key->noFields, Key->Fields);
  GenerateInput(Name, Key->noFields, Key->Fields);
  GenerateOutput(Name, Key->noFields, Key->Fields);
  GenerateRecToByte(Name);
  GenerateByteToRec(Name);
}

static void GenerateVBProc(PYYTable Table, PYYProc Proc)
{
  if (Proc->noFields == 0 || Proc->useStd || Proc->useKey)
    return;
  if (Table->TargetVBforADOR == 0 && (Proc->extProc & doVBFORADOR) == 0)
    return;
  char Name[WORK_SIZE], nwork[WORK_SIZE];
  sprintf(Name, "%s%s", Dehash(Table->Name, nwork), Proc->Name);
  GenerateMake(Name, Proc->noFields, Proc->Fields);
  GenerateInput(Name, Proc->noFields, Proc->Fields);
  GenerateOutput(Name, Proc->noFields, Proc->Fields);
  GenerateRecToByte(Name);
  GenerateByteToRec(Name);
}

static void StaticGenerateVB(PYYTable Table)
{
  if (Table->noFields)
  {
    GenerateMake(Table->Name, Table->noFields, Table->Fields);
    GenerateInput(Table->Name, Table->noFields, Table->Fields);
    GenerateOutput(Table->Name, Table->noFields, Table->Fields);
    GenerateRecToByte(Table->Name);
    GenerateByteToRec(Table->Name);
    if (Table->hasPrimaryKey && Table->hasDeleteOne)
      GenerateVBKey(Table, &Table->Keys[Table->PrimaryKey]);
  }
  for (int i=0; i<Table->noProcs; i++)
    GenerateVBProc(Table, &Table->Procs[i]);
}

void GenerateVBforADOR(PYYTable Table)
{
  MakeVBName(Table, "ADOR");
  int rc = OpenVBFile(Table->VBFileName);
  if (rc)
    return;
  GenerateVBTop(Table->Name, "ADOR");
  StaticGenerateVB(Table);
  fclose(VBforADORFile);
}
