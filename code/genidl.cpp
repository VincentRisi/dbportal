#include "machine.h"

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>

#include "sqldef.h"

#include "global.h"
#include "xdir.h"

#define US Table->UnderScore ? "_" : ""

static FILE *IDLFile = 0;

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

static void GenerateIDLStructs(PYYTable Table)
{
  int i;
  int stdDone = 0;
  char work[256];
  for (i=0; i<Table->noProcs; i++)
  {
    TYYProc* Proc = &Table->Procs[i];
    if (Proc->isData || (Proc->extProc & noIDL) == noIDL)
      continue;
    // Must be Marked if Target == 0
    if (Table->TargetIDL == 0 && (Proc->extProc & doIDL) == 0)
      continue;
    if (Proc->useStd != 0)
    {
      if (stdDone == 0)
      {
        sprintf(work, "%s%s", US, Dehash(Table->Name));
        fprintf(IDLFile, "struct t%-40s \"%s\"\n", work, strlwr(NameExt(Table->HeaderFileName)));
        stdDone = 1;
      }
    }
    else if (Proc->noFields)
    {
      sprintf(work, "%s%s%s%s", US, Dehash(Table->Name), US,
         (strcmp(Proc->Name, "DeleteOne")==0) ? "Key" : Proc->Name);
      fprintf(IDLFile, "struct t%-40s \"%s\"\n", work, strlwr(NameExt(Table->HeaderFileName)));
    }
  }
  fprintf(IDLFile, "\n");
}

static void GenerateIDLAction(const char *structName, const char *procName, const char *method, bool IDLUbi)
{
  fprintf(IDLFile, "public void _%s(t%s *Rec)\n", procName, structName);
  fprintf(IDLFile, "{\n");
  fprintf(IDLFile, "code\n");
  if (IDLUbi == true)
    fprintf(IDLFile, "  Rec->%s(*connect);\n", method);
  else
    fprintf(IDLFile, "  Rec->%s(Connect);\n", method);
  fprintf(IDLFile, "endcode\n");
  fprintf(IDLFile, "}\n\n");
  fprintf(IDLFile, "void %s(t%s *Rec)\n", procName, structName);
  fprintf(IDLFile, "{\n");
  fprintf(IDLFile, "message: #\n");
  fprintf(IDLFile, "input\n");
  fprintf(IDLFile, "  Rec;\n");
  fprintf(IDLFile, "output\n");
  fprintf(IDLFile, "  Rec;\n");
  fprintf(IDLFile, "code\n");
  fprintf(IDLFile, "  try\n");
  fprintf(IDLFile, "  {\n");
  fprintf(IDLFile, "    _%s(Rec);\n", procName);
  if (IDLUbi == true)
    fprintf(IDLFile, "    connect->Commit();\n");
  else
    fprintf(IDLFile, "    Connect.Commit();\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "  catch (xDBError &x)\n");
  fprintf(IDLFile, "  {\n");
  if (IDLUbi == true)
  {
    fprintf(IDLFile, "    logFile->Log(x);\n");
    fprintf(IDLFile, "    connect->Rollback();\n");
  }
  else
  {
    fprintf(IDLFile, "    LogFile.Log(x);\n");
    fprintf(IDLFile, "    Connect.Rollback();\n");
  }
  fprintf(IDLFile, "    THROW2(DB_ERROR, x.ErrorStr());\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "endcode\n");
  fprintf(IDLFile, "}\n\n");
}
static char* OLD32_LONG  = "int32";
static void GenerateIDLReadOne(const char *structName, const char *procName, const char *method, bool IDLUbi)
{
  fprintf(IDLFile, "%s %s(t%s *Rec)\n", OLD32_LONG, procName, structName);
  fprintf(IDLFile, "{\n");
  fprintf(IDLFile, "message: #\n");
  fprintf(IDLFile, "input\n");
  fprintf(IDLFile, "  Rec;\n");
  fprintf(IDLFile, "output\n");
  fprintf(IDLFile, "  Rec;\n");
  fprintf(IDLFile, "code\n");
  fprintf(IDLFile, "  try\n");
  fprintf(IDLFile, "  {\n");
  if (IDLUbi == true)
    fprintf(IDLFile, "    Rec->%s(*connect);\n", method);
  else
    fprintf(IDLFile, "    Rec->%s(Connect);\n", method);
  fprintf(IDLFile, "    return 0; // OK\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "  catch (xDBError &x)\n");
  fprintf(IDLFile, "  {\n");
  fprintf(IDLFile, "    if (x.NotFound())\n");
  fprintf(IDLFile, "      return 1; // NOT FOUND\n");
  if (IDLUbi == true)
    fprintf(IDLFile, "    logFile->Log(x);\n");
  else
    fprintf(IDLFile, "    LogFile.Log(x);\n");
  fprintf(IDLFile, "    THROW2(DB_ERROR, x.ErrorStr());\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "endcode\n");
  fprintf(IDLFile, "}\n\n");
}

static void GenerateIDLReadMany(const char *structName, const char *procName, const char *method, bool IDLUbi, int NoInput=0)
{
  if (NoInput == 0)
    fprintf(IDLFile, "%s %s(%s *NoOf, t%s *&Recs, %s Max)\n", OLD32_LONG, procName, OLD32_LONG, structName, OLD32_LONG);
  else
    fprintf(IDLFile, "%s %s(t%s *Rec, %s *NoOf, t%s *&Recs, %s Max)\n", OLD32_LONG, procName, structName, OLD32_LONG, structName, OLD32_LONG);
  fprintf(IDLFile, "{\n");
  fprintf(IDLFile, "message: #\n");
  fprintf(IDLFile, "input\n");
  fprintf(IDLFile, "  Max;\n");
  if (NoInput != 0)
    fprintf(IDLFile, "  Rec;\n");
  fprintf(IDLFile, "output\n");
  fprintf(IDLFile, "  NoOf;\n");
  fprintf(IDLFile, "  Recs size(NoOf);\n");
  fprintf(IDLFile, "code\n");
  fprintf(IDLFile, "  try\n");
  fprintf(IDLFile, "  {\n");
  if (NoInput == 0)
    fprintf(IDLFile, "  t%s Entry, *Rec=&Entry;\n", structName);
  fprintf(IDLFile, "    *NoOf = 0;\n");
  if (IDLUbi == true)
    fprintf(IDLFile, "    t%sQuery Query(*connect, Rec);\n", procName);
  else
    fprintf(IDLFile, "    t%sQuery Query(Connect, Rec);\n", procName);
  fprintf(IDLFile, "    while (Query.Fetch())\n");
  fprintf(IDLFile, "    {\n");
  fprintf(IDLFile, "      if (Max > 0 && *NoOf >= Max)\n");
  fprintf(IDLFile, "        return 1; // MAXIMUM EXCEEDED\n");
  fprintf(IDLFile, "      AddList(Recs, *NoOf, *Rec, (%s)(65536L/sizeof(*Rec)+1));\n", OLD32_LONG);
  fprintf(IDLFile, "    }\n");
  fprintf(IDLFile, "    return 0; // OK\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "  catch (xDBError &x)\n");
  fprintf(IDLFile, "  {\n");
  if (IDLUbi == true)
    fprintf(IDLFile, "    logFile->Log(x);\n");
  else
    fprintf(IDLFile, "    LogFile.Log(x);\n");
  fprintf(IDLFile, "    THROW2(DB_ERROR, x.ErrorStr());\n");
  fprintf(IDLFile, "  }\n");
  fprintf(IDLFile, "endcode\n");
  fprintf(IDLFile, "}\n\n");
}

static void GenerateIDLCode(PYYTable Table)
{
  char structName[256];
  char procName[256];
  if (Table->Internal == 0)
  {
    if (Table->hasInsert && (Table->extInsert & noIDL) == 0
    && (Table->MarkedIDL == 0 || (Table->extInsert & doIDL) == doIDL))
    {
      sprintf(structName, "%s%s", US, Dehash(Table->Name));
      sprintf(procName, "%s%s%sInsert", US, Dehash(Table->Name), US);
      GenerateIDLAction(structName, procName, "Insert", Table->IDLUbi != 0);
    }
    if (Table->hasPrimaryKey)
    {
      if (Table->hasSelectOne && (Table->extSelectOne & noIDL) == 0
      && (Table->MarkedIDL == 0 || (Table->extSelectOne & doIDL) == doIDL))
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sSelectOne", US, Dehash(Table->Name), US);
        GenerateIDLReadOne(structName, procName, "SelectOne", Table->IDLUbi != 0);
      }
      if (Table->hasSelectAll && (Table->extSelectAll & noIDL) == 0
      && (Table->MarkedIDL == 0 || (Table->extSelectAll & doIDL) == doIDL))
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%sSelectAll", US, Dehash(Table->Name), US);
        GenerateIDLReadMany(structName, procName, "SelectAll", Table->IDLUbi != 0);
      }
      if (Table->hasExists && (Table->extExists & noIDL) == 0
      && (Table->MarkedIDL == 0 || (Table->extExists & doIDL) == doIDL))
      {
        sprintf(procName, "%s%s%sExists", US, Dehash(Table->Name), US);
        GenerateIDLAction(procName, procName, "Exists", Table->IDLUbi != 0);
      }
      if (Table->hasCount && (Table->extCount & noIDL) == 0
      && (Table->MarkedIDL == 0 || (Table->extCount & doIDL) == doIDL))
      {
        sprintf(procName, "%s%s%sCount", US, Dehash(Table->Name), US);
        GenerateIDLAction(procName, procName, "Count", Table->IDLUbi != 0);
      }
      if (Table->hasDeleteOne && (Table->extDeleteOne & noIDL) == 0
      && (Table->MarkedIDL == 0 || (Table->extDeleteOne & doIDL) == doIDL))
      {
        sprintf(structName, "%s%s%sKey", US, Dehash(Table->Name), US);
        sprintf(procName, "%s%s%sDeleteOne", US, Dehash(Table->Name), US);
        GenerateIDLAction(structName, procName, "DeleteOne", Table->IDLUbi != 0);
      }
    }
    if (Table->hasUpdate && (Table->extUpdate & noIDL) == 0
    && (Table->MarkedIDL == 0 || (Table->extUpdate & doIDL) == doIDL))
    {
      sprintf(structName, "%s%s", US, Dehash(Table->Name));
      sprintf(procName, "%s%s%sUpdate", US, Dehash(Table->Name), US);
      GenerateIDLAction(structName, procName, "Update", Table->IDLUbi != 0);
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
        GenerateIDLReadOne(structName, procName, Proc->Name, Table->IDLUbi != 0);
      }
      else if (Proc->noFields == 0)
      {}
      else if (Proc->isSql != 0 && Proc->noOutputs == 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLAction(structName, procName, Proc->Name, Table->IDLUbi != 0);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLReadMany(structName, procName, Proc->Name, Table->IDLUbi != 0);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(structName, "%s%s", US, Dehash(Table->Name));
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLAction(structName, procName, Proc->Name, Table->IDLUbi != 0);
      }
    }
    else
    {
      if (Proc->isSingle != 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLReadOne(procName, procName, Proc->Name, Table->IDLUbi != 0);
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
        GenerateIDLAction(structName, procName, Proc->Name, Table->IDLUbi != 0);
      }
      else if (Proc->isSql != 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLReadMany(procName, procName, Proc->Name, Table->IDLUbi != 0, (int)Proc->noBinds);
      }
      else if (Proc->isSql == 0 && Proc->noOutputs > 0)
      {
        sprintf(procName, "%s%s%s%s", US, Dehash(Table->Name), US, Proc->Name);
        GenerateIDLAction(procName, procName, Proc->Name, Table->IDLUbi != 0);
      }
    }
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

static void MakeIDLName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (strlen(Table->IDLDir))
    FNameMerge(newpath, Table->IDLDir, fname, Table->IDLExt);
  else
    FNameMerge(newpath, dir, fname, Table->IDLExt);
  Table->IDLFileName = strdup(newpath);
  if (Table->HeaderFileName == 0)
  {
    if (strlen(Table->CDir))
      FNameMerge(newpath, Table->CDir, fname, Table->CExt);
    else
      FNameMerge(newpath, dir, fname, Table->CExt);
    Table->HeaderFileName = strdup(newpath);
  }
}

void GenerateIDL(PYYTable Table)
{
  MakeIDLName(Table);
  IDLFile = OpenFile("IDL File", Table->IDLFileName);
  if (IDLFile == 0)
    return;
  fprintf(IDLFile, "// generated code mutilation will follow manual changes done here \n\n");
  GenerateIDLStructs(Table);
  GenerateIDLCode(Table);
  fclose(IDLFile);
}

