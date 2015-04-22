#include "machine.h"

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>

#include "sqlcons.h"

#include "sqldef.h"

#include "global.h"
#include "xdir.h"
#include "binio.h"

//static ushort sqldefSignOCI  = 0x0C1A;
static ushort sqldefSignOCI2 = 0x0C2A;

#if defined(M_DOS) && !defined(__BORLANDC__)
#define scc static const char __based(__segname("_CODE"))
#else
#define scc static const char
#endif

scc Connect0[]      = "CONNECT %s/%s\n\n"
                      "SET HEADING OFF\n"
                      "SPOOL %s\n\n";
scc Connect1[]      = "SET HEADING OFF\n"
                      "SPOOL %s.log\n\n";
scc Connect2[]      = "CONNECT %s/%s%s\n\n"
                      "SET HEADING OFF\n"
                      "SPOOL %s\n\n";
scc Exit[]          = "\nSPOOL OFF"
                      "\nEXIT;\n\n";
scc Storage0[]      = "\n  STORAGE\n  (";
scc Storage9[]      = "\n  )";
scc TableSpace[]    = " TABLESPACE %s";
scc PctFree[]       = "\n  PCTFREE %u";
scc PctUsed[]       = "\n  PCTUSED %u";
scc IniTrans[]      = "\n  INITRANS %u";
scc MaxTrans[]      = "\n  MAXTRANS %u";
scc Initial[]       = "\n    INITIAL %u%c";
scc Next[]          = "\n    NEXT %u%c";
scc MinExtents[]    = "\n    MINEXTENTS %u";
scc MaxExtents[]    = "\n    MAXEXTENTS %u";
scc PctIncrease[]   = "\n    PCTINCREASE %u";
scc FreeLists[]     = "\n    FREELISTS %u";
scc FreeListGroups[]= "\n    FREELIST GROUPS %u";
scc DropTable1[]    = "DROP TABLE %s";
scc DropTable2[]    = " CASCADE CONSTRAINTS";
scc DropTable3[]    = ";\n\n";
scc CrTable0[]      = "CREATE TABLE %s\n"
                      "(";
scc CrTable1a[]     = "\n %c%s %s";
scc CrTable2[]      = "\n) /* approx row size %d */";
scc CrTable9[]      = ";\n\n";
scc DropAudit1[]    = "DROP TABLE %sAudit;\n\n";
scc CrAudit0[]      = "CREATE TABLE %sAudit\n"
                      "("
                      "\n  AuditChangeSeq number(12)"
                      "\n ,AuditChangeAction char(2)";
scc CrAudit0a[]     = "\n ,AuditChangeUserID char(8)"
                      "\n ,AuditChangeTmStamp date"
                      "\n ,AuditChangeRowID rowid";
scc CrAudit1[]      = "\n ,%s %s";
scc CrAudit2[]      = "\n);\n\n";
scc CrAudit3[]      = "DROP SEQUENCE %sAuditSeq;\n\n";
scc CrAudit4[]      = "CREATE SEQUENCE %sAuditSeq\n"
                      "  MINVALUE 1\n"
                      "  MAXVALUE 999999999999\n"
                      "  CYCLE\n"
                      "  ORDER;\n\n";
scc CrAudit5[]      = "DROP TRIGGER %sTrig;\n\n";
scc CrAudit6[]      = "CREATE TRIGGER %sTrig\n";
scc CrAudit7[]      = "AFTER UPDATE OR DELETE ON %s FOR EACH ROW\n"
                      "DECLARE\n"
                      "  Oper1 char(2);\n"
                      "BEGIN\n"
                      "  IF UPDATING THEN\n"
                      "    Oper1 := 'OC';\n"
                      "  END IF;\n"
                      "  IF DELETING THEN\n"
                      "    Oper1 := 'OD';\n"
                      "  END IF;\n";
scc CrAudit7F[]     = "AFTER INSERT OR UPDATE OR DELETE ON %s FOR EACH ROW\n"
                      "DECLARE\n"
                      "  Oper1 char(2);\n"
                      "  Oper2 char(2);\n"
                      "BEGIN\n"
                      "  IF INSERTING THEN\n"
                      "    Oper2 := 'NA';\n"
                      "  END IF;\n"
                      "  IF UPDATING THEN\n"
                      "    Oper1 := 'OC';\n"
                      "    Oper2 := 'NC';\n"
                      "  END IF;\n"
                      "  IF DELETING THEN\n"
                      "    Oper1 := 'OD';\n"
                      "  END IF;\n";
scc CrAudit8O[]     = "  IF UPDATING OR DELETING THEN\n"
                      "    INSERT INTO %sAudit VALUES\n"
                      "    (\n";
scc CrAudit8N[]     = "  IF UPDATING OR INSERTING THEN\n"
                      "    INSERT INTO %sAudit VALUES\n"
                      "    (\n";
scc CrAudit9O[]     = "      %sAuditSeq.NEXTVAL\n"
                      "    , Oper1\n";
scc CrAudit9Oa[]    = "    , USER\n"
                      "    , SYSDATE\n";
scc CrAudit9N[]     = "      %sAuditSeq.NEXTVAL\n"
                      "    , Oper2\n";
scc CrAudit9Na[]    = "    , USER\n"
                      "    , SYSDATE\n";
scc CrAudit10O[]    = "    , :OLD.%s\n";
scc CrAudit10N[]    = "    , :NEW.%s\n";
scc CrAudit11[]     = "    );\n"
                      "  END IF;\n";
scc CrAudit12[]     = "END;\n"
                      "/\n"
                      "show errors\n\n";
scc Syn1[]          = "DROP PUBLIC SYNONYM %s;\n"
                      "CREATE PUBLIC SYNONYM %s for %s;\n"
                      "GRANT SELECT, INSERT, DELETE, UPDATE "
                      "on %s to %s;\n\n";
scc Syn2[]          = "DROP PUBLIC SYNONYM %s%s%s;\n"
                      "CREATE PUBLIC SYNONYM %s%s%s for %s%s%s;\n"
                      "GRANT EXECUTE on %s%s%s to %s;\n\n";
scc Syn3[]          = "DROP PUBLIC SYNONYM %s;\n"
                      "CREATE PUBLIC SYNONYM %s for %s.%s;\n"
                      "GRANT SELECT on %s.%s to %s;\n\n";
scc Syn4[]          = "DROP PUBLIC SYNONYM %sSeq;\n"
                      "CREATE PUBLIC SYNONYM %sSeq for %sSeq;\n"
                      "GRANT SELECT on %sSeq to %s;\n\n";
scc Syn5[]          = "DROP PUBLIC SYNONYM %sAudit;\n"
                      "CREATE PUBLIC SYNONYM %sAudit for %sAudit;\n"
                      "GRANT SELECT on %sAudit to %s;\n\n";
scc DropSeq[]       = "DROP SEQUENCE %sSeq;\n\n";
scc CrSeq[]         = "CREATE SEQUENCE %sSeq\n"
                      "  MINVALUE 1\n"
                      "  MAXVALUE 999999999\n"
                      "  CYCLE\n"
                      "  ORDER;\n\n";
scc AlTable0[]      = "ALTER TABLE %s\n"
                      "ADD(";
scc AlTable1[]      = "ALTER TABLE %s\n"
                      "MODIFY(";
scc AlTable2[]      = "\n%s %s CONSTRAINT %s_NN%02d NOT NULL";
scc AlTable3[]      = "\n%s CONSTRAINT %s_CH%02d CHECK(%s %s)";
scc AlTable4[]      = "\n%s CONSTRAINT %s_CH%02d CHECK(%s IN (";
scc AlTable5[]      = "%s%d";
scc AlTable6[]      = "))";
scc AlTable9[]      = "\n);\n";
scc CrKey0PK[]      = "\n  CONSTRAINT %s_PK%02d PRIMARY KEY"
                      "\n  (";
scc CrKey0UN[]      = "\n  CONSTRAINT %s_UK%02d UNIQUE"
                      "\n  (";
scc CrKey1[]        = "\n   %c%s";
scc CrKey9[]        = "\n  )";
scc CrFKey0[]       = "\n  CONSTRAINT %s_FK%02d FOREIGN KEY"
                      "\n  (";
scc CrFKey1[]       = "\n   %c%s";
scc CrFKey9[]       = "\n  ) REFERENCES %s";
scc CrFKey10[]      = " ON DELETE CASCADE";
scc UsingIndex[]    = " USING INDEX";
scc CrIndex0a[]     = "CREATE %s INDEX %s_IX%02d ON %s\n(";
scc CrIndex1[]      = "\n %c%s";
scc CrIndex2[]      = "\n)";
scc CrIndex9[]      = ";\n\n";
scc CrGrant0[]      = "GRANT %s ON %s.%s TO %s;\n\n";
scc CrGrant0a[]     = "GRANT SELECT ON %s.%sSeq TO %s;\n\n";
scc CrGrant1[]      = "GRANT EXECUTE ON %s.%s%s%s TO %s;\n\n";
scc CrGrant1b[]     = "GRANT EXECUTE ON %s%s%s TO %s;\n\n";
scc CrGrant2[]      = "GRANT SELECT ON %s%s%s TO %s;\n\n";
scc CrViews0[]      = "CREATE OR REPLACE FORCE VIEW %s%s%s as\n"
                      "(\n";
scc CrViews1[]      = "%s\n";
scc CrViews9[]      = ");\n"
                      "l\n"
                      "show errors\n";
scc SPInsert0[]     = "CREATE OR REPLACE PROCEDURE %s%sInsert\n"
                      "(\n";
scc SPInsert1[]     = "%c a%s%s %s\n";
scc SPInsert2a[]    = ") as\n"
                      "BEGIN\n";
scc SPInsert2r[]    = "  select RowSequence.NEXTVAL into a%s%s from dual;\n";
scc SPInsert2b[]    = "  select %sSeq.NEXTVAL into a%s%s from dual;\n";
scc SPInsert2d[]    = "  a%s%s := to_char(SYSDATE, 'YYYYMMDDHH24MISS');\n";
scc SPInsert2e[]    = "  a%s%s := SUBSTR(USER, 1, 8);\n";
scc SPInsert2t[]    = "  TableRevisionsLatest('%s');\n";
scc SPInsert2c[]    = "  insert into %s\n"
                      "  (\n";
scc SPInsert2[]     = "  %c %s\n";
scc SPInsert2v[]    = "  ) values\n"
                      "  (\n";
scc SPInsert2s[]    = "  a%s%s := 'I';\n";
scc SPInsert3[]     = "  %c a%s%s\n";
scc SPInsert3d[]    = "  %c SYSDATE\n";
scc SPInsert3m[]    = "  %c to_number(a%s%s)\n";
scc SPInsert3t[]    = "  %c to_date(a%s%s,'YYYYMMDD')\n";
scc SPInsert3dt[]   = "  %c to_date(a%s%s,'YYYYMMDDHH24MISS')\n";
scc SPInsert4[]     = "  );\n";
scc SPInsert5[]     = "EXCEPTION\n"
                      "  WHEN DUP_VAL_ON_INDEX THEN\n"
                      "    %s%sUpdate\n"
                      "    (\n";
scc SPInsert6[]     = "    %c a%s%s\n";
scc SPInsert7[]     = "    );\n";
scc SPInsert9[]     = "END;\n/\nl\nshow errors\n\n";
scc SPSelOne0[]     = "CREATE OR REPLACE PROCEDURE %s%sSelectOne%s\n"
                      "(\n";
scc SPSelOne1[]     = "%c a%s%s %s %s\n";
scc SPSelOne2[]     = ") as\n"
                      "BEGIN\n"
                      "  select\n";
scc SPSelOne3[]     = "  %c %s\n";
scc SPSelOne3d[]    = "  %c to_char(%s,'YYYYMMDDHH24MISS')\n";
scc SPSelOne3m[]    = "  %c to_char(%s)\n";
scc SPSelOne3t[]    = "  %c to_char(%s,'YYYYMMDD')\n";
scc SPSelOne3dt[]   = "  %c to_char(%s,'YYYYMMDDHH24MISS')\n";
scc SPSelOne4[]     = "  into\n";
scc SPSelOne5[]     = "  %c a%s%s\n";
scc SPSelOne6[]     = "  from %s\n";
scc SPSelOne7[]     = "  %5s %s=a%s%s\n";
scc SPSelOne7m[]    = "  %5s %s=to_number(a%s%s)\n";
scc SPSelOne7t[]    = "  %5s %s=to_date(a%s%s,'YYYYMMDD')\n";
scc SPSelOne7dt[]   = "  %5s %s=to_date(a%s%s,'YYYYMMDDHH24MISS')\n";
scc SPSelOne8u[]    = "  for update";
scc SPSelOne8[]     = "  ;\n";
scc SPSelOne8x[]    = "  if aRowStatus = 'D' then\n"
                      "    RAISE NO_DATA_FOUND;\n"
                      "  end if;\n";
scc SPSelOne9[]     = "END;\n/\nl\nshow errors\n\n";
scc SPDelOne0[]     = "CREATE OR REPLACE PROCEDURE %s%sDeleteOne\n"
                      "(\n";
scc SPDelOne1[]     = "%c a%s%s in %s\n";
scc SPDelOne2[]     = ") as\n"
                      "BEGIN\n"
                      "  delete from %s\n";
scc SPDelUpd[]      = ") as\n"
                      "BEGIN\n"
                      "  TableRevisionsLatest('%s');\n"
                      "  update %s\n"
                      "  set\n"
                      "    RowStatus = 'D'\n"
                      "  , RowUpdated = SYSDATE\n";
scc SPDelOne3[]     = "  %5s %s=a%s%s\n";
scc SPDelOne3m[]    = "  %5s %s=to_number(a%s%s)\n";
scc SPDelOne3t[]    = "  %5s %s=to_date(a%s%s, 'YYYYMMDD')\n";
scc SPDelOne3dt[]   = "  %5s %s=to_date(a%s%s, 'YYYYMMDDHH24MISS')\n";
scc SPDelOne4[]     = "  ;\n"
                      "END;\n/\nl\nshow errors\n\n";
scc SPUpdate0[]     = "CREATE OR REPLACE PROCEDURE %s%sUpdate\n"
                      "(\n";
scc SPUpdate1[]     = "%c a%s%s %s\n";
scc SPUpdate2a[]    = ") as\n"
                      "wSYSDATE date;\n"
                      "BEGIN\n";
scc SPUpdate2z[]    = "  wSYSDATE := SYSDATE;\n";
scc SPUpdate2d[]    = "  a%s%s := to_char(wSYSDATE, 'YYYYMMDDHH24MISS');\n";
scc SPUpdate2e[]    = "  a%s%s := SUBSTR(USER, 1, 8);\n";
scc SPUpdate2t[]    = "  TableRevisionsLatest('%s');\n";
scc SPUpdate2b[]    = "  update %s\n"
                      "  set\n";
scc SPUpdate2s[]    = "  a%s%s := 'U';\n";
scc SPUpdate3[]     = "  %c %s = a%s%s\n";
scc SPUpdate3m[]    = "  %c %s = to_number(a%s%s)\n";
scc SPUpdate3t[]    = "  %c %s = to_date(a%s%s,'YYYYMMDD')\n";
scc SPUpdate3dt[]   = "  %c %s = to_date(a%s%s,'YYYYMMDDHH24MISS')\n";
scc SPUpdate3d[]    = "  %c %s = wSYSDATE\n";
scc SPUpdate4[]     = "  %5s %s = a%s%s\n";
scc SPUpdate4m[]    = "  %5s %s = to_number(a%s%s)\n";
scc SPUpdate4t[]    = "  %5s %s = to_date(a%s%s,'YYYYMMDD')\n";
scc SPUpdate4dt[]   = "  %5s %s = to_date(a%s%s,'YYYYMMDDHH24MISS')\n";
scc SPUpdate5[]     = "  ;\n"
                      "END;\n/\nl\nshow errors\n\n";
scc SPUsrProc0a[]    = "CREATE OR REPLACE PROCEDURE %s%s%s\n"
                       "(\n";
scc SPUsrProc0b[]    = "CREATE OR REPLACE PROCEDURE %s%s%s\n"
                       "as\n";
scc SPUsrProc1[]    = "%c a%s%s %s %s %s\n";
scc SPUsrProc2[]    = ") as\n";
scc SPUsrProc3[]    = "%s %s;\n";
scc SPUsrProc3a[]   = "CURSOR %s%s\n";
scc SPUsrProc3b[]   = "  %s%s\n";
scc SPUsrProc4[]    = "BEGIN\n";
scc SPUsrProc5[]    = "%s";
scc SPUsrProc6[]    = "END;\n/\nl\nshow errors\n\n";
scc SPDataLine0[]   = "%s\n";

static char *UpperName;
static char *ControlName;

static FILE *SqlFile      = 0;
static FILE *SqlTableFile = 0;
static FILE *SqlViewsFile = 0;
static FILE *SqlConFile   = 0;
static FILE *SqlIdxFile   = 0;
static FILE *SqlGrantFile = 0;
static FILE *SqlProcFile  = 0;
static FILE *SqlSnapFile  = 0;
static FILE *SqlAuditFile = 0;
static FILE *BinFile      = 0;

static void GenerateSyn1(FILE * file, const char *Name, const char *FullName, const char *User);
static void GenerateSyn2(FILE * file, const char *Name, const char *FullName, const char *User, const char *ProcName, const char *Under);
static void GenerateSyn3(FILE * file, const char *Name, const char *DBName,   const char *User);
static void GenerateSyn4(FILE * file, const char *Name, const char *FullName, const char *User);
static void GenerateSql(PYYTable Table);
static void GenerateTable(PYYTable Table);
static void GenerateAudit(PYYTable Table);
static void GenerateViews(PYYTable Table);
static void GenerateView(PYYTable Table, PYYView View);
static void GenerateSequence(PYYTable Table);
static void GenerateConstraints(PYYTable Table);
static void GenerateKeys(PYYTable Table);
static void GenerateKey(PYYTable Table, PYYKey Key, ushort *PkNo, ushort *UkNo);
static void GenerateNulls(PYYTable Table);
static void GenerateLinks(PYYTable Table);
static void GenerateLink(PYYTable Table, PYYLink Link, short FkNo);
static void GenerateIndexes(PYYTable Table);
static void GenerateIndex(PYYTable Table, PYYKey Key, pshort IxNo);
static void GenerateGrants(PYYTable Table);
static void GenerateGrant(PYYTable Table, PYYGrant Grant);
static void GenerateProcs(PYYTable Table);
static void GenerateStdProcs(PYYTable Table);
static void GenerateInsert(PYYTable Table);
static void GenerateSelectOne(PYYTable Table, bool ForUpd);

static void GenerateConnect(PYYTable Table, FILE* SqlFile);
static void GenerateExit(PYYTable Table, FILE* SqlFile);

static void GenerateDeleteOne(PYYTable Table);
static void GenerateUpdate(PYYTable Table);
static void GenerateUsrProcs(PYYTable Table);
static void GenerateUsrProc(PYYTable Table, PYYProc Proc);
static void GenerateDataLines(PYYProc Proc);

static void GenerateBinary(PYYTable Table);
static void GenerateBinTable(PYYTable Table);
static void GenerateBinProcs(PYYTable Table);
static void GenerateBinSelectAll(PYYTable Table, bool ForUpd);
static void GenerateBinProc(PYYTable Table, const char *ProcName,
                            ushort noFields, PYYField Fields,
                            ushort ABSize, const char *Code,
                            PYYField ExtraField,
                            uchar  testSeq);
static void GenerateBinSql(PYYTable Table, PYYProc Proc);

static char *CutUser(const char *aConnect);

static const char *ParmFieldType(PYYField Field);
static const char *ParmFieldTypeInsert(PYYField Field);
static const char *ParmFieldTypeUpdate(PYYField Field);
static const char *SqlFieldType(PYYField Field);
static ushort OCIFieldType(eFieldType FT);
static short SQLCFieldType(eFieldType FT);
static int ApproxFieldSize(PYYField Field);
static void MakeSQLNames(PYYTable Table);

void GenerateOCI(PYYTable Table)
{
  UpperName = strdup(Table->Name);
  if (strlen(Table->ControlDB))
  {
    ControlName = (char *) malloc(strlen(Table->Name)+strlen(Table->ControlDB)+2);
    strcpy(ControlName, Table->ControlDB);
    strcat(ControlName, ".");
    strcat(ControlName, Table->Name);
  }
  else
    ControlName = strdup(Table->Name);
  strupr(UpperName);
  MakeSQLNames(Table);
  if (Table->TargetSQL)
    GenerateSql(Table);
  fflush(stdout);
  if (Table->TargetVB || Table->MarkedVB)
    GenerateVB(Table);
  fflush(stdout);
  if (Table->TargetVB5 || Table->MarkedVB5)
    GenerateVB5(Table);
  fflush(stdout);
  if (Table->TargetVBCode3 || Table->MarkedVBCode3)
    GenerateVBCode3(Table);
  fflush(stdout);
  if (Table->TargetVBNet7 || Table->MarkedVBNet7)
    GenerateVBNet7(Table);
  fflush(stdout);
  if (Table->TargetCSNet7 || Table->MarkedCSNet7)
    GenerateCSNet7(Table);
  fflush(stdout);
  if (Table->TargetCSAdoNet || Table->MarkedCSAdoNet)
    GenerateCSAdoNet(Table);
  fflush(stdout);
  //if (Table->TargetCSDap || Table->MarkedCSDap)
  //  GenerateCSDap(Table);
  if (Table->TargetCSRW || Table->MarkedCSRW)
    GenerateCSRW(Table);
  fflush(stdout);
  if (Table->TargetCSIDL2 || Table->MarkedCSIDL2)
    GenerateCSIDL2(Table);
  fflush(stdout);
  if (Table->TargetVBforIDL || Table->MarkedVBforIDL)
    GenerateVBforIDL(Table);
  fflush(stdout);
  if (Table->TargetVBforADOR || Table->MarkedVBforADOR)
    GenerateVBforADOR(Table);
  fflush(stdout);
  if (Table->TargetPAS || Table->MarkedPAS)
    GeneratePAS(Table);
  fflush(stdout);
  if (Table->TargetPARMS)
    GeneratePARMS(Table);
  fflush(stdout);
  if (Table->TargetC || Table->MarkedC)
    GenerateHeader(Table);
  fflush(stdout);
  if (Table->TargetIDL || Table->MarkedIDL)
    GenerateIDL(Table);
  fflush(stdout);
  if (Table->TargetPython || Table->MarkedPython)
    GeneratePython(Table);
  fflush(stdout);
  if (Table->TargetSO)
    GenerateBinary(Table);
  fflush(stdout);
  if (Table->TargetARCHIVE)
    GenerateArchive(Table);
  fflush(stdout);
  free(UpperName);
  free(ControlName);
}

static FILE* OpenFile(const char *FileFlag, const char *FileName, const char *Mode = "wt")
{
  char Work[512];
  sprintf(Work, "%-13.13s: %s\n", FileFlag, FileName);
  yyerror(Work);
  FILE* File = fopen(FileName, Mode);
  if (File == 0)
    yyerror("Cannot open %s : %s\n", FileFlag, FileName);
  else
    setvbuf(File, 0, _IOFBF, 32768);
  return File;
}

static void MakeSQLNames(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->SqlDir))
    FNameMerge(newpath, Table->SqlDir, fname, Table->SqlExt);
  else
    FNameMerge(newpath, dir, fname, Table->SqlExt);
  Table->SqlFileName = strdup(newpath);
  if (strlen(Table->LogDir))
    FNameMerge(newpath, Table->LogDir, fname, Table->LogExt);
  else
    FNameMerge(newpath, dir, fname, Table->LogExt);
  Table->LogFileName = strdup(newpath);
  if (Table->OneSQLScript == 0)
  {
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlConExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlConExt);
    Table->SqlConFileName = strdup(newpath);
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlTableExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlTableExt);
    Table->SqlTableFileName = strdup(newpath);
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlGrantExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlGrantExt);
    Table->SqlGrantFileName = strdup(newpath);
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlProcExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlProcExt);
    Table->SqlProcFileName = strdup(newpath);
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlViewsExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlViewsExt);
    Table->SqlViewsFileName = strdup(newpath);
    if (strlen(Table->SqlDir))
      FNameMerge(newpath, Table->SqlDir, fname, Table->SqlIdxExt);
    else
      FNameMerge(newpath, dir, fname, Table->SqlIdxExt);
    Table->SqlIdxFileName = strdup(newpath);
  }
  if (strlen(Table->SqlDir))
    FNameMerge(newpath, Table->SqlDir, fname, Table->SqlSnapExt);
  else
    FNameMerge(newpath, dir, fname, Table->SqlSnapExt);
  Table->SqlSnapFileName = strdup(newpath);
  if (strlen(Table->SqlDir))
    FNameMerge(newpath, Table->SqlDir, fname, Table->SqlAuditExt);
  else
    FNameMerge(newpath, dir, fname, Table->SqlAuditExt);
  Table->SqlAuditFileName = strdup(newpath);
}

static void GenerateConnect(PYYTable Table, FILE* SqlFile)
{
  if (Table->ConnReqd)
  {
    if (Table->Server[0] == '@')
      fprintf(SqlFile, Connect2, Table->UserID, Table->PassWD,
                                 Table->Server, Table->LogFileName);
    else
      fprintf(SqlFile, Connect0, Table->UserID, Table->PassWD, Table->LogFileName);
  }
  else
    fprintf(SqlFile, Connect1, Table->LogFileName);
}

static void GenerateExit(PYYTable Table, FILE* SqlFile)
{
  if (Table->ExitReqd)
    fprintf(SqlFile, Exit);
}

static void GenerateSql(PYYTable Table)
{
  SqlFile = OpenFile("SQL File", Table->SqlFileName);
  if (SqlFile == 0)
     return;
  if (strlen(Table->Name) > 0)
  {
    GenerateConnect(Table, SqlFile);
    fprintf(SqlFile, "select 'LAST RUN: '||to_char(sysdate, 'YYYY/MM/DD HH24:MI:SS') \"LASTRUN\" from dual;\n\n");
    char Work[512];
    sprintf(Work, "%-13.13s: %s\n", "Log File", Table->LogFileName);
    yyerror(Work);
    GenerateTable(Table);
    if (Table->hasAudit)
      GenerateAudit(Table);
    GenerateConstraints(Table);
    GenerateIndexes(Table);
    if (Table->noGrants)
      GenerateGrants(Table);
    GenerateViews(Table);
    GenerateProcs(Table);
    GenerateExit(Table, SqlFile);
  }
  fclose(SqlFile);
}

static void GenerateSyn1(FILE * file, const char *Name, const char *FullName, const char *User)
{
  fprintf(file, Syn1, Name,
                      Name, FullName,
                      FullName, User);
}

static void GenerateSyn2(FILE * file, const char *Name, const char *FullName, const char *User,
                                      const char *ProcName, const char *Under)
{
  fprintf(file, Syn2, Name, Under, ProcName,
                      Name, Under, ProcName,
                      FullName, Under, ProcName,
                      FullName, Under, ProcName, User);
}

static void GenerateSyn3(FILE * file, const char *Name, const char *DBName, const char *User)
{
  fprintf(file, Syn3, Name, Name, DBName, Name, DBName, Name, User);
}

static void GenerateSyn4(FILE * file, const char *Name, const char *FullName, const char *User)
{
  fprintf(file, Syn4, Name,
                      Name, FullName,
                      FullName, User);
}

static void GenerateSyn5(FILE * file, const char *Name, const char *FullName, const char *User)
{
  fprintf(file, Syn5, Name,
                      Name, FullName,
                      FullName, User);
}

static void GenerateConstraints(PYYTable Table)
{
  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlConFileName);
    SqlConFile = OpenFile("SQL File(Con)", Table->SqlConFileName);
    if (SqlConFile == 0)
      return;
    GenerateConnect(Table, SqlConFile);
  }
  else
    SqlConFile = SqlFile;
  GenerateLinks(Table);
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlConFile);
    fclose(SqlConFile);
  }
}

static void GenerateNulls(PYYTable Table)
{
  ushort i,n,m;
  for (i=0; i<Table->noFields; i++)
    if (!Table->Fields[i].isNull)
      break;
  if (i >= Table->noFields)
    return;
  fprintf(SqlTableFile, AlTable1, ControlName);
  for (i=n=0,m=1; i<Table->noFields; i++)
  {
    if (!Table->Fields[i].isNull)
      fprintf(SqlTableFile, AlTable2,
                          n++ ? "," : " ",
                          Table->Fields[i].Name,
                          Table->Name,
                          m++);
  }
  fprintf(SqlTableFile, AlTable9);
}

static void GenerateProcs(PYYTable Table)
{
  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlProcFileName);
    SqlProcFile = OpenFile("SQL File(Pro)", Table->SqlProcFileName);
    if (SqlProcFile == 0)
      return;
    GenerateConnect(Table, SqlProcFile);
  }
  else
    SqlProcFile = SqlFile;
  if (strlen(Table->UConnect) && strchr(Table->UConnect, '/'))
    fprintf(SqlProcFile, "connect %s\n\n", Table->UConnect);
  if (Table->Internal == 0)
    GenerateStdProcs(Table);
  GenerateUsrProcs(Table);
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlProcFile);
    fclose(SqlProcFile);
  }
}

static void GenerateStdProcs(PYYTable Table)
{
  if (Table->hasSelectOne && Table->hasPrimaryKey)
    GenerateSelectOne(Table, false);
  if (Table->hasSelectOneUpd && Table->hasPrimaryKey)
    GenerateSelectOne(Table, true);
  if (Table->hasDeleteOne && Table->hasPrimaryKey)
    GenerateDeleteOne(Table);
  if (Table->hasUpdate && Table->hasPrimaryKey)
    GenerateUpdate(Table);
  if (Table->hasInsert)
    GenerateInsert(Table);
}

static char *ProcSuffix(PYYTable Table)
{
  static char result[64];
  if (strlen(Table->UConnect))
  {
    sprintf(result, "%s.%s", CutUser(Table->UConnect), Table->Name);
    return result;
  }
  return ControlName;
}

static void ExecuteGrant(PYYTable Table, const char *ProcName, const char *User)
{
  if (strlen(Table->UConnect) == 0)
    fprintf(SqlProcFile, CrGrant1,
            strlen(Table->ControlDB) ? Table->ControlDB : Table->UserID,
            Table->Name,
            Table->UnderScore ? "_" : "",
            ProcName,
            User);
  else
    fprintf(SqlProcFile, CrGrant1b,
            Table->Name,
            Table->UnderScore ? "_" : "",
            ProcName,
            User);
}

static void GenerateInsert(PYYTable Table)
{
  ushort i;
  if (Table->noFields == 0) return;
  fprintf(SqlProcFile, SPInsert0, ProcSuffix(Table), Table->UnderScore ? "_" : "");
  for (i=0; i<Table->noFields; i++)
    fprintf(SqlProcFile, SPInsert1,
            i==0       ? ' ' : ',',
            Table->UnderScore ? "_" : "",
            Table->Fields[i].Name,
            ParmFieldTypeInsert(&Table->Fields[i]));
  fprintf(SqlProcFile, SPInsert2a);//, Table->Name);
  for (i=0; i<Table->noFields; i++)
  {
    if (Table->Fields[i].isRowSequence)
      fprintf(SqlProcFile, SPInsert2r,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].isSequence)
      fprintf(SqlProcFile, SPInsert2b, Table->Name,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].Type == ftypeTimeStamp)
      fprintf(SqlProcFile, SPInsert2d,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].Type == ftypeUserStamp)
      fprintf(SqlProcFile, SPInsert2e,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].Type == ftypeStatus)
      fprintf(SqlProcFile, SPInsert2s,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
  }
  fprintf(SqlProcFile, SPInsert2c, Table->Name);
  for (i=0; i<Table->noFields; i++)
    fprintf(SqlProcFile, SPInsert2, i==0?' ':',', Table->Fields[i].Name);
  fprintf(SqlProcFile, SPInsert2v);//, Table->Name);
  for (i=0; i<Table->noFields; i++)
    if (Table->Fields[i].Type == ftypeTimeStamp)
      fprintf(SqlProcFile, SPInsert3d, i==0?' ':',');
    else
      fprintf(SqlProcFile,
            Table->Fields[i].Type == ftypeDate
            ? SPInsert3t
            : Table->Fields[i].Type == ftypeDateTime
            ? SPInsert3dt
            : Table->Fields[i].Type == ftypeMoney
            ? SPInsert3m
            : SPInsert3,
            i==0?' ':',',
            Table->UnderScore ? "_" : "",
            Table->Fields[i].Name);
  fprintf(SqlProcFile, SPInsert4);
  fprintf(SqlProcFile, SPInsert9);
  GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, "Insert", Table->UnderScore ? "_" : "");
  for (i=0; i<Table->noInsertUsers; i++)
    ExecuteGrant(Table, "Insert", Table->InsertUsers[i].p);
  for (i=0; i<Table->noExecuteUsers; i++)
    ExecuteGrant(Table, "Insert", Table->ExecuteUsers[i].p);
}

static void GenerateSelectOne(PYYTable Table, bool ForUpd)
{
  ushort i;
  ushort first;
  if (Table->noFields == 0) return;
  fprintf(SqlProcFile, SPSelOne0,
                   ProcSuffix(Table),
                   Table->UnderScore ? "_" : "",
                   ForUpd ? "Upd" : "");
  for (i=0; i<Table->noFields; i++)
    fprintf(SqlProcFile, SPSelOne1,
            i==0?' ':',',
            Table->UnderScore ? "_" : "",
            Table->Fields[i].Name,
            Table->Fields[i].isPrimaryKey?"in":"in out",
            ParmFieldType(&Table->Fields[i]));
  fprintf(SqlProcFile, SPSelOne2);
  for (i=0,first=0; i<Table->noFields; i++)
    if (Table->Fields[i].isPrimaryKey == 0)
    {
      if (Table->Fields[i].Type == ftypeTimeStamp)
        fprintf(SqlProcFile,
            SPSelOne3d,
            first==0?' ':',',
            Table->Fields[i].Name);
      else
        fprintf(SqlProcFile,
            Table->Fields[i].Type == ftypeDate
              ? SPSelOne3t
              : Table->Fields[i].Type == ftypeDateTime
              ? SPSelOne3dt
              : Table->Fields[i].Type == ftypeMoney
              ? SPSelOne3m
              : SPSelOne3,
            first==0?' ':',',
            Table->Fields[i].Name);
      first=1;
    }
  fprintf(SqlProcFile, SPSelOne4);
  for (i=0,first=0; i<Table->noFields; i++)
    if (Table->Fields[i].isPrimaryKey == 0)
    {
      fprintf(SqlProcFile, SPSelOne5,
              first==0?' ':',',
              Table->UnderScore ? "_" : "",
              Table->Fields[i].Name);
      first=1;
    }
  fprintf(SqlProcFile, SPSelOne6, Table->Name);
  for (i=0,first=0; i<Table->noFields; i++)
    if (Table->Fields[i].isPrimaryKey)
    {
      fprintf(SqlProcFile,
              Table->Fields[i].Type == ftypeDate
                ? SPSelOne7t
                : Table->Fields[i].Type == ftypeDateTime
                ? SPSelOne7dt
                : Table->Fields[i].Type == ftypeMoney
                ? SPSelOne7m
                : SPSelOne7,
              first==0?"where":"and",
              Table->Fields[i].Name,
              Table->UnderScore ? "_" : "",
              Table->Fields[i].Name);
      first=1;
    }
  if (ForUpd)
     fprintf(SqlProcFile, SPSelOne8u);
  fprintf(SqlProcFile, SPSelOne8);
  fprintf(SqlProcFile, SPSelOne9);
  if (ForUpd)
    GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, "SelectOneUpd", Table->UnderScore ? "_" : "");
  else
    GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, "SelectOne", Table->UnderScore ? "_" : "");
  if (ForUpd)
  {
    for (i=0; i<Table->noSelectOneUpdUsers; i++)
      ExecuteGrant(Table, "SelectOneUpd", Table->InsertUsers[i].p);
    for (i=0; i<Table->noExecuteUsers; i++)
      ExecuteGrant(Table, "SelectOneUpd", Table->ExecuteUsers[i].p);
  }
  else
  {
    for (i=0; i<Table->noSelectOneUsers; i++)
      ExecuteGrant(Table, "SelectOne", Table->InsertUsers[i].p);
    for (i=0; i<Table->noExecuteUsers; i++)
      ExecuteGrant(Table, "SelectOne", Table->ExecuteUsers[i].p);
  }
}


static void GenerateDeleteOne(PYYTable Table)
{
  ushort i;
  ushort first;
  if (Table->noFields == 0) return;
  if (Table->noKeys == 0) return;
  fprintf(SqlProcFile, SPDelOne0, ProcSuffix(Table), Table->UnderScore ? "_" : "");
  for (i=0,first=0; i<Table->Keys[0].noFields; i++)
  {
    fprintf(SqlProcFile, SPDelOne1,
            first==0?' ':',',
            Table->UnderScore ? "_" : "",
            Table->Keys[0].Fields[i].Name,
            ParmFieldType(&Table->Keys[0].Fields[i]));
    first=1;
  }
  fprintf(SqlProcFile, SPDelOne2, Table->Name);
  for (i=0,first=0; i<Table->Keys[0].noFields; i++)
  {
    fprintf(SqlProcFile,
            Table->Keys[0].Fields[i].Type == ftypeDate
              ? SPDelOne3t
              : Table->Keys[0].Fields[i].Type == ftypeDateTime
              ? SPDelOne3dt
              : Table->Keys[0].Fields[i].Type == ftypeMoney
              ? SPDelOne3m
              : SPDelOne3,
            first==0?"where":"and",
            Table->Keys[0].Fields[i].Name,
            Table->UnderScore ? "_" : "",
            Table->Keys[0].Fields[i].Name);
    first=1;
  }
  fprintf(SqlProcFile, SPDelOne4);
  GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, "DeleteOne", Table->UnderScore ? "_" : "");
  for (i=0; i<Table->noDeleteOneUsers; i++)
    ExecuteGrant(Table, "DeleteOne", Table->InsertUsers[i].p);
  for (i=0; i<Table->noExecuteUsers; i++)
    ExecuteGrant(Table, "DeleteOne", Table->ExecuteUsers[i].p);
}

static void GenerateUpdate(PYYTable Table)
{
  ushort i;
  ushort first;
  uchar hasTimeStamp = 0;

  // cannot have an update if no non-primary-key fields
  if (Table->noFields - Table->PrimaryKeyFields <= 0)
    return;

  fprintf(SqlProcFile, SPUpdate0, ProcSuffix(Table), Table->UnderScore ? "_" : "");
  for (i=0,first=0; i<Table->noFields; i++)
    {
    if (Table->Fields[i].isSequence   == 1
    &&  Table->Fields[i].isPrimaryKey == 0)
      continue;
    if (Table->Fields[i].isRowSequence == 1)
      continue;
    if (Table->Fields[i].Type == ftypeTimeStamp)
      hasTimeStamp = 1;
    fprintf(SqlProcFile, SPUpdate1,
            first==0?' ':',',
            Table->UnderScore ? "_" : "",
            Table->Fields[i].Name,
            ParmFieldTypeUpdate(&Table->Fields[i]));
    first = 1;
    }
  fprintf(SqlProcFile, SPUpdate2a);
  if (hasTimeStamp)
    fprintf(SqlProcFile, SPUpdate2z);
  for (i=0; i<Table->noFields; i++)
  {
    if (Table->Fields[i].Type == ftypeTimeStamp)
      fprintf(SqlProcFile, SPUpdate2d,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].Type == ftypeUserStamp)
      fprintf(SqlProcFile, SPUpdate2e,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
    else if (Table->Fields[i].Type == ftypeStatus)
      fprintf(SqlProcFile, SPUpdate2s,
                       Table->UnderScore ? "_" : "",
                       Table->Fields[i].Name);
  }
  fprintf(SqlProcFile, SPUpdate2b, Table->Name);
  for (i=0,first=0; i<Table->noFields; i++)
    if (Table->Fields[i].isPrimaryKey == 0
    &&  Table->Fields[i].isSequence   == 0
    &&  Table->Fields[i].isRowSequence == 0)
      {
      if (Table->Fields[i].Type == ftypeTimeStamp)
        fprintf(SqlProcFile, SPUpdate3d, first==0?' ':',',
                             Table->Fields[i].Name);
      else
        fprintf(SqlProcFile,
              Table->Fields[i].Type == ftypeDate
                ? SPUpdate3t
                : Table->Fields[i].Type == ftypeDateTime
                ? SPUpdate3dt
                : Table->Fields[i].Type == ftypeMoney
                ? SPUpdate3m
                : SPUpdate3,
              first==0?' ':',',
              Table->Fields[i].Name,
              Table->UnderScore ? "_" : "",
              Table->Fields[i].Name);
      first=1;
      }
  for (i=0,first=0; i<Table->noFields; i++)
    if (Table->Fields[i].isPrimaryKey)
      {
      fprintf(SqlProcFile,
              Table->Fields[i].Type == ftypeDate
                ? SPUpdate4t
                : Table->Fields[i].Type == ftypeDateTime
                ? SPUpdate4dt
                : Table->Fields[i].Type == ftypeMoney
                ? SPUpdate4m
                : SPUpdate4,
              first==0?"where":"and  ",
              Table->Fields[i].Name,
              Table->UnderScore ? "_" : "",
              Table->Fields[i].Name);
      first=1;
      }
  fprintf(SqlProcFile, SPUpdate5);
  GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, "Update", Table->UnderScore ? "_" : "");
  for (i=0; i<Table->noUpdateUsers; i++)
    ExecuteGrant(Table, "Update", Table->InsertUsers[i].p);
  for (i=0; i<Table->noExecuteUsers; i++)
    ExecuteGrant(Table, "Update", Table->ExecuteUsers[i].p);
}

static void GenerateUsrProcs(PYYTable Table)
{
  ushort i,j;
  for (i=0; i<Table->noProcs; i++)
  {
    if (Table->Procs[i].isData)
    {
      GenerateDataLines(&Table->Procs[i]);
      continue;
    }
    else
      GenerateUsrProc(Table, &Table->Procs[i]);
    if (!Table->Procs[i].isSql)
    {
      for (j=0; j<Table->Procs[i].noUsers; j++)
        ExecuteGrant(Table, Table->Procs[i].Name, Table->Procs[i].Users[j].p);
      for (j=0; j<Table->noExecuteUsers; j++)
        ExecuteGrant(Table, Table->Procs[i].Name, Table->ExecuteUsers[j].p);
    }
  }
}

static void GenerateUsrProc(PYYTable Table, PYYProc Proc)
{
  ushort i,j;
  if (Proc->isSql)
    return;
  fprintf(SqlProcFile, Proc->noFields ? SPUsrProc0a : SPUsrProc0b,
          ProcSuffix(Table),
          Table->UnderScore ? "_" : "",
          Proc->Name);
  for (i=0; i<Proc->noFields; i++)
    fprintf(SqlProcFile, SPUsrProc1,
            i==0?' ':',',
            Table->UnderScore ? "_" : "",
            Proc->Fields[i].Name,
            Proc->Fields[i].isInput?"IN":"",
            Proc->Fields[i].isOutput?"OUT":"",
            ParmFieldType(&Proc->Fields[i])
            );
  if (Proc->noFields)
    fprintf(SqlProcFile, SPUsrProc2);
  for (i=0; i<Proc->noWorkFields; i++)
    fprintf(SqlProcFile, SPUsrProc3,
            Proc->WorkFields[i].Name,
            SqlFieldType(&Proc->WorkFields[i])
            );
  for (i=0; i<Proc->noCursors;i++)
  {
    PYYCursor Cursor = &Proc->Cursors[i];
    for (j=0; j<Cursor->noLines; j++)
    {
      fprintf(SqlProcFile, j==0?SPUsrProc3a:SPUsrProc3b,
              Cursor->Lines[j].p,
              (j+1)==Cursor->noLines?";":""
              );
    }
  }
  fprintf(SqlProcFile, SPUsrProc4);
  for (i=0; i<Proc->noLines; i++)
  {
    fprintf(SqlProcFile, SPUsrProc5, Proc->Lines[i].p);
    //if (Proc->Lines[i].isEOL)
    //  fprintf(SqlProcFile, "\n");
  }
  fprintf(SqlProcFile, SPUsrProc6);
  GenerateSyn2(SqlProcFile, Table->Name, ProcSuffix(Table), Table->UserID, Proc->Name, Table->UnderScore ? "_" : "");
}

static void GenerateDataLines(PYYProc Proc)
{
  ushort i;
  for (i=0; i<Proc->noLines; i++)
    fprintf(SqlProcFile, SPDataLine0, Proc->Lines[i].p);
}

static void GenerateViews(PYYTable Table)
{
  ushort i;
  if (Table->noViews == 0)
    return;
  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlViewsFileName);
    SqlViewsFile = OpenFile("SQL File(Vws)", Table->SqlViewsFileName);
    if (SqlViewsFile == 0)
       return;
    GenerateConnect(Table, SqlViewsFile);
  }
  else
    SqlViewsFile = SqlFile;
  for (i=0; i<Table->noViews; i++)
  {
    GenerateView(Table, &Table->Views[i]);
  }
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlViewsFile);
    fclose(SqlViewsFile);
  }
}

static char *CutUser(const char *aConnect)
{
  static char result[64];
  int i;
  for (i=0; i<64 && aConnect[i] && aConnect[i] != '/'; i++)
    result[i] = aConnect[i];
  result[i] = 0;
  return result;
}

static void GenerateView(PYYTable Table, PYYView View)
{
  ushort i;
  fprintf(SqlViewsFile, CrViews0,
                        strlen(Table->ControlDB) ? Table->ControlDB : "",
                        strlen(Table->ControlDB) ? "." : "",
                        View->Name);
  for (i = 0; i < View->noLines; i++)
    fprintf(SqlViewsFile, CrViews1, View->Lines[i].p);
  fprintf(SqlViewsFile, CrViews9);
  for (i = 0; i < View->noUsers; i++)
    fprintf(SqlViewsFile, CrGrant2,
                          strlen(Table->ControlDB) ? Table->ControlDB : "",
                          strlen(Table->ControlDB) ? "." : "",
                          View->Name, View->Users[i].p);
  for (i=0; i<Table->noSelectUsers; i++)
    fprintf(SqlViewsFile, CrGrant2,
                          strlen(Table->ControlDB) ? Table->ControlDB : "",
                          strlen(Table->ControlDB) ? "." : "",
                          View->Name, Table->SelectUsers[i].p);
  if (strlen(Table->ControlDB))
    GenerateSyn3(SqlViewsFile, View->Name, Table->ControlDB,
                    strlen(Table->UConnect) ? CutUser(Table->UConnect) : Table->UserID);
}

static void GenerateAudit(PYYTable Table)
{
  ushort i;
  PYYField Field;

  if (Table->noFields == 0) return;
  SqlAuditFile = OpenFile("SQL File(Tab)", Table->SqlAuditFileName);
  if (SqlAuditFile == 0)
    return;
  GenerateConnect(Table, SqlAuditFile);
  fprintf(SqlAuditFile, "SET ECHO ON\n\n");
  fprintf(SqlAuditFile, "SET SHOW ON\n\n");
  fprintf(SqlAuditFile, "SPOOL %8.8s.LOG\n\n", Table->Name);
  fprintf(SqlAuditFile, "REMARK AUDIT TABLE\n\n");
  fprintf(SqlAuditFile, DropAudit1, ControlName);
  fprintf(SqlAuditFile, CrAudit0, ControlName);
  for (i=0; i<Table->noFields; i++)
  {
    Field = &Table->Fields[i];
    fprintf(SqlAuditFile, CrAudit1, Field->Name, SqlFieldType(Field));
  }
  fprintf(SqlAuditFile, CrAudit0a);
  fprintf(SqlAuditFile, CrAudit2);
  fprintf(SqlAuditFile, CrAudit3, ControlName);
  fprintf(SqlAuditFile, CrAudit4, ControlName);
  fprintf(SqlAuditFile, CrAudit5, ControlName);
  fprintf(SqlAuditFile, CrAudit6, ControlName);
  if (Table->hasAuditFull)
    fprintf(SqlAuditFile, CrAudit7F, ControlName);
  else
    fprintf(SqlAuditFile, CrAudit7, ControlName);
  fprintf(SqlAuditFile, CrAudit8O, ControlName);
  fprintf(SqlAuditFile, CrAudit9O, ControlName);
  for (i=0; i<Table->noFields; i++)
  {
    Field = &Table->Fields[i];
    fprintf(SqlAuditFile, CrAudit10O, Field->Name);
  }
  fprintf(SqlAuditFile, CrAudit9Oa);
  fprintf(SqlAuditFile, CrAudit10O, "RowId");
  fprintf(SqlAuditFile, CrAudit11);
  if (Table->hasAuditFull)
  {
    fprintf(SqlAuditFile, CrAudit8N, ControlName);
    fprintf(SqlAuditFile, CrAudit9N, ControlName);
    for (i=0; i<Table->noFields; i++)
    {
      Field = &Table->Fields[i];
      fprintf(SqlAuditFile, CrAudit10N, Field->Name);
    }
    fprintf(SqlAuditFile, CrAudit9Na);
    fprintf(SqlAuditFile, CrAudit10N, "RowId");
    fprintf(SqlAuditFile, CrAudit11);
  }
  fprintf(SqlAuditFile, CrAudit12);
  if (strlen(Table->ControlDB))
    GenerateSyn5(SqlAuditFile, Table->Name, ControlName,
       strlen(Table->UConnect) ? CutUser(Table->UConnect) : Table->UserID);
  fprintf(SqlAuditFile, "SPOOL OFF\n\n");
  GenerateExit(Table, SqlAuditFile);
  fclose(SqlAuditFile);
}

static void GenerateTable(PYYTable Table)
{
  char comma;
  ushort i;
  PYYField Field;
  if (Table->noFields == 0) return;
  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlTableFileName);
    SqlTableFile = OpenFile("SQL File(Tab)", Table->SqlTableFileName);
    if (SqlTableFile == 0)
       return;
    GenerateConnect(Table, SqlTableFile);
  }
  else
    SqlTableFile = SqlFile;
  fprintf(SqlTableFile, "REMARK TABLE\n\n");
  fprintf(SqlTableFile, DropTable1, ControlName);
  fprintf(SqlTableFile, DropTable2);
  fprintf(SqlTableFile, DropTable3);
  fprintf(SqlTableFile, CrTable0, ControlName);
  int TableSize = 0;
  for (i=0, comma=' '; i<Table->noFields; i++, comma = ',')
  {
    Field = &Table->Fields[i];
    TableSize += ApproxFieldSize(Field);
    fprintf(SqlTableFile, CrTable1a, comma, Field->Name,
            SqlFieldType(Field));
  }
  fprintf(SqlTableFile, CrTable2, TableSize);
  if (Table->Free)
   fprintf(SqlTableFile, PctFree, Table->Free);
  if (Table->Used)
    fprintf(SqlTableFile, PctUsed, Table->Used);
  if (Table->IniTrans)
    fprintf(SqlTableFile, IniTrans, Table->IniTrans);
  if (Table->MaxTrans)
    fprintf(SqlTableFile, MaxTrans, Table->MaxTrans);
  if (Table->Space)
    fprintf(SqlTableFile, TableSpace, Table->Space);
  if (Table->Initial   ||  Table->StorageNext ||  Table->MinExtents
  || Table->MaxExtents ||  Table->Optimal     ||  Table->FreeLists
  || Table->FreeListGroups)
  {
    fprintf(SqlTableFile, Storage0);
    if (Table->Initial)
      fprintf(SqlTableFile, Initial, Table->Initial, Table->InitialSize);
    if (Table->StorageNext)
      fprintf(SqlTableFile, Next, Table->StorageNext, Table->NextSize);
    if (Table->MinExtents)
      fprintf(SqlTableFile, MinExtents, Table->MinExtents);
    if (Table->MaxExtents)
      fprintf(SqlTableFile, MaxExtents, Table->MaxExtents);
    if (Table->Increase)
      fprintf(SqlTableFile, PctIncrease, Table->Increase);
    if (Table->Optimal)
      fprintf(SqlTableFile, Initial, Table->Optimal, Table->OptimalSize);
    if (Table->FreeLists)
      fprintf(SqlTableFile, FreeLists, Table->FreeLists);
    if (Table->FreeListGroups)
      fprintf(SqlTableFile, FreeListGroups, Table->FreeListGroups);
    fprintf(SqlTableFile, Storage9);
  }
  fprintf(SqlTableFile, CrTable9);
  GenerateSequence(Table);
  if (strlen(Table->ControlDB))
    GenerateSyn1(SqlTableFile, Table->Name, ControlName,
                 strlen(Table->UConnect) ? CutUser(Table->UConnect) : Table->UserID);
  fprintf(SqlTableFile, "REMARK CONTRAINTS\n\n");
  GenerateNulls(Table);
  GenerateKeys(Table);
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlTableFile);
    fclose(SqlTableFile);
  }
}

static void GenerateKeys(PYYTable Table)
{
  ushort i,PkNo=1,UkNo=1;

  for (i = 0; i < Table->noKeys; i++)
    GenerateKey(Table, &Table->Keys[i], &PkNo, &UkNo);
}

static void GenerateKey(PYYTable Table, PYYKey Key, ushort *PkNo, ushort *UkNo)
{
  ushort i;
  char comma;

  if (!Key->isPrimary && !Key->isUnique)
    return;
  if (Key->isPrimary)
    fprintf(SqlTableFile, "REMARK PRIMARY KEY\n\n");
  else if (Key->isUnique)
    fprintf(SqlTableFile, "REMARK UNIQUE KEY\n\n");
  fprintf(SqlTableFile, AlTable0, ControlName);
  if (Key->isPrimary)
    fprintf(SqlTableFile, CrKey0PK, Table->Name, (*PkNo)++);
  else if (Key->isUnique)
    fprintf(SqlTableFile, CrKey0UN, Table->Name, (*UkNo)++);
  for (i = 0, comma=' '; i < Key->noFields; i++, comma = ',')
    fprintf(SqlTableFile, CrKey1, comma, Key->Fields[i].Name);
  fprintf(SqlTableFile, CrKey9);
  if (Key->Space      || Key->Free        || Key->IniTrans   || Key->MaxTrans
  ||  Key->Initial    || Key->StorageNext || Key->Increase   || Key->MinExtents
  ||  Key->MaxExtents || Key->Optimal     || Key->FreeLists
  ||  Key->FreeListGroups)
  {
    fprintf(SqlTableFile, UsingIndex);
    if (Key->Free)
      fprintf(SqlTableFile, PctFree, Key->Free);
    if (Key->IniTrans)
      fprintf(SqlTableFile, IniTrans, Key->IniTrans);
    if (Key->MaxTrans)
      fprintf(SqlTableFile, MaxTrans, Key->MaxTrans);
    if (Key->Space)
      fprintf(SqlTableFile, TableSpace, Key->Space);
    if (Key->Initial   ||  Key->StorageNext ||  Key->MinExtents
    || Key->MaxExtents ||  Key->Optimal     ||  Key->FreeLists
    || Key->FreeListGroups)
    {
      fprintf(SqlTableFile, Storage0);
      if (Key->Initial)
        fprintf(SqlTableFile, Initial, Key->Initial, Key->InitialSize);
      if (Key->StorageNext)
        fprintf(SqlTableFile, Next, Key->StorageNext, Key->NextSize);
      if (Key->MinExtents)
        fprintf(SqlTableFile, MinExtents, Key->MinExtents);
      if (Key->MaxExtents)
        fprintf(SqlTableFile, MaxExtents, Key->MaxExtents);
      if (Key->Increase)
        fprintf(SqlTableFile, PctIncrease, Key->Increase);
      if (Key->Optimal)
        fprintf(SqlTableFile, Initial, Key->Optimal, Key->OptimalSize);
      if (Key->FreeLists)
        fprintf(SqlTableFile, FreeLists, Key->FreeLists);
      if (Key->FreeListGroups)
        fprintf(SqlTableFile, FreeListGroups, Key->FreeListGroups);
      fprintf(SqlTableFile, Storage9);
    }
  }
  fprintf(SqlTableFile, AlTable9);
}

static void GenerateLinks(PYYTable Table)
{
  ushort i;

  for (i = 0; i < Table->noLinks; i++)
    GenerateLink(Table, &Table->Links[i], i);
}

static void GenerateLink(PYYTable Table, PYYLink Link, short count)
{
  ushort i;
  char comma;

  fprintf(SqlTableFile, "REMARK FOREIGN KEY\n\n");
  fprintf(SqlConFile, AlTable0, ControlName);
  fprintf(SqlConFile, CrFKey0, Table->Name, ++count);
  for (i = 0, comma=' '; i < Link->noFields; i++, comma = ',')
    fprintf(SqlConFile, CrFKey1, comma, Link->Fields[i].Name);
  fprintf(SqlConFile, CrFKey9, Link->MasterName);
  if (Link->DeleteCascade)
    fprintf(SqlConFile, CrFKey10);
  fprintf(SqlConFile, AlTable9);
}

static void GenerateIndexes(PYYTable Table)
{
  ushort i;
  short IxNo=1;

  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlIdxFileName);
    SqlIdxFile = OpenFile("SQL File(Idx)", Table->SqlIdxFileName);
    if (SqlIdxFile == 0)
      return;
    GenerateConnect(Table, SqlFile);
  }
  else SqlIdxFile = SqlFile;
  for (i = 0; i < Table->noKeys; i++)
    GenerateIndex(Table, &Table->Keys[i], &IxNo);
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlFile);
    fclose(SqlIdxFile);
  }
}

static void GenerateIndex(PYYTable Table, PYYKey Key, pshort IxNo)
{
  ushort i;
  char comma;

  if ((Key->isPrimary || Key->isUnique))
    return;
  fprintf(SqlTableFile, "REMARK INDEX\n\n");
  fprintf(SqlIdxFile, CrIndex0a,
                      (Key->isPrimary || Key->isUnique) ? "UNIQUE" : "",
                      Table->Name, (*IxNo)++, Table->Name);
  for (i = 0, comma=' '; i < Key->noFields; i++, comma = ',')
    fprintf(SqlIdxFile, CrIndex1, comma, Key->Fields[i].Name);
  fprintf(SqlIdxFile, CrIndex2);
  if (Key->Free)
    fprintf(SqlIdxFile, PctFree, Key->Free);
  if (Key->IniTrans)
    fprintf(SqlIdxFile, IniTrans, Key->IniTrans);
  if (Key->MaxTrans)
    fprintf(SqlIdxFile, MaxTrans, Key->MaxTrans);
  if (Key->Space)
    fprintf(SqlIdxFile, TableSpace, Key->Space);
  if (Key->Initial   ||  Key->StorageNext ||  Key->MinExtents
  || Key->MaxExtents ||  Key->Optimal     ||  Key->FreeLists
  || Key->FreeListGroups)
  {
    fprintf(SqlIdxFile, Storage0);
    if (Key->Initial)
      fprintf(SqlIdxFile, Initial, Key->Initial, Key->InitialSize);
    if (Key->StorageNext)
      fprintf(SqlIdxFile, Next, Key->StorageNext, Key->NextSize);
    if (Key->MinExtents)
      fprintf(SqlIdxFile, MinExtents, Key->MinExtents);
    if (Key->MaxExtents)
      fprintf(SqlIdxFile, MaxExtents, Key->MaxExtents);
    if (Key->Increase)
      fprintf(SqlIdxFile, PctIncrease, Key->Increase);
    if (Key->Optimal)
      fprintf(SqlIdxFile, Initial, Key->Optimal, Key->OptimalSize);
    if (Key->FreeLists)
      fprintf(SqlIdxFile, FreeLists, Key->FreeLists);
    if (Key->FreeListGroups)
      fprintf(SqlIdxFile, FreeListGroups, Key->FreeListGroups);
    fprintf(SqlIdxFile, Storage9);
  }
  fprintf(SqlIdxFile, CrIndex9);
}

static void GenerateSequence(PYYTable Table)
{
  if (Table->hasSequence)
  {
    fprintf(SqlTableFile, "REMARK SEQUENCE\n\n");
    if (strlen(Table->ControlDB))
    {
      fprintf(SqlTableFile, DropSeq, ControlName);
      fprintf(SqlTableFile, CrSeq,   ControlName);
      GenerateSyn4(SqlTableFile, Table->Name, ControlName,
                   strlen(Table->UConnect) ? CutUser(Table->UConnect) : Table->UserID);
    }
    else
    {
      fprintf(SqlTableFile, DropSeq, Table->Name);
      fprintf(SqlTableFile, CrSeq,   Table->Name);
    }
  }
}

static void GenerateGrants(PYYTable Table)
{
  ushort i;

  if (Table->OneSQLScript == 0)
  {
    fprintf(SqlFile, "\n@%s\n", Table->SqlGrantFileName);
    SqlGrantFile = OpenFile("SQL File(Gra)", Table->SqlGrantFileName);
    if (SqlGrantFile == 0)
      return;
    GenerateConnect(Table, SqlGrantFile);
  }
  else
    SqlGrantFile = SqlFile;
  for (i=0; i < Table->noGrants; i++)
    GenerateGrant(Table, &Table->Grants[i]);
  if (Table->OneSQLScript == 0)
  {
    GenerateExit(Table, SqlGrantFile);
    fclose(SqlGrantFile);
  }
}

static void GenerateGrant(PYYTable Table, PYYGrant Grant)
{
  ushort i,j;
  PYYLine Perm, User;

  for (i=0; i < Grant->noUsers; i++)
  {
    User = &Grant->Users[i];
    if (Table->hasSequence)
      fprintf(SqlGrantFile, CrGrant0a,
                            strlen(Table->ControlDB) ? Table->ControlDB : Table->UserID,
                            Table->Name,
                            User->p);
    for (j=0; j < Grant->noPerms; j++)
    {
      Perm = &Grant->Perms[j];
      fprintf(SqlGrantFile, CrGrant0,
                            Perm->p,
                            strlen(Table->ControlDB) ? Table->ControlDB : Table->UserID,
                            Table->Name,
                            User->p);
    }
  }
}

static ushort sqldefTailMark  = 0xBEEF;

static void GenerateTailField(PYYField Field)
{
  char work[1];work[0]=0;
  putString(Field->Name, BinFile);
  if (Field->Alias)
    putString(Field->Alias, BinFile);
  else
    putString(work, BinFile);
  putUInt16(Field->Type, BinFile);
  putUInt16(Field->Length, BinFile);
  putUInt16(Field->Offset, BinFile);
  putUInt16(Field->Padding, BinFile);
  putUInt16(Field->Filler, BinFile);
  putUInt16(Field->Precision, BinFile);
  putUInt16(Field->Scale, BinFile);
  putUInt16(Field->isPrimaryKey, BinFile);
  putUInt16(Field->isNull, BinFile);
  putUInt16(Field->isInput, BinFile);
  putUInt16(Field->isBound, BinFile);
  putUInt16(Field->isOutput, BinFile);
  putUInt16(Field->isWork, BinFile);
  putUInt16(Field->isRowSequence, BinFile);
  putUInt16(Field->isSequence, BinFile);
}

static void GenerateTailTable(PYYTable Table)
{
  ushort i;
  putUInt16(Table->noFields, BinFile);
  for (i=0; i<Table->noFields; i++)
    GenerateTailField(&Table->Fields[i]);
}

static void GenerateTailKey(PYYKey Key)
{
  ushort i;
  putString(Key->Name, BinFile);
  putUInt16(Key->isPrimary, BinFile);
  putUInt16(Key->isUnique, BinFile);
  putUInt16(Key->noFields, BinFile);
  for (i=0; i<Key->noFields; i++)
    GenerateTailField(&Key->Fields[i]);
}

static void GenerateTailKeys(PYYTable Table)
{
  ushort i;
  putUInt16(Table->hasPrimaryKey, BinFile);
  putUInt16(Table->PrimaryKey, BinFile);
  putUInt16(Table->noKeys, BinFile);
  for (i=0; i<Table->noKeys; i++)
    GenerateTailKey(&Table->Keys[i]);
}

char *TableToken(PYYTable Table)
{
  static char work[32];
  int i = 0, n;
  ulong l = Table->Token;
  while (l > 0 && i < 20)
  {
    n = l % 26;
    l = l / 26;
    work[i++] = (char)('A'+n);
    work[i] = 0;
  }
  return work;
}

void GenerateBinTail(PYYTable Table)
{
//  char buf[512];
  char *p=FNameName(Table->InputFileName);
  putString(p, BinFile);
  free(p);
  putString(TableToken(Table), BinFile);
  putUInt16(sqldefTailMark, BinFile);
  GenerateTailTable(Table);
  GenerateTailKeys(Table);
}

static void MakeBinaryName(PYYTable Table)
{
  char newpath[512];
  char dir[512];
  char ext[512];
  char fname[512];
  FNameSplit(Table->InputFileName, dir, fname, ext);
  if (Table->ShortName == 0)
    free(Table->ShortName);
  Table->ShortName = strdup(fname);
  if (strlen(Table->SODir))
    FNameMerge(newpath, Table->SODir, fname, Table->SOExt);
  else
    FNameMerge(newpath, dir, fname, Table->SOExt);
  Table->BinFileName = strdup(newpath);
}

static void GenerateBinary(PYYTable Table)
{
  MakeBinaryName(Table);
  BinFile = OpenFile("Bin File", Table->BinFileName, "wb");
  if (BinFile == 0)
    return;
  GenerateBinTable(Table);
  fclose(BinFile);
}

static int _HasProject(int noProjects, PYYLine Projects, PYYTable Table)
{
  if (Table->UseUsedBy == 0)
    return 1;
  if (noProjects == 0)
    return 1;
  char *Name = Table->Name;
  int i;
  for (i=0; i < noProjects; i++)
  {
    PYYLine Project = &Projects[i];
    if (stricmp(Project->p, Name) == 0)
      return 1;
    if (stricmp(Project->p, "ALL") == 0)
      return 1;
  }
  return 0;
}

static ushort _CountProcs(PYYTable Table)
{
  if (Table->UseUsedBy == 0)
    return (ushort)(Table->noProcs - Table->noDatas);
  ushort result = 0;
  int i;
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    result += _HasProject(Proc->noProjects, Proc->Projects, Table);
  }
  return result;
}

static void GenerateBinTable(PYYTable Table)
{
  ushort Count = _CountProcs(Table);
  if (Table->Internal == 0)
  {
    if (Table->hasInsert)
      Count += _HasProject(Table->noInsertProjects, Table->InsertProjects, Table);
    if (Table->hasSelectAll)
      Count += _HasProject(Table->noSelectAllProjects, Table->SelectAllProjects, Table);
    if (Table->hasSelectAllUpd)
      Count += _HasProject(Table->noSelectAllUpdProjects, Table->SelectAllUpdProjects, Table);
    if (Table->hasDeleteAll)
      Count += _HasProject(Table->noDeleteAllProjects, Table->DeleteAllProjects, Table);
    if (Table->hasCount)
      Count += _HasProject(Table->noCountProjects, Table->CountProjects, Table);
    if (Table->hasSelectOne)
      Count += _HasProject(Table->noSelectOneProjects, Table->SelectOneProjects, Table);
    if (Table->hasSelectOneUpd)
      Count += _HasProject(Table->noSelectOneUpdProjects, Table->SelectOneUpdProjects, Table);
    if (Table->hasDeleteOne)
      Count += _HasProject(Table->noDeleteOneProjects, Table->DeleteOneProjects, Table);
    if (Table->hasUpdate)
      Count += _HasProject(Table->noUpdateProjects, Table->UpdateProjects, Table);
  }
  putUInt16(sqldefSignOCI2, BinFile);
  putString(Table->Server, BinFile);
  putString(Table->UserID, BinFile);
  putString(Table->Name, BinFile);
  putUInt16(Count, BinFile);
  GenerateBinProcs(Table);
  GenerateBinTail(Table);
}

static void GenerateBinProcs(PYYTable Table)
{
  ushort i;
  if (Table->hasCount && Table->Internal == 0
  && _HasProject(Table->noCountProjects, Table->CountProjects, Table) == 1)
    GenerateBinProc(Table,
                       "Count",
                       0, 0, 0,
                       "BEGIN\n"
                       " Select count(*) into :NoOfRecs from %s;"
                       "END;",
                       &Table->Count, 0);
  if (Table->hasDeleteAll && Table->Internal == 0
  && _HasProject(Table->noDeleteAllProjects, Table->DeleteAllProjects, Table) == 1)
    GenerateBinProc(Table,
                       "DeleteAll",
                       0, 0, 0,
                       "Delete %s",
                       0, 0);
  if (Table->hasInsert && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noInsertProjects, Table->InsertProjects, Table) == 1)
    GenerateBinProc(Table,
                       "Insert",
                       Table->noFields,
                       Table->Fields,
                       Table->ABSize,
                       0, 0, 0);
  if (Table->hasSelectAll && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noSelectAllProjects, Table->SelectAllProjects, Table) == 1)
    GenerateBinSelectAll(Table, false);
  if (Table->hasSelectAllUpd && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noSelectAllUpdProjects, Table->SelectAllUpdProjects, Table) == 1)
    GenerateBinSelectAll(Table, true);
  if (Table->hasSelectOne && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noSelectOneProjects, Table->SelectOneProjects, Table) == 1)
    GenerateBinProc(Table,
                       "SelectOne",
                       Table->noFields,
                       Table->Fields,
                       Table->ABSize,
                       0, 0, 0);
  if (Table->hasSelectOneUpd && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noSelectOneUpdProjects, Table->SelectOneUpdProjects, Table) == 1)
    GenerateBinProc(Table,
                       "SelectOneUpd",
                       Table->noFields,
                       Table->Fields,
                       Table->ABSize,
                       0, 0, 0);
  if (Table->hasDeleteOne && Table->hasPrimaryKey && Table->Internal == 0
  && _HasProject(Table->noDeleteOneProjects, Table->DeleteOneProjects, Table) == 1)
    GenerateBinProc(Table,
                       "DeleteOne",
                       Table->Keys[Table->PrimaryKey].noFields,
                       Table->Keys[Table->PrimaryKey].Fields,
                       Table->Keys[Table->PrimaryKey].ABSize,
                       0, 0, 0);
  if (Table->hasUpdate && Table->noFields && Table->Internal == 0
  && _HasProject(Table->noUpdateProjects, Table->UpdateProjects, Table) == 1)
    GenerateBinProc(Table,
                       "Update",
                       Table->noFields,
                       Table->Fields,
                       Table->ABSize,
                       0, 0, 1);
  for (i=0; i<Table->noProcs; i++)
  {
    PYYProc Proc = &Table->Procs[i];
    if (Proc->isData)
      continue;
    if (_HasProject(Proc->noProjects, Proc->Projects, Table) == 0)
      continue;
    if (Proc->isSql)
      GenerateBinSql (Table, Proc);
    else
      GenerateBinProc(Table,
                      Proc->Name,
                      Proc->noFields,
                      Proc->Fields,
                      Proc->ABSize,
                      0, 0, 0);
  }
}

static void GenerateBinProc(PYYTable Table, const char *ProcName,
                            ushort noFields, PYYField Fields,
                            ushort ABSize, const char *Code,
                            PYYField ExtraField,
                            uchar testSeq)
{
  char *TableName = Table->Name;
  char *AB;
  ushort i,first, no;
  ABSize += (ushort)512;
  AB = (char *) malloc(ABSize);
  strcpy(AB, TableName);
  if (Table->UnderScore)
    strcat(AB, "_");
  strcat(AB, ProcName);
  putString(AB, BinFile);
  if (testSeq)
    for (i=0,no=0; i<noFields; i++)
    {
      if (Fields[i].isSequence && Fields[i].isPrimaryKey == 0)
        continue;
      if (Fields[i].isRowSequence)
        continue;
      no++;
    }
  else
    no = noFields;
  if (ExtraField)
    putUInt16((ushort)(no+1), BinFile);
  else
    putUInt16(no, BinFile);
  for (i=0; i<noFields; i++)
  {
    if (testSeq && Fields[i].isSequence && Fields[i].isPrimaryKey == 0)
      continue;
    if (testSeq && Fields[i].isRowSequence)
      continue;
    sprintf(AB, ":%s", Fields[i].Name);
    putString(AB, BinFile);
    putInt16(SQLCFieldType(Fields[i].Type), BinFile);
    putUInt16(OCIFieldType(Fields[i].Type), BinFile);
    if (Fields[i].Type == ftypeImagePtr)
      putUInt32(0, BinFile);
    else
      putUInt32(Fields[i].Length, BinFile);
    putUInt32(Fields[i].Offset, BinFile);
    putUInt32(Fields[i].Precision, BinFile);
    putUInt32(Fields[i].Scale, BinFile);
    putUInt16(1, BinFile);
    putUInt16(0, BinFile);
  }
  if (ExtraField)
  {
    sprintf(AB, ":%s", ExtraField->Name);
    putString(AB, BinFile);
    putInt16(SQLCFieldType(ExtraField->Type), BinFile);
    putUInt16(OCIFieldType(ExtraField->Type), BinFile);
    putUInt32(ExtraField->Length, BinFile);
    putUInt32(ExtraField->Offset, BinFile);
    putUInt32(ExtraField->Precision, BinFile);
    putUInt32(ExtraField->Scale, BinFile);
    putUInt16(1, BinFile);
    putUInt16(0, BinFile);
  }
  if (noFields)
  {
    sprintf(AB, "BEGIN\n  %s%s%s",
                TableName,
                Table->UnderScore ? "_" : "",
                ProcName);
    if (Code)
      {
      strcat(AB, ".");
      strcat(AB, Code);
      }
    strcat(AB, "\n  (\n");
    if (ExtraField)
    {
      strcat(AB, "    :");
      strcat(AB, ExtraField->Name);
      strcat(AB, "\n");
    }
    for (i=first=0; i<noFields; i++)
    {
      if (testSeq && Fields[i].isSequence && Fields[i].isPrimaryKey == 0)
        continue;
      if (testSeq && Fields[i].isRowSequence)
        continue;
      strcat(AB, (first==0 && !ExtraField)?"    :":"  , :");
      strcat(AB, Fields[i].Name);
      strcat(AB, "\n");
      first=1;
    }
    strcat(AB, "  );\n");
    strcat(AB, "END;");
  }
  else if (Code)
  {
    sprintf(AB, Code, TableName);
  }
  else
  {
    sprintf(AB, "BEGIN %s%s%s; END;",
                TableName,
                Table->UnderScore ? "_" : "",
                ProcName);
  }
  putUInt16(strlen(AB), BinFile);
  putExact(AB, BinFile);
  free(AB);
}

static ushort FieldFlag(TYYField Field)
{
  return (Field.isPrimaryKey   ? 0x8000 : 0)
       | (Field.isNull         ? 0x4000 : 0)
       | (Field.isInput        ? 0x2000 : 0)
       | (Field.isBound        ? 0x1000 : 0)
       | (Field.isOutput       ? 0x0800 : 0)
       | (Field.isWork         ? 0x0400 : 0)
       | (Field.isRowSequence  ? 0x0200 : 0)
       | (Field.isSequence     ? 0x0100 : 0)
       | (NullableField(&Field)? 0x0010 : 0);
}

static void GenerateBinSelectAll(PYYTable Table, bool ForUpd)
{
  char *AB, *pAB;
  ushort i;
  PYYField Fields = Table->Fields;

  AB = (char *)calloc(strlen(Table->Name)+20, 1);
  strcpy(AB, Table->Name);
  if (Table->UnderScore)
    strcat(AB, "_");
  strcat(AB, "SelectAll");
  if (ForUpd)
    strcat(AB, "Upd");
  putString(AB, BinFile);
  free(AB);
  putUInt16(((Table->noFields & 0x0FFF) | 0xA000), BinFile);
  AB = (char *)malloc(4096);
  for (i=0; i<Table->noFields; i++)
  {
    sprintf(AB, ":%s", Fields[i].Name);
    putString(AB, BinFile);
    putInt16(SQLCFieldType(Fields[i].Type), BinFile);
    putUInt16(OCIFieldType(Fields[i].Type), BinFile);
    if (Fields[i].Type == ftypeImagePtr)
      putUInt32(0, BinFile);
    else
      putUInt32(Fields[i].Length, BinFile);
    putUInt32(Fields[i].Offset, BinFile);
    putUInt32(Fields[i].Precision, BinFile);
    putUInt32(Fields[i].Scale, BinFile);
    putUInt16(0, BinFile);
    putUInt16(1|FieldFlag(Fields[i]), BinFile);
  }
  pAB = AB;
  pAB += sprintf(pAB, "select");
  for (i=0; i<Table->noFields; i++)
    pAB += sprintf(pAB,
        Table->Fields[i].Type == ftypeDate
        ? "%s to_char(%s, 'YYYYMMDD')"
        : (Table->Fields[i].Type == ftypeTimeStamp
          || Table->Fields[i].Type == ftypeDateTime)
        ? "%s to_char(%s, 'YYYYMMDDHH24MISS')"
        : Table->Fields[i].Type == ftypeMoney
        ? "%s to_char(%s)"
        : "%s %s",
        i == 0
        ? ""
        : ",",
        Fields[i].Name);
  pAB += sprintf(pAB, " from %s", Table->Name);
  if (ForUpd)
    pAB += sprintf(pAB, " for update");
  putUInt16(strlen(AB), BinFile);
  putExact(AB, BinFile);
  free(AB);
}

static void GenerateBinSql(PYYTable Table, PYYProc Proc)
{
  char *AB;
  ushort i, j, ABSize;
  PYYField Fields = Proc->Fields;

  AB = (char *)calloc(strlen(Table->Name)+strlen(Proc->Name)+1, 1);
  strcpy(AB, Table->Name);
  if (Table->UnderScore)
    strcat(AB, "_");
  strcat(AB, Proc->Name);
  putString(AB, BinFile);
  free(AB);
  putUInt16(( (Proc->noBinds & 0x0FFF)
            | (Proc->isSql   ? 0x8000 : 0)
            | (Proc->isFetch ? 0x4000 : 0)
            | (Proc->isMultiFetch ? 0x2000 : 0)
            | (Table->isNullEnabled ? 0x1000 : 0)), BinFile);
  AB = (char *)malloc(512);
  for (j=0; j<Proc->noBinds; j++)
  {
    i = Proc->Binds[j];
    sprintf(AB, ":%s", Fields[i].Name);
    putString(AB, BinFile);
    putInt16(SQLCFieldType(Fields[i].Type), BinFile);
    putUInt16(OCIFieldType(Fields[i].Type), BinFile);
    if (Fields[i].Type == ftypeImagePtr)
      putUInt32(0, BinFile);
    else
      putUInt32(Fields[i].Length, BinFile);
    putUInt32(Fields[i].Offset, BinFile);
    putUInt32(Fields[i].Precision, BinFile);
    putUInt32(Fields[i].Scale, BinFile);
    if (Fields[i].Type == ftypeDynamic)
    {
      putUInt16(0, BinFile);
      putUInt16(0, BinFile);
    }
    else
    {
      putUInt16(j<Proc->noOutputs?0:1|FieldFlag(Fields[i]), BinFile);
      putUInt16(j<Proc->noOutputs?1|FieldFlag(Fields[i]):0, BinFile);
    }
  }
  free(AB);
  ABSize = 0;
  for (i=0; i<Proc->noLines; i++)
    ABSize += strlen(Proc->Lines[i].p)+2;
  AB = (char *)calloc(ABSize, 1);
  for (i=0; i<Proc->noLines; i++)
  {
    if (i) strcat(AB, " ");
    strcat(AB, strtrim(Proc->Lines[i].p));
  }
  for (i=strlen(AB)-1; AB[i] <= ' ' || AB[i] == ';'; i--)
  {
    AB[i] = 0;
  }
  if ((i > 4)
  && (i < ABSize)
  && (strnicmp(AB+(i-2), "end", 3) == 0)
  && (strchr("abcdefghijklmnopqrstuvwxyz"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "0123456789_#$", AB[i-3]) == 0))
    AB[++i] = ';';
  putUInt16(strlen(AB), BinFile);
  putExact(AB, BinFile);
  free(AB);
}

static const char *SqlFieldType(PYYField Field)
{
  static char work[20];
  switch (Field->Type)
  {
  case ftypeChar:
  case ftypeUserStamp:
  case ftypeStatus:
    sprintf(work, "%s(%hu)",
                  (Table->BlankPadded || Table->CharZ)
                  ? "char"
                  : "varchar2",
                  Field->Length - 1);
    return work;
  case ftypeBinary:
    sprintf(work, "raw(%hu)", Field->Length - 1);
    return work;
  case ftypeDate:
    return "date";
  case ftypeDateTime:
    return "date";
  case ftypeTimeStamp:
    return "date";
  case ftypeBoolean:
    return "number(1)";
  case ftypeTinyint:
    return "number(2)";
  case ftypeSmallint:
    return "number(4)";
  case ftypeInt:
    return "number(9)";
  case ftypeLong:
    return "number(18)";
  case ftypeFloat:
  case ftypeVarNum:
  case ftypeMoney:
    if (Field->Precision)
    {
      sprintf(work, "number(%hu,%hu)", Field->Precision, Field->Scale);
      return work;
    }
    return "number";
  case ftypeBLOB:
  case ftypeZLOB:
  case ftypeCLOB:
  case ftypeImage:
  case ftypeImagePtr:
    return "long raw";
  case ftypeHugeCHAR:
    return "clob";
  case ftypeXMLTYPE:
    return "xmltype";
  case ftypeException:
    return "exception";
  }
  return "junk";
}

static const char *ParmFieldType(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
    return "raw";
  case ftypeChar:
  case ftypeDateTime:
  case ftypeDate:
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeMoney:
    return Table->CharZ ? "char" : "varchar2";
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeInt:
  case ftypeLong:
  case ftypeFloat:
  case ftypeVarNum:
    return "number";
  case ftypeBLOB:
  case ftypeZLOB:
  case ftypeCLOB:
  case ftypeImage:
  case ftypeImagePtr:
    return "long raw";
  case ftypeHugeCHAR:
    return "clob";
  case ftypeXMLTYPE:
    return "xmltype";
  }
  return "junk";
}

static const char *ParmFieldTypeInsert(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
    return "raw";
  case ftypeChar:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeMoney:
    return Table->CharZ ? "char" : "varchar2";
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
    return Table->CharZ ? "char" : "varchar2";
  case ftypeInt:
  case ftypeLong:
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeFloat:
  case ftypeVarNum:
    return "number";
  case ftypeBLOB:
  case ftypeZLOB:
  case ftypeCLOB:
  case ftypeImage:
  case ftypeImagePtr:
    return "long raw";
  case ftypeHugeCHAR:
    return "clob";
  case ftypeXMLTYPE:
    return "xmltype";
  }
  return "junk";
}

static const char *ParmFieldTypeUpdate(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
    return "raw";
  case ftypeChar:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeMoney:
    return Table->CharZ ? "char" : "varchar2";
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
    return Table->CharZ ? "char" : "varchar2";
  case ftypeInt:
  case ftypeLong:
  case ftypeBoolean:
  case ftypeTinyint:
  case ftypeSmallint:
  case ftypeFloat:
  case ftypeVarNum:
    return "number";
  case ftypeBLOB:
  case ftypeZLOB:
  case ftypeCLOB:
  case ftypeImage:
  case ftypeImagePtr:
    return "long raw";
  case ftypeHugeCHAR:
    return "clob";
  case ftypeXMLTYPE:
    return "xmltype";
  }
  return "junk";
}

static ushort OCIFieldType(eFieldType FT)
{
  switch(FT)
  {
  case ftypeTimeStamp:
  case ftypeUserStamp:
  case ftypeStatus:
  case ftypeDate:
  case ftypeDateTime:
  case ftypeMoney:
  case ftypeChar:    return Table->BlankPadded
                         ? SQL_OCI_CHARx
                         : Table->CharZ
                         ? SQL_OCI_CHARz
                         : SQL_OCI_VARCHAR2;
  case ftypeFloat:   return SQL_OCI_FLOAT;
  case ftypeVarNum:  return SQL_OCI_VARNUM;
  case ftypeLong:
  case ftypeInt:
  case ftypeSmallint:
  case ftypeBoolean:
  case ftypeTinyint: return SQL_OCI_NUMBER;
  case ftypeImage:   return SQL_OCI_IMAGE;
  case ftypeImagePtr:return SQL_OCI_IMAGE;
  case ftypeBLOB:    return SQL_OCI_BLIMAGE;
  case ftypeCLOB:    return SQL_OCI_CLIMAGE;
  case ftypeZLOB:    return SQL_OCI_BLIMAGE;
  case ftypeHugeCHAR:return SQL_OCI_HUGECHAR;
  case ftypeXMLTYPE: return SQL_OCI_XMLTYPE;
  case ftypeBinary:  return SQL_OCI_RAW;
  case ftypeDynamic: return '$';
  }
  return 0;
}

static short SQLCFieldType(eFieldType FT)
{
  switch(FT)
  {
  case ftypeChar:      return SQL_C_CHAR;
  case ftypeFloat:     return SQL_C_DOUBLE;
  case ftypeInt:       return SQL_C_LONG;
  case ftypeLong:      return SQL_C_LONG64;
  case ftypeSmallint:  return SQL_C_SHORT;
  case ftypeBoolean:   return SQL_C_TINYINT; //use of SQL_C_BIT under consideration
  case ftypeTinyint:   return SQL_C_TINYINT;
  case ftypeImage:     return SQL_C_BINARY;
  case ftypeImagePtr:  return SQL_C_BINARY;
  case ftypeBLOB:      return SQL_C_BLIMAGE;
  case ftypeCLOB:      return SQL_C_CLIMAGE;
  case ftypeZLOB:      return SQL_C_ZLIMAGE;
  case ftypeHugeCHAR:  return SQL_C_HUGECHAR;
  case ftypeXMLTYPE:   return SQL_C_XMLTYPE;
  case ftypeDate:      return SQL_C_CHAR;
  case ftypeDateTime:  return SQL_C_CHAR;
  case ftypeTimeStamp: return SQL_C_CHAR;
  case ftypeUserStamp: return SQL_C_CHAR;
  case ftypeStatus:    return SQL_C_CHAR;
  case ftypeMoney:     return SQL_C_CHAR;
  case ftypeBinary:    return SQL_C_BINARY;
  case ftypeDynamic:   return '$';
  }
  return 0;
}

static int ApproxFieldSize(PYYField Field)
{
  switch (Field->Type)
  {
  case ftypeBinary:
  case ftypeChar:
  case ftypeUserStamp:
  case ftypeStatus:
    return Field->Length+1;
  case ftypeTimeStamp:
  case ftypeDate:
  case ftypeDateTime:
    return 7;
  case ftypeTinyint:
    return 2;
  case ftypeBoolean:
    return 1;
  case ftypeSmallint:
    return 4;
  case ftypeInt:
    return 9;
  case ftypeLong:
    return 19;
  case ftypeFloat:
  case ftypeMoney:
  case ftypeVarNum:
    if (Field->Precision)
      return Field->Precision+2;
    return 38;
  case ftypeImage:
    return Field->Length-2;
  case ftypeImagePtr:
    return 32000;
  case ftypeBLOB:
  case ftypeCLOB:
  case ftypeXMLTYPE:
    return Field->Precision;
  case ftypeHugeCHAR:
    return 64000;
  case ftypeZLOB:
    return Field->Precision/10;
  }
  return 38;
}


