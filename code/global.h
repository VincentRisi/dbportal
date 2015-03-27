#ifndef globalH
#define globalH

#include <stdio.h>
#ifndef sqldefALL
#include "sqldef.h"
#endif
#include "xtalloc.h"
#include "yyerror.h"

enum eState
{
  stateTableGrant
, stateProcGrant
, stateViewGrant
, stateCountProcGrant
, stateDeleteAllProcGrant
, stateDeleteOneProcGrant
, stateInsertProcGrant
, stateSelectAllProcGrant
, stateSelectOneProcGrant
, stateSelectAllUpdProcGrant
, stateSelectOneUpdProcGrant
, stateSelectPlusProcGrant
, stateUpdateProcGrant
, stateExistsProcGrant
, stateProcProject
, stateCountProcProject
, stateDeleteAllProcProject
, stateDeleteOneProcProject
, stateInsertProcProject
, stateSelectAllProcProject
, stateSelectOneProcProject
, stateSelectAllUpdProcProject
, stateSelectOneUpdProcProject
, stateSelectPlusProcProject
, stateUpdateProcProject
, stateExistsProcProject
, stateTable
, stateInput
, stateInOut
, stateOutput
, stateWork
, stateCode
, stateCursor
, stateData
, stateView
, stateLink
, stateKey
};

enum eFieldType
{ ftypeStatus    = 'A'
, ftypeBLOB      = 'b'
, ftypeImage     = 'B'
, ftypeCLOB      = 'c'
, ftypeChar      = 'C'
, ftypeDate      = 'D'
, ftypeMoney     = 'E'
, ftypeFloat     = 'F'
, ftypeHugeCHAR  = 'H'
, ftypeInt       = 'I'
, ftypeLong      = 'L'
, ftypeTimeStamp = 'M'
, ftypeBoolean   = 'O'
, ftypeImagePtr  = 'P'
, ftypeBinary    = 'R'
, ftypeSmallint  = 'S'
, ftypeTinyint   = 'T'
, ftypeUserStamp = 'U'
, ftypeVarNum    = 'V'
, ftypeDateTime  = 'X'
, ftypeXMLTYPE   = 'x'
, ftypeException = 'Z'
, ftypeZLOB      = 'z'
, ftypeDynamic   = '$'
};

enum {
//--------
       noC         = 0x00000001
     , noPAS       = 0x00000002
     , noVB        = 0x00000004
     , noIDL       = 0x00000008
//--------
     , noEXTC      = 0x00000010
     , noEXTPAS    = 0x00000020
     , noEXTVB     = 0x00000040
     , noEXT       = 0x00000070
//--------
     , doC         = 0x00000100
     , doPAS       = 0x00000200
     , doVB        = 0x00000400
     , doIDL       = 0x00000800
//--------
     , doEXTC      = 0x00001000
     , doEXTPAS    = 0x00002000
     , doEXTVB     = 0x00004000
//--------
     , doPYTHON    = 0x00010000
     , doVB5       = 0x00020000
     , doVBCODE3   = 0x00040000
     , doVBFORIDL  = 0x00080000
//--------
     , doVBFORADOR = 0x00100000
     , doVBNET7    = 0x00200000
     , doCSADONET  = 0x00200000 // Overload VBNET7
     , doCSNET7    = 0x00400000
     //, doCSDAP     = 0x00400000 // Overload CSNET7
     , doCSIDL2    = 0x00800000
     , doCSRW      = 0x00800000 // Overload CSIDL2
//--------
     , noPYTHON    = 0x01000000
     , noVB5       = 0x02000000
     , noVBCODE3   = 0x04000000
     , noVBFORIDL  = 0x08000000
//--------
     , noVBFORADOR = 0x10000000
     , noVBNET7    = 0x20000000
     , noCSADONET  = 0x20000000 // Overload VBNET7
     , noCSNET7    = 0x40000000
     //, noCSDAP     = 0x40000000 // Overload CSNET7
     , noCSIDL2    = 0x80000000
     , noCSRW      = 0x80000000 // Overload CSIDL2
     };

struct TKeywordList
{
  const char *name;
  int token_value;
};

struct TYYConst;
struct TYYField;
struct TYYTable;
struct TYYGrant;
struct TYYKey;
struct TYYView;
struct TYYProc;
struct TYYCursor;
struct TYYLine;
struct TYYLink;

typedef struct TYYConst  *PYYConst;
typedef struct TYYField  *PYYField;
typedef struct TYYTable  *PYYTable;
typedef struct TYYGrant  *PYYGrant;
typedef struct TYYKey    *PYYKey;
typedef struct TYYView   *PYYView;
typedef struct TYYProc   *PYYProc;
typedef struct TYYCursor *PYYCursor;
typedef struct TYYLine   *PYYLine;
typedef struct TYYLink   *PYYLink;

struct TYYConst
{
  pchar      Name;
  int        Value;
  int        Type;  // 0=number const 1=character const
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYField
{
  pchar      Name;
  pchar      Alias;
  pchar      Comment;
  pchar      Check;
  eFieldType Type;
  int32      Length;
  int32      Offset;
  int32      Padding;
  int32      Filler;
  int32      Precision;
  int32      Scale;
  int32      noConsts;
  PYYConst   Consts;
  uchar      isPrimaryKey;
  uchar      isNull;
  uchar      isInput;
  uchar      isBound;
  uchar      isOutput;
  uchar      isWork;         // 0 not work, 1 work, 2 work exception field
  uchar      isRowSequence;
  uchar      isSequence;
  uchar      isUpper;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
  TYYField() {Clear();}
  TYYField(TYYField &AField)
  {
    *this = AField;
    noConsts = 0;
    Consts = 0;
  }
};

struct TYYTable
{
  pchar    Server;
  pchar    UserID;
  pchar    PassWD;
  pchar    Name;
  pchar    Alias;
  pchar    Check;
  int32    Token;
  int32    noBuffers;
  int32    Length;
  int32    ABSize;
  int32    noFields;
  PYYField Fields;
  int32    noGrants;
  PYYGrant Grants;
  int32    noKeys;
  PYYKey   Keys;
  uchar    hasPrimaryKey;
  int32    PrimaryKey;
  int32    noLinks;
  PYYLink  Links;
  uchar    hasInsert;
  int32    extInsert;
  int32    noInsertUsers;
  PYYLine  InsertUsers;
  int32    noInsertProjects;
  PYYLine  InsertProjects;
  uchar    hasSelectAll;
  int32    extSelectAll;
  int32    noSelectAllUsers;
  PYYLine  SelectAllUsers;
  int32    noSelectAllProjects;
  PYYLine  SelectAllProjects;
  uchar    hasSelectPlus;
  int32    extSelectPlus;
  int32    noSelectPlusUsers;
  PYYLine  SelectPlusUsers;
  int32    noSelectPlusProjects;
  PYYLine  SelectPlusProjects;
  uchar    hasSelectOne;
  int32    extSelectOne;
  int32    noSelectOneUsers;
  PYYLine  SelectOneUsers;
  int32    noSelectOneProjects;
  PYYLine  SelectOneProjects;
  uchar    hasSelectAllUpd;
  int32    extSelectAllUpd;
  int32    noSelectAllUpdUsers;
  PYYLine  SelectAllUpdUsers;
  int32    noSelectAllUpdProjects;
  PYYLine  SelectAllUpdProjects;
  uchar    hasSelectOneUpd;
  int32    extSelectOneUpd;
  int32    noSelectOneUpdUsers;
  PYYLine  SelectOneUpdUsers;
  int32    noSelectOneUpdProjects;
  PYYLine  SelectOneUpdProjects;
  uchar    hasDeleteAll;
  int32    extDeleteAll;
  int32    noDeleteAllUsers;
  PYYLine  DeleteAllUsers;
  int32    noDeleteAllProjects;
  PYYLine  DeleteAllProjects;
  uchar    hasDeleteOne;
  int32    extDeleteOne;
  int32    noDeleteOneUsers;
  PYYLine  DeleteOneUsers;
  int32    noDeleteOneProjects;
  PYYLine  DeleteOneProjects;
  uchar    hasExists;
  int32    extExists;
  int32    noExistsUsers;
  PYYLine  ExistsUsers;
  int32    noExistsProjects;
  PYYLine  ExistsProjects;
  uchar    hasUpdate;
  int32    extUpdate;
  int32    noUpdateUsers;
  PYYLine  UpdateUsers;
  int32    noUpdateProjects;
  PYYLine  UpdateProjects;
  uchar    hasCount;
  int32    extCount;
  int32    noCountUsers;
  PYYLine  CountUsers;
  int32    noCountProjects;
  PYYLine  CountProjects;
  uchar    hasExecute;
  int32    noExecuteUsers;
  PYYLine  ExecuteUsers;
  int32    noExecuteProjects;
  PYYLine  ExecuteProjects;
  uchar    hasSelect;
  int32    noSelectUsers;
  PYYLine  SelectUsers;
  int32    noSelectProjects;
  PYYLine  SelectProjects;
  uchar    hasSequence;
  TYYField Count;
  int32    noUsers;
  PYYLine  Users;
  int32    noProjects;
  PYYLine  Projects;
  int32    noViews;
  PYYView  Views;
  int32    noDatas;
  int32    noProcs;
  PYYProc  Procs;
  pchar    Space;
  int32    Free;
  int32    Used;
  int32    Increase;
  int32    IniTrans;
  int32    MaxTrans;
  int32    Initial;
  char     InitialSize;
  int32    StorageNext;
  char     NextSize;
  int32    MinExtents;
  int32    MaxExtents;
  int32    Optimal;
  char     OptimalSize;
  int32    FreeLists;
  int32    FreeListGroups;
  uchar    hasImage;
  uchar    hasLOB;
  uchar    hasHugeCHAR;
  uchar    hasAudit;
  uchar    hasAuditFull;
  uchar    isNullEnabled;
  uchar    hasVB5IO;
  uchar    hasXMLTYPE;
  int      PrimaryKeyFields;
  pchar    ControlDB;
  pchar    ArchiveDir;
  pchar    ArchiveExt;
  pchar    CDir;
  pchar    CExt;
  pchar    CSIDL2Dir;
  pchar    CSIDL2Ext;
  pchar    CSNet7Dir;
  pchar    CSNet7Ext;
  pchar    CSRWDir;
  pchar    CSRWExt;
  pchar    IDLDir;
  pchar    IDLExt;
  pchar    IDLModule;
  pchar    LogDir;
  pchar    LogExt;
  pchar    PARMSDescr;
  pchar    PARMSDir;
  pchar    PARMSExt;
  pchar    PASDir;
  pchar    PASExt;
  pchar    PythonDir;
  pchar    PythonExt;
  pchar    SqlAuditExt;
  pchar    SqlConExt;
  pchar    SqlDir;
  pchar    SqlExt;
  pchar    SqlGrantExt;
  pchar    SqlIdxExt;
  pchar    SqlProcExt;
  pchar    SqlSnapExt;
  pchar    SqlTableExt;
  pchar    SqlViewsExt;
  pchar    SODir;
  pchar    SOExt;
  pchar    UConnect;
  pchar    VB5Dir;
  pchar    VB5Ext;
  pchar    VBCode3Dir;
  pchar    VBCode3Ext;
  pchar    VBDir;
  pchar    VBExt;
  pchar    VBNet7Dir;
  pchar    VBNet7Ext;
  pchar    VBforIDLDir;
  pchar    VBforIDLExt;
  pchar    ArchiveFileName;
  pchar    BinFileName;
  pchar    CSAdoNetFileName;
  pchar    CSAdoNetDir;
  pchar    CSAdoNetExt;
  pchar    CSIDL2FileName;
  pchar    CSNet7FileName;
  pchar    CSRWFileName;
  pchar    HeaderFileName;
  pchar    IDLFileName;
  pchar    InputFileName;
  pchar    LogFileName;
  pchar    PARMSFileName;
  pchar    PASFileName;
  pchar    PythonFileName;
  pchar    ShortName;
  pchar    SqlAuditFileName;
  pchar    SqlConFileName;
  pchar    SqlFileName;
  pchar    SqlGrantFileName;
  pchar    SqlIdxFileName;
  pchar    SqlProcFileName;
  pchar    SqlSnapFileName;
  pchar    SqlTableFileName;
  pchar    SqlViewsFileName;
  pchar    VB5FileName;
  pchar    VBCode3FileName;
  pchar    VBFileName;
  pchar    VBNet7FileName;
  pchar    VBforIDLFileName;
  pchar    PARMSLookup;
  pchar    Show;
  int      BlankPadded;
  int      CharZ;
  int      ConnReqd;
  int      ExitReqd;
  int      EightByte;
  int      LittleTrue;
  int      CppHeader;
  int      MarkedExtC;
  int      MarkedExtPAS;
  int      MarkedExtVB;
  int      ExtendedC;
  int      ExtendedPAS;
  int      ExtendedVB;
  int      Internal;
  int      Version2Bin;
  int      UseUsedBy;
  int      OneSQLScript;
  int      TargetSQL;
  int      TargetSO;
  int      TargetC;
  int      TargetIDL;
  int      TargetPAS;
  int      TargetARCHIVE;
  int      TargetPARMS;
  int      TargetVB;
  int      TargetVB5;
  int      TargetPython;
  int      TargetVBCode3;
  int      TargetVBNet7;
  int      TargetCSNet7;
  int      TargetCSIDL2;
  int      LittleTCSIDL2;
  int      TargetCSAdoNet;
  //int      TargetCSDap;
  int      TargetCSRW;
  int      TargetVBforIDL;
  int      TargetVBforADOR;
  int      MarkedC;
  int      MarkedIDL;
  int      MarkedPAS;
  int      MarkedVB;
  int      MarkedVB5;
  int      MarkedPython;
  int      MarkedVBCode3;
  int      MarkedVBNet7;
  int      MarkedCSNet7;
  int      MarkedCSIDL2;
  int      MarkedCSAdoNet;
  //int      MarkedCSDap;
  int      MarkedCSRW;
  int      MarkedVBforIDL;
  int      MarkedVBforADOR;
  int      UnderScore;
  int      IDLUbi;
  int      PrefixVBClasses;
  int      NoDomain;
  int      ViewOnly;
  TYYTable()
  {
    Clear();
  }
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYGrant
{
  int32  noPerms;
  PYYLine  Perms;
  int32  noUsers;
  PYYLine  Users;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYKey
{
  pchar  Name;
  int32  Length;
  int32  ABSize;
  int32  noFields;
  PYYField Fields;
  uchar  isPrimary;
  uchar  isUnique;
  pchar  Space;
  int32  Free;
  int32  IniTrans;
  int32  MaxTrans;
  int32  Initial;
  char   InitialSize;
  int32  StorageNext;
  char   NextSize;
  int32  Increase;
  int32  MinExtents;
  int32  MaxExtents;
  int32  Optimal;
  char   OptimalSize;
  int32  FreeLists;
  int32  FreeListGroups;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYView
{
  pchar  Name;
  int32  noLines;
  PYYLine  Lines;
  int32  noUsers;
  PYYLine  Users;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYProc
{
  pchar     Name;
  int32     Length;
  int32     ABSize;
  int32     noFields;
  PYYField  Fields;
  int32     noOutputs;
  int32     noBinds;
  int32*    Binds;
  int32     noWorkFields;
  PYYField  WorkFields;
  int32     noCursors;
  PYYCursor Cursors;
  int32     noLines;
  PYYLine   Lines;
  int32     noUsers;
  PYYLine   Users;
  int32     noProjects;
  PYYLine   Projects;
  uchar     isSql;
  uchar     isFetch;
  uchar     isData;
  uchar     useStd;
  uchar     useKey;
  uchar     isStd;
  uchar     isMultiFetch;
  uchar     isSProc;
  uchar     hasImage;
  uchar     hasLOB;
  uchar     hasHugeCHAR;
  uchar     hasXMLTYPE;
  int32     extProc;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYCursor
{
  int32  noLines;
  PYYLine  Lines;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

struct TYYLine
{
  pchar p;
  uchar isEOL;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup()
  {
    if (p) free(p);
  }
};

struct TYYLink
{
  pchar  MasterName;
  int32  noFields;
  PYYField Fields;
  bool   DeleteCascade;
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Cleanup();
};

extern int   yyleng;
extern FILE *yyin, *yyout;
extern pchar yytext;
extern pchar Work;

extern YYSTYPE yylval, yyval;

extern PYYTable Table;

extern TKeywordList KeywordsList[];
extern int KeywordsCount;

#ifdef __cplusplus
extern "C" {
#endif

int yylex();
int inbuf_parse(PYYTable aTable, char* InBuf);
int inbuf_lex(char* InBuf, int& BufIdx);
#ifdef __cplusplus
}
#endif

void GenerateOCI(PYYTable table);
void GenerateHeader(PYYTable Table);
void GenerateVB(PYYTable Table);
void GenerateVB5(PYYTable Table);
void GenerateVBCode3(PYYTable Table);
void GenerateVBNet7(PYYTable Table);
void GenerateCSNet7(PYYTable Table);
void GenerateCSIDL2(PYYTable Table);
void GenerateCSAdoNet(PYYTable Table);
//void GenerateCSDap(PYYTable Table);
void GenerateCSRW(PYYTable Table);
void GeneratePAS(PYYTable Table);
void GenerateIDL(PYYTable Table);
void GeneratePython(PYYTable Table);
void GenerateVBforIDL(PYYTable Table);
void GenerateVBforADOR(PYYTable Table);
void GeneratePARMS(PYYTable Table);
void GenerateArchive(PYYTable Table);
pchar TableToken(PYYTable Table);
int NullableField(PYYField Field);
int PortalLink(const char* ProjectName, const char* BinDir, const char* BinExt, const char* InputList, ushort version=0);
FILE* OpenOutput(const char* FullName);

extern char *OracleReservedWords;/* = ":"
"ABS:ACCESS:ACOS:ADD:ADD_MONTHS:ALL:ALTER:AND:ANY:AS:ASC:ASCII:ASCIISTR:ASIN:"
"ATAN:ATAN2:AUDIT:AVG:BETWEEN:BFILENAME:BIN_TO_NUM:BITAND:BY:CAST:CEIL:CHAR:"
"CHARTOROWID:CHECK:CHR:CLOB:CLUSTER:COALESCE:COLUMN:COMMENT:COMPOSE:COMPRESS:"
"CONCAT:CONNECT:CONVERT:CORR:COS:COSH:COUNT:COVAR_POP:COVAR_SAMP:CREATE:"
"CUME_DIST:CURRENT:CURRENT_DATE:CURRENT_TIMESTAMP:DATE:DBTIMEZONE:DECIMAL:"
"DECODE:DECOMPOSE:DEFAULT:DELETE:DENSE_RANK:DEREF:DESC:DISTINCT:DROP:DUMP:ELSE:"
"EMPTY_BLOB:EMPTY_:EXCLUSIVE:EXISTS:EXISTSNODE:EXP:EXTRACT:FILE:FIRST:FIRST:"
"FIRST_VALUE:FLOAT:FLOOR:FOR:FROM:FROM_TZ:GRANT:GREATEST:GROUP:GROUPING:"
"GROUPING_ID:GROUP_ID:HAVING:HEXTORAW:IDENTIFIED:IMMEDIATE:IN:INCREMENT:"
"INDEX:INITCAP:INITIAL:INSERT:INSTR:INTEGER:INTERSECT:INTO:IS:LAG:LAST:LAST:"
"LAST_DAY:LAST_VALUE:LEAD:LEAST:LENGTH:LEVEL:LIKE:LN:LOCALTIMESTAMP:LOCK:LOG:"
"LONG:LOWER:LPAD:LTRIM:MAKE_REF:MAX:MAXEXTENTS:MIN:MINUS:MLSLABEL:MOD:MODE:"
"MODIFY:MONTHS_BETWEEN:NEW_TIME:NEXT_DAY:NLSSORT:NLS_CHARSET_DECL_LEN:"
"NLS_CHARSET_ID:NLS_CHARSET_NAME:NLS_INITCAP:NLS_LOWER:NLS_UPPER:NOAUDIT:"
"NOCOMPRESS:NOT:NOWAIT:NTILE:NULL:NULLIF:NUMBER:NUMTODSINTERVAL:"
"NUMTODSINTERVAL:NUMTOYMINTERVAL:NUMTOYMINTERVAL:NVL:"
"NVL2:OF:OFFLINE:ON:ONLINE:OPTION:OR:ORDER:PCTFREE:PERCENTILE_CONT:"
"PERCENTILE_DISC:PERCENT_RANK:POWER:PRIOR:PRIVILEGES:PUBLIC:RANK:"
"RATIO_TO_REPORT:RAW:RAWTOHEX:RAWTONHEX:REF:REFTOHEX:REGR_:RENAME:REPLACE:"
"RESOURCE:REVOKE:ROUND:ROUND:ROW:ROWID:ROWIDTOCHAR:ROWIDTONCHAR:ROWNUM:ROWS:"
"ROW_NUMBER:RPAD:RTRIM:SELECT:SESSION:SESSIONTIMEZONE:SET:SHARE:SIGN:SIN:SINH:"
"SIZE:SMALLINT:SOUNDEX:SQRT:START:STDDEV:STDDEV_POP:STDDEV_SAMP:SUBSTR:"
"SUCCESSFUL:SUM:SYNONYM:SYSDATE:SYSTIMESTAMP:SYS_CONNECT_BY_PATH:"
"SYS_CONTEXT:SYS_DBURIGEN:SYS_EXTRACT_UTC:SYS_GUID:SYS_TYPEID:SYS_XMLAGG:"
"SYS_XMLGEN:TABLE:TAN:TANH:THEN:TO:TO_CHAR:TO_CLOB:TO_DATE:TO_DSINTERVAL:"
"TO_LOB:TO_MULTI_BYTE:TO_NCHAR:TO_NCLOB:TO_NUMBER:TO_SINGLE_BYTE:TO_TIMESTAMP:"
"TO_TIMESTAMP_TZ:TO_YMINTERVAL:TRANSLATE:TREAT:TRIGGER:TRIM:TRUNC:TZ_OFFSET:"
"UID:UNION:UNIQUE:UNISTR:UPDATE:UPPER:USER:USERENV:USING:VALIDATE:VALUE:VALUES:"
"VARCHAR:VARCHAR2:VARIANCE:VAR_POP:VAR_SAMP:VIEW:VSIZE:WHENEVER:WHERE:WITH:XMLTYPE:"
;*/

#endif


