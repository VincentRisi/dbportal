#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#include "xstring.h"

#if defined(M_OS2) || defined(M_DOS) || defined(M_NT)
 #include <io.h>
 #include <sys\stat.h>
#else
 #include <sys/stat.h>
#endif
#include <fcntl.h>

#include "global.h"
#include "getargs.h"
#include "xdir.h"
//#include "switches.h"
char *BinDir="", *BinExt=".so", *ControlDB=""
   , *HeaderDir="", *HeaderExt=".sh"
   , *IDLDir="", *IDLExt=".ii"
   , *LogDir="", *LogExt=".log"
   , *SqlAuditExt=".aud", *SqlConExt=".con", *SqlDir="", *SqlExt=".sql"
   , *SqlGrantExt=".gra", *SqlIdxExt=".idx", *SqlProcExt=".pro", *SqlSnapExt=".pop"
   , *SqlTableExt=".tab", *SqlViewsExt=".vws"
   , *UConnect="";
int BlankPadded = 0, CharZ = 0, ConnReqd = 0, ExitReqd = 0, ExtendedCode = 0, Internal = 0
  , OneSQLScript = 0, TargetC = 0, TargetIDL = 0, TargetSQL = 0, UnderScore = 0;

char* Others="";
int Help = 0;

static ARG argtab[] =
 {{'b', STRING, (int*)&BinDir,       "Binary Directory"}
 ,{'B', STRING, (int*)&BinExt,       "Binary Extension"}
 ,{'p', BOOLEAN,      &BlankPadded,  "Char fields blank padded"}
 ,{'q', BOOLEAN,      &CharZ,        "Null terminated char fields in procs"}
 ,{'w', BOOLEAN,      &ConnReqd,     "CONNECT start SQL"}
 ,{'c', STRING, (int*)&ControlDB,    "Control Database"}
 ,{'x', BOOLEAN,      &ExitReqd,     "EXIT ends SQL"}
 ,{'X', BOOLEAN,      &ExtendedCode, "Extended Runtime Code"}
 ,{'h', STRING, (int*)&HeaderDir,    "Header Directory"}
 ,{'H', STRING, (int*)&HeaderExt,    "Header Extension"}
 ,{'k', STRING, (int*)&IDLDir,       "IDLE Directory"}
 ,{'j', STRING, (int*)&IDLExt,       "IDLE Extension"}
 ,{'i', BOOLEAN,      &Internal,     "OCI Standard procs internal"}
 ,{'l', STRING, (int*)&LogDir,       "Log Directory"}
 ,{'L', STRING, (int*)&LogExt,       "Log Extension"}
 ,{'0', BOOLEAN,      &OneSQLScript, "One Sql file script"}
 ,{'o', STRING, (int*)&Others,       "The plethora of other switches as -o:x=1:y=str:..."}
 ,{'A', STRING, (int*)&SqlAuditExt,  "Sql Audit Extension"}
 ,{'1', STRING, (int*)&SqlConExt,    "Sql Constraints Extension"}
 ,{'s', STRING, (int*)&SqlDir,       "Sql Directory"}
 ,{'S', STRING, (int*)&SqlExt,       "Sql Extension"}
 ,{'2', STRING, (int*)&SqlGrantExt,  "Sql Grants Extension"}
 ,{'7', STRING, (int*)&SqlIdxExt,    "Sql Indexes Extension"}
 ,{'3', STRING, (int*)&SqlProcExt,   "Sql Procs Extension"}
 ,{'5', STRING, (int*)&SqlSnapExt,   "Sql Snapshots Extension"}
 ,{'4', STRING, (int*)&SqlTableExt,  "Sql Table Extension"}
 ,{'6', STRING, (int*)&SqlViewsExt,  "Sql Views Extension"}
 ,{'C', BOOLEAN,      &TargetC,      "Target C Header"}
 ,{'I', BOOLEAN,(int*)&TargetIDL,    "Target IDLE Header"}
 ,{'9', BOOLEAN,(int*)&TargetSQL,    "Target SQL File"}
 ,{'z', STRING, (int*)&UConnect,     "User store proc connect"}
 ,{'u', BOOLEAN,      &UnderScore,   "Underscore (a_) for procedures"}
 ,{'?', BOOLEAN,      &Help,         "This Help"}
};
#define TABSIZE (sizeof(argtab) / sizeof(ARG))

static FILE * yyerrfile;

#ifndef M_NT
 long filelength(int infile)
 {
    long result = lseek(infile ,0, SEEK_END);
    lseek(infile ,0, SEEK_SET);
    return result;
 }
#endif
static void OutputHandler(const char *Line)
{
  fprintf(yyerrfile, "%s\n", Line);
}

static char* StripSwitch(char* key, char* work, int worksize)
{
  char pk[512];strncpyz(pk, key, sizeof(pk)-1);
  char oth[512];strncpyz(oth, Others, sizeof(oth)-1);
  strlwr(pk);
  strlwr(oth);
  char* fp = strstr(oth, pk);
  if (fp != 0 && fp > oth && fp[-1] == ':' && fp[strlen(pk)] == '=')
  {
    fp += (strlen(pk)+1);
    char* p2 = strchr(fp, ':');
    if (p2 != 0 && p2-fp < worksize)
    {
      strncpy(work, fp, p2-fp);
      work[p2-fp] = 0;
      return work;
    }
    else if (p2 == 0 && strlen(fp) < worksize)
    {
      strcpy(work, fp);
      return work;
    }
  }
  return 0;
}

static void GetOther(int &i, char* key, int def=0)
{
  char work[256];
  char *p;
  p = StripSwitch(key, work, sizeof(work));
  if (p)
    i = atoi(p);
  else
    i = def;
}

static void GetOther(char *&s, char* key, char* def="")
{
  char work[256];
  char *p;
  p = StripSwitch(key, work, sizeof(work));
  if (p)
    s = strdup(p);
  else
    s = strdup(def);
}

void SetSwitches()
{
  Table->SODir            = strdup(BinDir);
  Table->SOExt            = strdup(BinExt);
  Table->ControlDB        = strdup(ControlDB);
  Table->CDir             = strdup(HeaderDir);
  Table->CExt             = strdup(HeaderExt);
  Table->IDLDir           = strdup(IDLDir);
  Table->IDLExt           = strdup(IDLExt);
  Table->LogDir           = strdup(LogDir);
  Table->LogExt           = strdup(LogExt);
  Table->SqlAuditExt      = strdup(SqlAuditExt);
  Table->SqlConExt        = strdup(SqlConExt);
  Table->SqlDir           = strdup(SqlDir);
  Table->SqlExt           = strdup(SqlExt);
  Table->SqlGrantExt      = strdup(SqlGrantExt);
  Table->SqlIdxExt        = strdup(SqlIdxExt);
  Table->SqlProcExt       = strdup(SqlProcExt);
  Table->SqlSnapExt       = strdup(SqlSnapExt);
  Table->SqlTableExt      = strdup(SqlTableExt);
  Table->SqlViewsExt      = strdup(SqlViewsExt);
  Table->UConnect         = strdup(UConnect);
  Table->BlankPadded      = BlankPadded;
  Table->CharZ            = CharZ;
  Table->ConnReqd         = ConnReqd;
  Table->ExitReqd         = ExitReqd;
  Table->Internal         = Internal;
  Table->OneSQLScript     = OneSQLScript;
  Table->TargetC          = TargetC;
  Table->TargetIDL        = TargetIDL;
  Table->TargetSQL        = TargetSQL;
  Table->UnderScore       = UnderScore;
  GetOther(Table->ArchiveDir,      "ArchiveDir");
  GetOther(Table->ArchiveExt,      "ArchiveExt", ".arc");
  GetOther(Table->CSAdoNetDir,     "CSAdoNetDir");
  GetOther(Table->CSAdoNetExt,     "CSAdoNetExt", ".cs");
  GetOther(Table->CSIDL2Dir,       "CSIDL2Dir");
  GetOther(Table->CSIDL2Ext,       "CSIDL2Ext", ".cs");
  GetOther(Table->CSNet7Dir,       "CSNet7Dir");
  GetOther(Table->CSNet7Ext,       "CSNet7Ext", ".cs");
  GetOther(Table->EightByte,       "EightByte");
  GetOther(Table->ExtendedC,       "ExtendedC", 1);
  GetOther(Table->ExtendedPAS,     "ExtendedPAS");
  GetOther(Table->ExtendedVB,      "ExtendedVB");
  GetOther(Table->IDLModule,       "IDLModule");
  GetOther(Table->LittleTrue,      "LittleTrue", 1);
  GetOther(Table->NoDomain,        "NoDomain");
  GetOther(Table->PARMSDescr,      "PARMSDescr");
  GetOther(Table->PARMSDir,        "PARMSDir");
  GetOther(Table->PARMSExt,        "PARMSExt", ".pi");
  GetOther(Table->PARMSLookup,     "PARMSLookup");
  GetOther(Table->PASDir,          "PASDir");
  GetOther(Table->PASExt,          "PASExt", ".pas");
  GetOther(Table->PrefixVBClasses, "PrefixVBClasses");
  GetOther(Table->PythonDir,       "PythonDir");
  GetOther(Table->PythonExt,       "PythonExt", ".py");
  GetOther(Table->Show,            "Show");
  GetOther(Table->TargetARCHIVE,   "TargetARCHIVE");
  GetOther(Table->TargetCSAdoNet,  "TargetCSAdoNet");
  GetOther(Table->TargetCSIDL2,    "TargetCSIDL2");
  GetOther(Table->TargetCSNet7,    "TargetCSNet7");
  GetOther(Table->TargetPARMS,     "TargetPARMS");
  GetOther(Table->TargetPAS,       "TargetPAS");
  GetOther(Table->TargetPython,    "TargetPython");
  GetOther(Table->TargetSO,        "TargetSO", 1);
  GetOther(Table->TargetVB,        "TargetVB");
  GetOther(Table->TargetVB5,       "TargetVB5");
  GetOther(Table->TargetVBCode3,   "TargetVBCode3");
  GetOther(Table->TargetVBNet7,    "TargetVBNet7");
  GetOther(Table->TargetVBforADOR, "TargetVBforADOR");
  GetOther(Table->TargetVBforIDL,  "TargetVBforIDL");
  GetOther(Table->VB5Dir,          "VB5Dir");
  GetOther(Table->VBCode3Dir,      "VBCode3Dir");
  GetOther(Table->VBDir,           "VBDir");
  GetOther(Table->VBExt,           "VBExt", ".bas");
  GetOther(Table->VBNet7Dir,       "VBNet7Dir");
  GetOther(Table->VBNet7Ext,       "VBNet7Ext", ".vb");
  GetOther(Table->VBforIDLDir,     "VBforIDLDir");
  GetOther(Table->Version2Bin,     "Version2Bin");
  GetOther(Table->ViewOnly,        "ViewOnly");
}

int main(int argc, char *argv[])
{
  argc = GetArgs(argc, argv, argtab, TABSIZE);
  yyerrfile = stdout;
  if (argc < 2)
  {
    fprintf(yyerrfile, "Release(16.12) deprecated with switches Compiled %s\n", __DATE__);
    fprintf(yyerrfile, "Usage: pocioci_sw <options> filename\n"
                       "where filename is fully qualified path\n");
    PrUsage(argtab, TABSIZE);
    fprintf(yyerrfile, "\nCheck a current PortalIDE project file for switches\n"
                       "that can be used with the -o switch, be aware that\n"
                       "some of the switches are already covered by standard\n"
                       "switches and some are purely IDE switches only and\n"
                       "will have no effect on the compile.\n");
    return 1;
  }
  yyerrsrc = argv[1];
  char *Buffer = 0;
  int infile = open(yyerrsrc, O_RDONLY|O_BINARY);
  if (infile < 0)
    return 1;
  long size = filelength(infile);
  Buffer = (char*)calloc(1, size+16);
  read(infile, Buffer, size);
  close(infile);
  Table = new TYYTable;
  Table->InputFileName = strdup(yyerrsrc);
  SetSwitches();
  yyhandler = OutputHandler;
  int rc = inbuf_parse(Table, Buffer);
  if (rc == 0) 
  {
    printf("%s parses ok, generating\n", yyerrsrc); 
    fflush(stdout);
    GenerateOCI(Table);
  }
  else
  {
    printf("%s fails yyparse\n", yyerrsrc); 
    fflush(stdout);
  }
  return rc;
}

