#ifndef GenOciH
#define GenOciH

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>

#include "sqlcons.h"
#include "sqldef.h"

#include "global.h"
#include "xdir.h"
#include "binio.h"

const ushort sqldefSignOCI  = 0x0C1A;

struct TGen
{
  TYYTable &Table;
  TGen(TYYTable &aTable)
  : Table(aTable)
  {}
  pchar Dehash(pchar Name);
  pchar NameOf(PYYField Field);
  bool hasStdRtns();
};

struct TGenSQL : public TGen
{
  FILE* SqlFile;
  FILE* SqlTableFile;
  FILE* SqlViewsFile;
  FILE* SqlConFile;
  FILE* SqlIdxFile;
  FILE* SqlGrantFile;
  FILE* SqlProcFile;
  FILE* SqlAuditFile;
  pchar UpperName;
  pchar ControlName;
  TGenSQL(TYYTable &aTable, pchar aUpperName, pchar aControlName)
  : TGen(aTable)
  , UpperName(aUpperName)
  , ControlName(aControlName)
  {}
  void Generate();
  void GenSyn1(FILE* file, pchar Name, pchar FullName, pchar User);
  void GenSyn2(FILE* file, pchar Name, pchar FullName, pchar User,
                                       pchar ProcName, pchar Under);
  void GenSyn3(FILE* file, pchar Name, pchar DBName, pchar User);
  void GenSyn4(FILE* file, pchar Name, pchar FullName, pchar User);
  void GenSyn5(FILE* file, pchar Name, pchar FullName, pchar User);
  void GenTable();
  void GenAudit();
  void GenViews();
  void GenView(PYYView View);
  void GenSequence();
  void GenConstraints();
  void GenKeys();
  void GenKey(PYYKey Key, ushort *PkNo, ushort *UkNo);
  void GenNulls();
  void GenLinks();
  void GenLink(PYYLink Link);
  void GenIndexes();
  void GenIndex(PYYKey Key, pshort IxNo);
  void GenGrants();
  void GenGrant(PYYGrant Grant);
  void GenProcs();
  void GenStdProcs();
  void GenInsert();
  void GenSelectOne(bool ForUpd);
  void GenDeleteOne();
  void GenUpdate();
  void GenUsrProcs();
  void GenUsrProc(PYYProc Proc);
  void GenDataLines(PYYProc Proc);
  void ExeGrant(pchar ProcName, pchar User);
  pchar ProcSuffix();
  pchar SqlFieldType(PYYField Field);
  pchar ParmFieldType(PYYField Field);
  pchar ParmFieldTypeInsert(PYYField Field);
  pchar ParmFieldTypeUpdate(PYYField Field);
  pchar CutUser(pchar aConnect);
  int ApproxFieldSize(PYYField Field);
};

struct TGenBin : public TGen
{
  FILE* BinFile;
  TGenBin(TYYTable &aTable)
  : TGen(aTable)
  {}
  void Generate();
  void GenBinTable();
  void GenBinProcs();
  void GenBinSelectAll(bool ForUpd);
  void GenBinProc(pchar TableName, pchar ProcName,
                  ushort noFields, PYYField Fields,
                  ushort ABSize, pchar Code,
                  PYYField ExtraField, uchar  testSeq);
  void GenBinSql(PYYProc Proc);
  void GenBinTail();
  void GenTailTable();
  void GenTailKeys();
  void GenTailKey(PYYKey Key);
  void GenTailField(PYYField Field);
  ushort FieldFlag(TYYField Field);
  ushort OCIFieldType(eFieldType FT);
  short  SQLCFieldType(eFieldType FT);
};

struct TGenIDL : public TGen
{
  FILE* IDLFile;
  TGenIDL(TYYTable &aTable)
  : TGen(aTable)
  {}
  void Generate();
  void GenStructs();
  void GenAction(pchar structName, pchar procName, pchar method);
  void GenReadOne(pchar structName, pchar procName, pchar method);
  void GenReadMany(pchar structName, pchar procName, pchar method, int NoInput=0);
  void GenCode();
  pchar NameExt(char *path);
};

struct TGenVB : public TGen
{
  FILE* VBFile;
  TGenVB(TYYTable &aTable)
  : TGen(aTable)
  {}
  char  CurrClassName[512];
  void  Generate();
  int   OpenFile(pchar FileName);
  void  GenTop(pchar FName);
  void  GenStart();
  void  GenTable();
  void  GenBottom();
  void  GenProc(PYYProc Proc);
  void  GenKey(PYYKey Key);
  void  GenExtRec(pchar ProcName);
  void  GenExtKey(PYYKey Key, pchar ProcName);
  void  GenExtKeyRec(PYYKey Key, pchar ProcName);
  void  GenExtKeyExists(PYYKey Key, pchar ProcName);
  void  GenExtCount(pchar ProcName);
  void  GenExtDeleteAll(pchar ProcName);
  void  GenExt();
  void  GenRec(bool isArrayed = false);
  void  GenIO(bool isArrayed = false);
  void  GenGetLet(pchar GetFT, bool isString, pchar Name, pchar SetFT, bool isArray = false);
  void  GenNoOutputProc(pchar TableName, pchar Proc, bool isArray = false);
  void  GenSingleProc(pchar TableName, pchar Proc, bool isArray = false);
  void  GenMultiProc(pchar TableName, pchar Proc);
  void  GenNoIOProc(pchar TableName, pchar Proc);
  bool  GenCursorVars(pchar Proc);
  pchar MakeFileName(pchar path, pchar funcname);
  pchar FieldType(PYYField Field);
  pchar ParmType(PYYField Field);
  pchar ConstrFieldType(PYYField Field, bool isArray = false);
  pchar GetFieldType(PYYField Field, bool isArray = false);
  pchar SetFieldType(PYYField Field, bool isArray = false);
  bool  FieldIsString(PYYField Field);
  void  Genx8();
  void  Genx4();
};

struct TGenPAS : public TGen
{
  FILE* PASFile;
  TGenPAS(TYYTable &aTable)
  : TGen(aTable)
  {}
  void Generate();
};

struct TGenC : public TGen
{
  FILE* CFile;
  TGenC(TYYTable &aTable)
  : TGen(aTable)
  {}
  void Generate();
};

#endif
