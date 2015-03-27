%code top {
  #include "machine.h"
  
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <ctype.h>
  
  #include "xstring.h"
  #include "global.h"
  #include "sqltok.h"

  PYYTable Table;
  /* ------------------------ */
  /*  static global variables */
  /* ------------------------ */
  static eState    GrantState;
  static eState    ProjectState;
  static eState    FieldState;
  static eState    LineState;
  static eState    ColumnState;
  static int32     ExtState;
  static TYYConst  Const;
  static TYYField  Field;
  static TYYKey    Key;
  static TYYLink   Link;
  static TYYGrant  Grant;
  static TYYCursor Cursor;
  static TYYProc   Proc;
  static TYYView   View;
  static TYYLine   Line;
  static TYYLine   Perm;
  static TYYLine   User;
  static char      NumberSize;
}

%union {
  int   i;
  char* s;
}

%code {
  /*  ---------- */
  /*  prototypes */
  /*  ---------- */
  static void SetLink(char* s);
  static void AddLink(void);
  static void DeleteCascadeLink(bool val);
  static void AddField(char* s);
  static void AddCharConst(char* s, int i);
  static void AddConst(char* s, int i);
  static void AddConstIdent(char* s);
  static void AddStringConst(char* s, char* d);
  static void UseStdRecOutput();
  static void SetFieldType(eFieldType Type, int Length);
  static void SetSameFromTable();
  static void SetFromTable(char *s);
  static void AddPermission(char* s);
  static void AddUser(char* s);
  static void AddProject(char* s);
  static void AddGrant(void);
  static void SetKey(char* s);
  static void AddKey(void);
  static void AddColumn(char* s);
  static void SetProc(char *s);
  static void SetFlag(char *s);
  static void SetView(char *s);
  static void AddProc(void);
  static void AddView(void);
  static void AddLine(char *s);
  static PYYField FindField(char* s);
  static char *Concat(char *s1, char *s2);
  static char *Combine(char *s1, char *s2);
  static void CreateDelVer(void);
  static void CreateRowSeqExists(void);
  static void CreateCount(void);
  static void CreateDeleteAll(void);
  static void CreateDeleteOne(void);
  static void CreateInsert(void);
  static void CreateInsertUsingReturning(void);
  static void CreateSelectAll(int Update);
  static void CreateSelectOne(int Update);
  static void CreateSelectPlus(int size);
  static void CreateUpdate(void);
  static void CreateUpdateUsingReturning(void);
  static void CreateExists(void);
  static pchar Qualify(pchar TableName, pchar FieldName);
  static pchar InputConversion(TYYField Field, int Extended=0);
  static pchar OutputConversion(TYYField Field);
  static void AddRowUser(void);
  static void AddRowUpdated(void);
  static void AddRowStatus(void);
  static int32 Hash(char *s, int m);
  static void SetSwitch(char *&Switch, char *value);
  static void SetSwitch(int  &Switch,  int value);

  #define AddList(Pointer, Type, Data, Index, Delta) do {\
    if (Index % Delta == 0) \
      Pointer = (Type*) realloc(Pointer, sizeof(Data)*(Index + Delta)); \
    Pointer[Index++] = Data; \
    } while(0)

  char *OracleReservedWords = ":"
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
    "VARCHAR:VARCHAR2:VARIANCE:VAR_POP:VAR_SAMP:VIEW:VSIZE:WHENEVER:WHERE:WITH:"
    ;
}

%token     sqldefALL
%token     sqldefAUDIT
%token     sqldefBINARY
%token     sqldefBLOB
%token     sqldefBOOLEAN
%token     sqldefCASCADE
%token     sqldefCHAR
%token <i> sqldefCHARCONST
%token     sqldefCHECK
%token     sqldefCLOB
%token     sqldefCODE
%token     sqldefCONNECT
%token     sqldefCOUNT
%token     sqldefCURSOR
%token     sqldefDATETIME
%token     sqldefDATA
%token     sqldefDATE
%token     sqldefDECLARE
%token     sqldefDELETE
%token     sqldefDELETEALL
%token     sqldefDELETEONE
%token     sqldefENDCODE
%token     sqldefENDCURSOR
%token     sqldefENDDATA
%token     sqldefEXCEPTION
%token     sqldefEXECUTE
%token     sqldefEXISTS
%token <s> sqldefEXTENDED
%token     sqldefFAST
%token     sqldefFLOAT
%token     sqldefFREELIST
%token     sqldefFREELISTS
%token     sqldefFROM
%token     sqldefFULL
%token     sqldefGRANT
%token     sqldefGROUPS
%token     sqldefHUGECHAR
%token <s> sqldefIDENT
%token     sqldefINITIAL
%token     sqldefIMAGE
%token     sqldefINITRANS
%token     sqldefINOUT
%token     sqldefINPUT
%token     sqldefINSERT
%token     sqldefINT
%token     sqldefK
%token     sqldefKEY
%token <s> sqldefLINE
%token     sqldefLINK
%token     sqldefLONG
%token     sqldefM
%token     sqldefMAXEXTENTS
%token     sqldefMAXTRANS
%token     sqldefMINEXTENTS
%token     sqldefMONEY
%token     sqldefMULTIPLE
%token     sqldefNEXT
%token     sqldefNOC
%token     sqldefNOEXT
%token     sqldefNOIDL
%token     sqldefNOPAS
%token     sqldefNOT
%token     sqldefNOVB
%token     sqldefNULL
%token <i> sqldefNUMBER
%token     sqldefOPTIMAL
%token     sqldefOUTPUT
%token     sqldefPCTFREE
%token     sqldefPCTINCREASE
%token     sqldefPCTUSED
%token     sqldefPRIMARY
%token     sqldefPROC
%token     sqldefSCHEMA
%token     sqldefSELECT
%token     sqldefSELECTALL
%token     sqldefSELECTALLUPD
%token     sqldefSELECTONE
%token     sqldefSELECTONEUPD
%token     sqldefSELECTPLUS
%token     sqldefSEQUENCE
%token     sqldefSERVER
%token     sqldefSINGLE
%token     sqldefSMALLINT
%token     sqldefSQL
%token     sqldefSQLCODE
%token <s> sqldefSTRING
%token     sqldefTABLE
%token     sqldefTABLESPACE
%token     sqldefTIMESTAMP
%token     sqldefTINYINT
%token     sqldefTO
%token     sqldefUNIQUE
%token     sqldefUPDATE
%token     sqldefUPPER
%token     sqldefUSEDBY
%token     sqldefUSERSTAMP
%token     sqldefVARNUM
%token     sqldefVIEW
%token     sqldefXMLTYPE
%token     sqldefZLOB

%token     switchBINDIR
%token     switchBINEXT
%token     switchBLANKPADDED
%token     switchCDIR
%token     switchCEXT
%token     switchCHARZ
%token     switchCONNREQD
%token     switchCONTROLDB
%token     switchCPPHEADER
%token     switchEXITREQD
%token     switchEIGHTBYTE
%token     switchEXTENDEDC
%token     switchEXTENDEDPAS
%token     switchEXTENDEDVB
%token     switchHEADERDIR
%token     switchHEADEREXT
%token     switchIDLDIR
%token     switchIDLEXT
%token     switchIDLMODULE
%token     switchIDLUBI
%token     switchINTERNAL
%token     switchVERSION2BIN
%token     switchLITTLETRUE
%token     switchLOGDIR
%token     switchLOGEXT
%token     switchNODOMAIN
%token     switchONESQLSCRIPT
%token     switchPASDIR
%token     switchPASEXT
%token     switchPYTHONDIR
%token     switchPYTHONEXT
%token     switchSHOW
%token     switchSODIR
%token     switchSOEXT
%token     switchSQLAUDITEXT
%token     switchSQLCONEXT
%token     switchSQLDIR
%token     switchSQLEXT
%token     switchSQLGRANTEXT
%token     switchSQLIDXEXT
%token     switchSQLPROCEXT
%token     switchSQLSNAPEXT
%token     switchSQLTABLEEXT
%token     switchSQLVIEWSEXT
%token     switchTARGETC
%token     switchTARGETIDL
%token     switchTARGETPAS
%token     switchARCHIVEDIR
%token     switchARCHIVEEXT
%token     switchPARMSDIR
%token     switchPARMSDESCR
%token     switchPARMSEXT
%token     switchPARMSLOOKUP
%token     switchTARGETARCHIVE
%token     switchTARGETPARMS
%token     switchTARGETPYTHON
%token     switchTARGETVB
%token     switchTARGETVB5
%token     switchUCONNECT
%token     switchUNDERSCORE
%token     switchVB5DIR
%token     switchVB5EXT
%token     switchVBDIR
%token     switchVBEXT
%token     switchVBFORIDLDIR
%token <s> switchINVALID
%token     switchPREFIXVBCLASSES
%token     switchTARGETVBCODE3
%token     switchTARGETVBNET7
%token     switchTARGETCSNET7
%token     switchTARGETCSIDL2
%token     switchTARGETCSADONET
%token     switchTARGETCSRW
%token     switchVBCODE3DIR
%token     switchVBCODE3EXT
%token     switchVBNET7DIR
%token     switchVBNET7EXT
%token     switchCSNET7DIR
%token     switchCSADONETDIR
%token     switchCSRWDIR
%token     switchCSIDL2DIR
%token     switchCSNET7EXT
%token     switchCSADONETEXT
%token     switchCSRWEXT
%token     switchCSIDL2EXT
%token     switchVIEWONLY
%token     switchTARGETVBFORIDL
%token     switchTARGETVBFORADOR
%token     switchLITTLETCSIDL2

%type  <i> OptNumber
%type  <i> OptPlusNumber
%type  <i> Number
%type  <i> ANumber
%type  <i> SwitchNumber
%type  <s> AnIdent
%type  <s> QualifiedIdent
%type  <s> FullServerIdent
/*%type  <s> ServerIdent*/

%start SqlDef

%%

SqlDef
: Switches ServerName Flags ConnectName SetModDefaults OptTableMods TableDef OptExtras ProcDef
;

SetModDefaults :
{
  Table->Count.Name = strdup("NoOfRecs");
  Table->Count.Type = ftypeInt;
  Table->Count.Length = 4;
  Table->Count.Padding = Table->EightByte==1 ? 8 : 4;
  Table->ABSize = 12;
  Table->Space = 0;
  Table->Free = 0;
  Table->Used = 0;
  Table->Increase = 0;
  Table->IniTrans = 0;
  Table->MaxTrans = 0;
  Table->Initial = 0;
  Table->StorageNext = 0;
  Table->MinExtents = 0;
  Table->MaxExtents = 0;
  Table->Optimal = 0;
  Table->FreeLists = 0;
  Table->FreeListGroups = 0;
  Table->Alias = 0;
  Table->isNullEnabled = 0;
};

Switches
:
| Switches Switch
;

Switch
: switchBINDIR          sqldefSTRING  {SetSwitch(Table->SODir           , $2);}
| switchBINEXT          sqldefSTRING  {SetSwitch(Table->SOExt           , $2);}
| switchCDIR            sqldefSTRING  {SetSwitch(Table->CDir            , $2);}
| switchCEXT            sqldefSTRING  {SetSwitch(Table->CExt            , $2);}
| switchBLANKPADDED     SwitchNumber  {SetSwitch(Table->BlankPadded     , $2);}
| switchCHARZ           SwitchNumber  {SetSwitch(Table->CharZ           , $2);}
| switchCONNREQD        SwitchNumber  {SetSwitch(Table->ConnReqd        , $2);}
| switchCONTROLDB       sqldefSTRING  {SetSwitch(Table->ControlDB       , $2);}
| switchEIGHTBYTE       SwitchNumber  {SetSwitch(Table->EightByte       , $2);}
| switchEXITREQD        SwitchNumber  {SetSwitch(Table->ExitReqd        , $2);}
| switchEXTENDEDC       SwitchNumber  {SetSwitch(Table->ExtendedC       , $2);}
| switchEXTENDEDPAS     SwitchNumber  {SetSwitch(Table->ExtendedPAS     , $2);}
| switchEXTENDEDVB      SwitchNumber  {SetSwitch(Table->ExtendedVB      , $2);}
| switchHEADERDIR       sqldefSTRING  {SetSwitch(Table->CDir            , $2);}
| switchHEADEREXT       sqldefSTRING  {SetSwitch(Table->CExt            , $2);}
| switchIDLDIR          sqldefSTRING  {SetSwitch(Table->IDLDir          , $2);}
| switchIDLEXT          sqldefSTRING  {SetSwitch(Table->IDLExt          , $2);}
| switchIDLMODULE       sqldefSTRING  {SetSwitch(Table->IDLModule       , $2);}
| switchINTERNAL        SwitchNumber  {SetSwitch(Table->Internal        , $2);}
| switchVERSION2BIN     SwitchNumber  {SetSwitch(Table->Version2Bin     , $2);}
| switchLOGDIR          sqldefSTRING  {SetSwitch(Table->LogDir          , $2);}
| switchLOGEXT          sqldefSTRING  {SetSwitch(Table->LogExt          , $2);}
| switchNODOMAIN        SwitchNumber  {SetSwitch(Table->NoDomain        , $2);}
| switchONESQLSCRIPT    SwitchNumber  {SetSwitch(Table->OneSQLScript    , $2);}
| switchARCHIVEDIR      sqldefSTRING  {SetSwitch(Table->ArchiveDir      , $2);}
| switchARCHIVEEXT      sqldefSTRING  {SetSwitch(Table->ArchiveExt      , $2);}
| switchPARMSDESCR      sqldefSTRING  {SetSwitch(Table->PARMSDescr      , $2);}
| switchPARMSDIR        sqldefSTRING  {SetSwitch(Table->PARMSDir        , $2);}
| switchPARMSEXT        sqldefSTRING  {SetSwitch(Table->PARMSExt        , $2);}
| switchPARMSLOOKUP     sqldefSTRING  {SetSwitch(Table->PARMSLookup     , $2);}
| switchPASDIR          sqldefSTRING  {SetSwitch(Table->PASDir          , $2);}
| switchPASEXT          sqldefSTRING  {SetSwitch(Table->PASExt          , $2);}
| switchPYTHONDIR       sqldefSTRING  {SetSwitch(Table->PythonDir       , $2);}
| switchPYTHONEXT       sqldefSTRING  {SetSwitch(Table->PythonExt       , $2);}
| switchSHOW            sqldefSTRING  {SetSwitch(Table->Show            , $2);}
| switchSODIR           sqldefSTRING  {SetSwitch(Table->SODir           , $2);}
| switchSOEXT           sqldefSTRING  {SetSwitch(Table->SOExt           , $2);}
| switchSQLAUDITEXT     sqldefSTRING  {SetSwitch(Table->SqlAuditExt     , $2);}
| switchSQLCONEXT       sqldefSTRING  {SetSwitch(Table->SqlConExt       , $2);}
| switchSQLDIR          sqldefSTRING  {SetSwitch(Table->SqlDir          , $2);}
| switchSQLEXT          sqldefSTRING  {SetSwitch(Table->SqlExt          , $2);}
| switchSQLGRANTEXT     sqldefSTRING  {SetSwitch(Table->SqlGrantExt     , $2);}
| switchSQLIDXEXT       sqldefSTRING  {SetSwitch(Table->SqlIdxExt       , $2);}
| switchSQLPROCEXT      sqldefSTRING  {SetSwitch(Table->SqlProcExt      , $2);}
| switchSQLSNAPEXT      sqldefSTRING  {SetSwitch(Table->SqlSnapExt      , $2);}
| switchSQLTABLEEXT     sqldefSTRING  {SetSwitch(Table->SqlTableExt     , $2);}
| switchSQLVIEWSEXT     sqldefSTRING  {SetSwitch(Table->SqlViewsExt     , $2);}
| switchTARGETC         SwitchNumber  {SetSwitch(Table->TargetC         , $2);}
| switchTARGETIDL       SwitchNumber  {SetSwitch(Table->TargetIDL       , $2);}
| switchTARGETARCHIVE   SwitchNumber  {SetSwitch(Table->TargetARCHIVE   , $2);}
| switchTARGETPARMS     SwitchNumber  {SetSwitch(Table->TargetPARMS     , $2);}
| switchTARGETPAS       SwitchNumber  {SetSwitch(Table->TargetPAS       , $2);}
| switchTARGETPYTHON    SwitchNumber  {SetSwitch(Table->TargetPython    , $2);}
| switchTARGETVB        SwitchNumber  {SetSwitch(Table->TargetVB        , $2);}
| switchTARGETVB5       SwitchNumber  {SetSwitch(Table->TargetVB5       , $2);}
| switchTARGETVBCODE3   SwitchNumber  {SetSwitch(Table->TargetVBCode3   , $2);}
| switchTARGETVBNET7    SwitchNumber  {SetSwitch(Table->TargetVBNet7    , $2);}
| switchTARGETCSNET7    SwitchNumber  {SetSwitch(Table->TargetCSNet7    , $2);}
| switchTARGETCSIDL2    SwitchNumber  {SetSwitch(Table->TargetCSIDL2    , $2);}
| switchLITTLETCSIDL2   SwitchNumber  {SetSwitch(Table->LittleTCSIDL2   , $2);}
| switchTARGETCSADONET  SwitchNumber  {SetSwitch(Table->TargetCSAdoNet  , $2);}
| switchTARGETCSRW      SwitchNumber  {SetSwitch(Table->TargetCSRW      , $2);}
| switchTARGETVBFORIDL  SwitchNumber  {SetSwitch(Table->TargetVBforIDL  , $2);}
| switchTARGETVBFORADOR SwitchNumber  {SetSwitch(Table->TargetVBforADOR , $2);}
| switchPREFIXVBCLASSES SwitchNumber  {SetSwitch(Table->PrefixVBClasses , $2);}
| switchUCONNECT        sqldefSTRING  {SetSwitch(Table->UConnect        , $2);}
| switchUNDERSCORE      SwitchNumber  {SetSwitch(Table->UnderScore      , $2);}
| switchVB5DIR          sqldefSTRING  {SetSwitch(Table->VB5Dir          , $2);}
| switchVB5EXT          sqldefSTRING  {SetSwitch(Table->VB5Ext          , $2);}
| switchVBCODE3DIR      sqldefSTRING  {SetSwitch(Table->VBCode3Dir      , $2);}
| switchVBCODE3EXT      sqldefSTRING  {SetSwitch(Table->VBCode3Ext      , $2);}
| switchVBNET7DIR       sqldefSTRING  {SetSwitch(Table->VBNet7Dir       , $2);}
| switchVBNET7EXT       sqldefSTRING  {SetSwitch(Table->VBNet7Ext       , $2);}
| switchCSNET7DIR       sqldefSTRING  {SetSwitch(Table->CSNet7Dir       , $2);}
| switchCSADONETDIR     sqldefSTRING  {SetSwitch(Table->CSAdoNetDir     , $2);}
| switchCSRWDIR         sqldefSTRING  {SetSwitch(Table->CSRWDir         , $2);}
| switchCSNET7EXT       sqldefSTRING  {SetSwitch(Table->CSNet7Ext       , $2);}
| switchCSADONETEXT     sqldefSTRING  {SetSwitch(Table->CSAdoNetExt     , $2);}
| switchCSRWEXT         sqldefSTRING  {SetSwitch(Table->CSRWExt         , $2);}
| switchCSIDL2DIR       sqldefSTRING  {SetSwitch(Table->CSIDL2Dir       , $2);}
| switchCSIDL2EXT       sqldefSTRING  {SetSwitch(Table->CSIDL2Ext       , $2);}
| switchVBFORIDLDIR     sqldefSTRING  {SetSwitch(Table->VBforIDLDir     , $2);}
| switchVBDIR           sqldefSTRING  {SetSwitch(Table->VBDir           , $2);}
| switchVBEXT           sqldefSTRING  {SetSwitch(Table->VBExt           , $2);}
| switchVIEWONLY        SwitchNumber  {SetSwitch(Table->ViewOnly        , $2);}
| switchLITTLETRUE      SwitchNumber  {SetSwitch(Table->LittleTrue      , $2);}
| switchCPPHEADER       SwitchNumber  {SetSwitch(Table->CppHeader       , $2);}
| switchINVALID
{
  yyreport(CODING_ERROR, "%s is not a valid switch\n", $1);
  free($1);
}
;

SwitchNumber
:
{
  $$ = 1;
}
| Number
{
  $$ = $1;
}
;

ServerName
: sqldefSERVER FullServerIdent
{
  Table->Server = $2;
};

FullServerIdent
: sqldefIDENT
{
  $$ = $1;
}
| sqldefEXTENDED
{
  $$ = $1;
};

/*ServerIdent
: sqldefIDENT
{
  $$ = $1;
}
| ServerIdent ":" sqldefIDENT
{
  $$ = Combine($1, Combine(strdup(":"), $3));
};*/

Flags
:
| Flags Flag
;

Flag
: '(' AnIdent ')'
{
  SetFlag($2);
};

AnIdent
: sqldefIDENT
{
  $$ = $1;
};

ConnectName
: sqldefSCHEMA AnIdent
{
  Table->UserID = $2;
  Table->PassWD = strdup($2);
}
| sqldefCONNECT AnIdent '/' AnIdent
{
  Table->UserID = $2;
  Table->PassWD = $4;
};

OptTableMods
:
| OptTableMods TableMod
;

TableMod
: sqldefINITIAL ANumber NumberSize
{
  Table->Initial = (int32) $2;
  Table->InitialSize = NumberSize;
}
| sqldefINITRANS ANumber
{
  Table->IniTrans = (int32) $2;
}
| sqldefFREELIST sqldefGROUPS ANumber
{
  Table->FreeListGroups = (int32) $3;
}
| sqldefFREELISTS ANumber
{
  Table->FreeLists = (int32) $2;
}
| sqldefMINEXTENTS ANumber
{
  Table->MinExtents = (int32) $2;
}
| sqldefMAXTRANS ANumber
{
  Table->MaxTrans = (int32) $2;
}
| sqldefMAXEXTENTS ANumber
{
  Table->MaxExtents = (int32) $2;
}
| sqldefNEXT ANumber NumberSize
{
  Table->StorageNext = (int32) $2;
  Table->NextSize = NumberSize;
}
| sqldefOPTIMAL ANumber NumberSize
{
  Table->Optimal = (int32) $2;
  Table->OptimalSize = NumberSize;
}
| sqldefPCTFREE ANumber
{
  Table->Free = (int32) $2;
}
| sqldefPCTUSED ANumber
{
  Table->Used = (int32) $2;
}
| sqldefPCTINCREASE ANumber
{
  Table->Increase = (int32) $2;
}
| sqldefTABLESPACE AnIdent
{
  Table->Space = $2;
};

ANumber
: sqldefNUMBER
{
  $$ = $1;
};

NumberSize
:
{
  NumberSize = ' ';
}
| sqldefK
{
  NumberSize = 'K';
}
| sqldefM
{
  NumberSize = 'M';
};

TableDef
: TableName SetStateTable FieldDef
;

SetStateTable :
{
  FieldState = stateTable;
};

TableName
: sqldefTABLE AnIdent OptTableAlias OptTableCheck
{
  Table->Name = $2;
  Table->Token = Hash(Table->Name, 13);
  Table->ABSize += (strlen(Table->Name)+8);
};

AnIdentNull
: sqldefNULL
{
  Table->isNullEnabled = 1;
}
| AnIdent
{
  Table->Alias = $1;
}
;

OptTableAlias
:
| OptTableAlias '[' AnIdentNull ']'
| OptTableAlias '(' AnIdentNull ')' 
;

OptTableCheck
:
{
  Table->Check = 0;
}
| sqldefCHECK sqldefSTRING
{
  Table->Check = $2;
};

FieldDef
:
| FieldDef Field
;

Field
: AnIdent SetField {Field.Name = $1;} OptAlias FieldType OptNullUppers
{
  AddField($1);
};

SetField :
{
  Field.Clear();
};

OptAlias
:
{
  Field.Alias = 0;
}
| '(' AnIdent ')'
{
  Field.Alias = $2;
};

FieldType
: sqldefCHAR OptNumber OptConstIdents
{
  SetFieldType(ftypeChar, $2);
}
| sqldefBINARY OptNumber
{
  SetFieldType(ftypeBinary, $2);
}
| sqldefBOOLEAN
{
  SetFieldType(ftypeBoolean, 0);
}
| sqldefTINYINT OptConsts
{
  SetFieldType(ftypeTinyint, 0);
}
| sqldefSMALLINT OptConsts
{
  SetFieldType(ftypeSmallint, 0);
}
| sqldefINT OptConsts
{
  SetFieldType(ftypeInt, 0);
}
| '=' OptRefs
| sqldefLONG OptConsts
{
  SetFieldType(ftypeLong, 0);
}
| sqldefDATE
{
  SetFieldType(ftypeDate, 0);
}
| sqldefTIMESTAMP
{
  SetFieldType(ftypeTimeStamp, 0);
}
| sqldefUSERSTAMP
{
  SetFieldType(ftypeUserStamp, 0);
}
| sqldefSEQUENCE
{
  SetFieldType(ftypeInt, 0);
  if (Table->hasSequence)
    yyreport(CODING_ERROR, "Table already has sequence\n");
  else
  {
    Field.isSequence = 1;
    Table->hasSequence = 1;
  }
}
| sqldefFLOAT OptPrecision
{
  SetFieldType(ftypeFloat, 0);
}
| sqldefMONEY Precision
{
  SetFieldType(ftypeMoney, 0);
}
| sqldefBLOB Number
{
  if (FieldState == stateTable)
    Table->hasLOB = 1;
  else
    Proc.hasLOB = 1;
  SetFieldType(ftypeBLOB, $2);
  if (FieldState == stateOutput && Proc.isMultiFetch)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "(multiple) cannot be used with LOB data.\n");
    Proc.isMultiFetch = 0;
  }
}
| sqldefCLOB Number
{
  if (FieldState == stateTable)
    Table->hasLOB = 1;
  else
    Proc.hasLOB = 1;
  SetFieldType(ftypeCLOB, $2);
  if (FieldState == stateOutput && Proc.isMultiFetch)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "(multiple) cannot be used with LOB data.\n");
    Proc.isMultiFetch = 0;
  }
}
| sqldefZLOB Number
{
  if (FieldState == stateTable)
    Table->hasLOB = 1;
  else
    Proc.hasLOB = 1;
  SetFieldType(ftypeZLOB, $2);
  if (FieldState == stateOutput && Proc.isMultiFetch)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "(multiple) cannot be used with LOB data.\n");
    Proc.isMultiFetch = 0;
  }
}
| sqldefHUGECHAR
{
  if (FieldState == stateTable)
    Table->hasHugeCHAR = 1;
  else
    Proc.hasHugeCHAR = 1;
  SetFieldType(ftypeHugeCHAR, 4);
}
| sqldefIMAGE OptNumber
{
  if (FieldState == stateTable)
    Table->hasImage = 1;
  else
    Proc.hasImage = 1;
  SetFieldType(ftypeImage, $2);
  if (FieldState == stateOutput && Proc.isMultiFetch)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "(multiple) cannot be used with Image data.\n");
    Proc.isMultiFetch = 0;
  }
}
| sqldefXMLTYPE Number
{
  if (FieldState == stateTable)
    Table->hasXMLTYPE = 1;
  else
    Proc.hasXMLTYPE = 1;
  SetFieldType(ftypeXMLTYPE, $2);
  if (FieldState == stateOutput && Proc.isMultiFetch)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "(multiple) cannot be used with XMLTYPE data.\n");
    Proc.isMultiFetch = 0;
  }
}
| sqldefDATETIME
{
  SetFieldType(ftypeDateTime, 0);
}
| sqldefVARNUM OptPrecision
{
  SetFieldType(ftypeVarNum, 0);
};

OptRefs
:
{
  SetSameFromTable();
}
| '(' AnIdent ')'
{
  SetFromTable($2);
}
;

OptPrecision
:
{
  Field.Precision = 0;
  Field.Scale = 0;
}
| Precision
;

Precision
: '(' Number ',' Number ')'
{
  Field.Precision = $2;
  Field.Scale     = $4;
}
;

OptNumber
:
{
  $$ = 1;
}
| Number
{
  $$ = $1;
}
;

Number
: ANumber
{
  $$ = $1;
}
| '[' ANumber ']'
{
  $$ = $2;
}
| '(' ANumber ')'
{
  $$ = $2;
};

OptNullUppers
:
| OptNullUppers OptNullUpper
;

OptNullUpper
: sqldefNULL
{
  Field.isNull = 1;
}
| sqldefNOT sqldefNULL
{
  Field.isNull = 0;
}
| sqldefUPPER
{
  Field.isUpper = 1;
}
| sqldefCHECK sqldefSTRING
{
  if (Field.Check) free(Field.Check);
  Field.Check = $2;
}
;

OptConstIdents
:
| '{' ConstIdents '}'
;

ConstIdents
: ConstIdent
| ConstIdents OptComma ConstIdent
;

ConstIdent
: AnIdent
{
  AddConstIdent($1);
}
;

OptConsts
:
| '[' Consts ']'
| '(' Consts ')'
| '{' Consts '}'
;

Consts
: Const
| Consts OptComma Const
;

OptComma
:
| ','
;

Const
: AnIdent '=' Number
{
  AddConst($1, $3);
}
| AnIdent '=' sqldefCHARCONST
{
  AddCharConst($1, $3);
}
| AnIdent '=' sqldefSTRING
{
  AddStringConst($1, $3);
}
;

OptExtras
:
| OptExtras Extra
;

Extra
: GrantDef
| KeyDef
| Link
| Audit
| ViewDef
;

GrantDef
: sqldefGRANT SetGrant Permissions sqldefTO Users
{
  AddGrant();
};

SetGrant :
{
  GrantState = stateTableGrant;
  memset(&Grant, 0, sizeof(TYYGrant));
};

Permissions
: Permission
| Permissions Permission
;

Permission
: sqldefALL
{
  AddPermission(strdup("all"));
  Table->hasExecute = 1;
  Table->hasSelect  = 1;
}
| sqldefDELETE
{
  AddPermission(strdup("delete"));
}
| sqldefINSERT
{
  AddPermission(strdup("insert"));
}
| sqldefSELECT
{
  AddPermission(strdup("select"));
  Table->hasSelect  = 1;
}
| sqldefUPDATE
{
  AddPermission(strdup("update"));
}
| sqldefEXECUTE
{
  Table->hasExecute = 1;
};

Users
: User
| Users User
;

User
: AnIdent
{
  AddUser($1);
};

KeyDef
: sqldefKEY KeyName Modifiers Columns
{
  AddKey();
};

KeyName
: AnIdent
{
  SetKey($1);
  ColumnState = stateKey;
};

Modifiers
:
| Modifiers Modifier
;

Modifier
: sqldefUNIQUE
{
  Key.isUnique = 1;
}
| sqldefPRIMARY
{
  Key.isPrimary = 1;
  Table->hasPrimaryKey = 1;
}
| sqldefTABLESPACE AnIdent
{
  Key.Space = $2;
}
| sqldefINITIAL ANumber NumberSize
{
  Key.Initial = (int32) $2;
  Key.InitialSize = NumberSize;
}
| sqldefINITRANS ANumber
{
  Key.IniTrans = (int32) $2;
}
| sqldefFREELIST sqldefGROUPS ANumber
{
  Key.FreeListGroups = (int32) $3;
}
| sqldefFREELISTS ANumber
{
  Key.FreeLists = (int32) $2;
}
| sqldefMINEXTENTS ANumber
{
  Key.MinExtents = (int32) $2;
}
| sqldefMAXTRANS ANumber
{
  Key.MaxTrans = (int32) $2;
}
| sqldefMAXEXTENTS ANumber
{
  Key.MaxExtents = (int32) $2;
}
| sqldefNEXT ANumber NumberSize
{
  Key.StorageNext = (int32) $2;
  Key.NextSize = NumberSize;
}
| sqldefOPTIMAL ANumber NumberSize
{
  Key.Optimal = (int32) $2;
  Key.OptimalSize = NumberSize;
}
| sqldefPCTFREE ANumber
{
  Key.Free = (int32) $2;
}
| sqldefPCTINCREASE ANumber
{
  Key.Increase = (int32) $2;
};

Columns
: Column
| Columns OptComma Column
;

Column
: AnIdent
{
  AddColumn($1);
};

Link
: sqldefLINK NewLink MasterName Columns OptDeleteCascade
{
  AddLink();
};

OptDeleteCascade
:
{
  DeleteCascadeLink(false);
}
| sqldefDELETE sqldefCASCADE
{
  DeleteCascadeLink(true);
};

NewLink
:
{
  memset(&Link, 0, sizeof(TYYLink));
};

MasterName
: QualifiedIdent
{
  SetLink($1);
}
| error
{
  yyreport(CODING_ERROR, "The link table name expected\n");
};

QualifiedIdent
: sqldefIDENT
{
  $$ = $1;
}
| QualifiedIdent "." sqldefIDENT
{
  $$ = Concat($1, $3);
};

Audit
: sqldefAUDIT AuditFull
;

AuditFull
:
{
  Table->hasAudit = 1;
}
| sqldefFULL
{
  Table->hasAudit = 1;
  Table->hasAuditFull = 1;
};

ProcDef
:
| ProcDef Proc
;

Proc
: sqldefPROC SetExt StdProc ToUsers UsedBy
| sqldefPROC SetExt SetProcGrant SetProjectState UserProc
| Data
;

SetExt :
{
  ExtState = 0x00000000;
};

SetProcGrant :
{
  GrantState = stateProcGrant;
};

SetProjectState :
{
  ProjectState = stateProcProject;
};

StdProc
: sqldefCOUNT NoExt
{
  if (Table->hasCount == 0)
    CreateCount();
  Table->hasCount = 1;
  Table->extCount = ExtState;
  GrantState = stateCountProcGrant;
  ProjectState = stateCountProcProject;
}
| sqldefDELETEALL NoExt
{
  if (Table->hasDeleteAll == 0)
    CreateDeleteAll();
  Table->hasDeleteAll = 1;
  Table->extDeleteAll = ExtState;
  GrantState = stateDeleteAllProcGrant;
  ProjectState = stateDeleteAllProcProject;
}
| sqldefDELETEONE NoExt
{
  if (!Table->hasPrimaryKey)
    yyreport(CODING_ERROR, "No primary key for DeleteOne\n");
  if (Table->hasDeleteOne == 0)
     CreateDeleteOne();
  Table->hasDeleteOne = 1;
  Table->extDeleteOne = ExtState;
  GrantState = stateDeleteOneProcGrant;
  ProjectState = stateDeleteOneProcProject;
}
| sqldefEXISTS NoExt
{
  if (!Table->hasPrimaryKey)
    yyreport(CODING_ERROR, "No primary key for Exists\n");
  if (Table->hasExists == 0)
    CreateExists();
  Table->hasExists = 1;
  Table->extExists = ExtState;
  GrantState = stateExistsProcGrant;
  ProjectState = stateExistsProcProject;
}
| sqldefINSERT NoExt
{
  if (Table->hasInsert == 0)
  {
    if (Table->hasLOB == 1 || Table->Version2Bin == 1)
      CreateInsertUsingReturning();
    else
      CreateInsert();
  }
  Table->hasInsert = 1;
  Table->extInsert = ExtState;
  GrantState = stateInsertProcGrant;
  ProjectState = stateInsertProcProject;
}
| sqldefSELECTALL NoExt
{
  if (Table->hasSelectAll == 0)
    CreateSelectAll(0);
  Table->hasSelectAll = 1;
  Table->extSelectAll = ExtState;
  GrantState = stateSelectAllProcGrant;
  ProjectState = stateSelectAllProcProject;
}
| sqldefSELECTONE NoExt
{
  if (!Table->hasPrimaryKey)
    yyreport(CODING_ERROR, "No primary key for SelectOne\n");
  if (Table->hasSelectOne == 0)
    CreateSelectOne(0);
  Table->hasSelectOne = 1;
  Table->extSelectOne = ExtState;
  GrantState = stateSelectOneProcGrant;
  ProjectState = stateSelectOneProcProject;
}
| sqldefSELECTALLUPD NoExt
{
  if (Table->hasSelectAllUpd == 0)
    CreateSelectAll(1);
  Table->hasSelectAllUpd = 1;
  Table->extSelectAllUpd = ExtState;
  GrantState = stateSelectAllUpdProcGrant;
  ProjectState = stateSelectAllUpdProcProject;
}
| sqldefSELECTONEUPD NoExt
{
  if (!Table->hasPrimaryKey)
    yyreport(CODING_ERROR, "No primary key for SelectOne\n");
  if (Table->hasSelectOneUpd == 0)
    CreateSelectOne(1);
  Table->hasSelectOneUpd = 1;
  Table->extSelectOneUpd = ExtState;
  GrantState = stateSelectOneUpdProcGrant;
  ProjectState = stateSelectOneUpdProcProject;
}
| sqldefSELECTPLUS OptPlusNumber NoExt
{
  if (Table->hasSelectPlus == 0)
    CreateSelectPlus($2);
  Table->hasSelectPlus = 1;
  Table->extSelectPlus = ExtState;
  GrantState = stateSelectPlusProcGrant;
  ProjectState = stateSelectPlusProcProject;
}
| sqldefUPDATE NoExt
{
  if (!Table->hasPrimaryKey)
    yyreport(CODING_ERROR, "No primary key for Update\n");
  if (Table->hasUpdate == 0)
  {
    if (Table->hasLOB == 1 || Table->Version2Bin == 1)
      CreateUpdateUsingReturning();
    else
      CreateUpdate();
  }
  Table->hasUpdate = 1;
  Table->extUpdate = ExtState;
  GrantState = stateUpdateProcGrant;
  ProjectState = stateUpdateProcProject;
};

OptPlusNumber
:
{
  $$ = 128;
}
| Number
{
  $$ = $1;
};

ViewDef
: sqldefVIEW ViewName SetViewGrant ToUsers ViewCode
{
  AddView();
};

SetViewGrant :
{
  GrantState = stateViewGrant;
};

ViewName
: sqldefIDENT
{
  SetView($1);
};

ViewCode
: sqldefCODE SetView Lines sqldefENDCODE
;

SetView :
{
  LineState = stateView;
};

UserProc
: ProcName NoExt ToUsers UsedBy Input InOut Output Declares Code
{
  Proc.extProc = ExtState;
  AddProc();
};

NoExt
: '(' NoNos ')'
| ProcSwitches
;

NoNos
: NoNo
| NoNos NoNo
;

NoNo
: sqldefNOC
{
  ExtState |= noC;
}
| sqldefNOPAS
{
  ExtState |= noPAS;
}
| sqldefNOVB
{
  ExtState |= noVB;
}
| sqldefNOEXT
{
  ExtState |= noEXT;
}
| sqldefNOIDL
{
  ExtState |= noIDL;
};

ProcSwitches
:
| ProcSwitches ProcSwitch
;

ProcSwitch
: switchEXTENDEDC       SwitchNumber
{
  if ($2 != 0) {ExtState |= doEXTC; Table->MarkedExtC = 1;}
  else          ExtState |= noEXTC;
}
| switchEXTENDEDPAS     SwitchNumber
{
  if ($2 != 0) {ExtState |= doEXTPAS; Table->MarkedExtPAS = 1;}
  else          ExtState |= noEXTPAS;
}
| switchEXTENDEDVB      SwitchNumber
{
  if ($2 != 0) {ExtState |= doEXTVB; Table->MarkedExtVB = 1;}
  else          ExtState |= noEXTVB;
}
| switchTARGETC         SwitchNumber
{
  if ($2 != 0) {ExtState |= doC; Table->MarkedC = 1;}
  else          ExtState |= noC;
}
| switchTARGETIDL       SwitchNumber
{
  if ($2 != 0) {ExtState |= doIDL; Table->MarkedIDL = 1;}
  else          ExtState |= noIDL;
}
| switchTARGETPAS       SwitchNumber
{
  if ($2 != 0) {ExtState |= doPAS; Table->MarkedPAS = 1;}
  else          ExtState |= noPAS;
}
| switchTARGETPYTHON    SwitchNumber
{
  if ($2 != 0) {ExtState |= doPYTHON; Table->MarkedPython = 1;}
  else          ExtState |= noPYTHON;
}
| switchTARGETVB        SwitchNumber
{
  if ($2 != 0) {ExtState |= doVB; Table->MarkedVB = 1;}
  else          ExtState |= noVB;
}
| switchTARGETVB5       SwitchNumber
{
  if ($2 != 0) {ExtState |= doVB5; Table->MarkedVB5 = 1;}
  else          ExtState |= noVB5;
}
| switchTARGETVBCODE3   SwitchNumber
{
  if ($2 != 0) {ExtState |= doVBCODE3; Table->MarkedVBCode3 = 1;}
  else          ExtState |= noVBCODE3;
}
| switchTARGETVBNET7    SwitchNumber
{
  if ($2 != 0) {ExtState |= doVBNET7; Table->MarkedVBNet7 = 1;}
  else          ExtState |= noVBNET7;
}
| switchTARGETCSNET7    SwitchNumber
{
  if ($2 != 0) {ExtState |= doCSNET7; Table->MarkedCSNet7 = 1;}
  else          ExtState |= noCSNET7;
}
| switchTARGETCSADONET  SwitchNumber
{
  if ($2 != 0) {ExtState |= doCSADONET; Table->MarkedCSAdoNet = 1;}
  else          ExtState |= noCSADONET;
}
| switchTARGETCSRW  SwitchNumber
{
  if ($2 != 0) {ExtState |= doCSRW; Table->MarkedCSRW = 1;}
  else          ExtState |= noCSRW;
}
| switchTARGETCSIDL2    SwitchNumber
{
  if ($2 != 0) {ExtState |= doCSIDL2; Table->MarkedCSIDL2 = 1;}
  else          ExtState |= noCSIDL2;
}
| switchTARGETVBFORIDL  SwitchNumber
{
  if ($2 != 0) {ExtState |= doVBFORIDL; Table->MarkedVBforIDL = 1;}
  else          ExtState |= noVBFORIDL;
}
| switchTARGETVBFORADOR SwitchNumber
{
  if ($2 != 0) {ExtState |= doVBFORADOR; Table->MarkedVBforADOR = 1;}
  else          ExtState |= noVBFORADOR;
}
| switchINVALID
{
  yyreport(CODING_ERROR, "%s is not a valid switch\n", $1);
  free($1);
}
;

UsedBy
:
| sqldefUSEDBY Projects
;

Projects
: Project
| Projects Project
;

Project
: AnIdent
{
  AddProject($1);
};

ProcName
: sqldefIDENT
{
  SetProc($1);
};

ToUsers
:
| sqldefGRANT sqldefTO Users
;

Input
:
| sqldefINPUT SetStateInput FieldDef
;

SetStateInput
:
{
  FieldState = stateInput;
};

InOut
:
| sqldefINOUT SetStateInOut FieldDef
;

SetStateInOut
:
{
  FieldState = stateInOut;
};

Output
:
| sqldefOUTPUT SetStateOutput OptMultiSingle ReUse
;

ReUse
: '*'
{
  UseStdRecOutput();
}
| FieldDef
;

OptMultiSingle
:
{
  Proc.isMultiFetch = 2;
}
| '(' sqldefMULTIPLE ')'
{
  Proc.isMultiFetch = 1;
}
| '(' sqldefSINGLE   ')'
{
  Proc.isFetch = 1;
};

SetStateOutput
:
{
  FieldState = stateOutput;
};

Declares
:
| Declares Declare
;

Declare
: sqldefDECLARE SetStateDeclare DeclareFieldDef
| sqldefCURSOR SetCursor Lines sqldefENDCURSOR
{
  AddList(Proc.Cursors, TYYCursor, Cursor, Proc.noCursors, 8);
};

DeclareFieldDef
:
| DeclareFieldDef DeclareField
;

DeclareField
: Field
| AnIdent SetField sqldefEXCEPTION
{
  SetFieldType(ftypeException, 0);
  AddField($1);
};

SetCursor
:
{
  memset(&Cursor, 0, sizeof(TYYCursor));
  LineState = stateCursor;
};

SetStateDeclare
:
{
  FieldState = stateWork;
};

Code
: sqldefSQL SetSql sqldefCODE SetCode Lines sqldefENDCODE
| sqldefSQLCODE SetSql SetCode Lines sqldefENDCODE
| sqldefCODE SetCode Lines sqldefENDCODE
;

SetCode
:
{
  LineState = stateCode;
};

SetSql
:
{
  Proc.isSql = 1;
  if (Proc.noWorkFields)
    yyreport(CODING_ERROR, "%s SQL code should not have DECLARE work fields\n", Proc.Name);
};

Lines
: Line
| Lines Line
;

Line
: sqldefLINE
{
  AddLine($1);
  if (LineState == stateCode)
    AddLine(strdup("\n"));
};

Data
: sqldefDATA SetData Lines sqldefENDDATA
{
  Table->noDatas++;
  AddProc();
};

SetData
:
{
  SetProc(0);
  LineState = stateData;
  Proc.isData = 1;
};

%%
static char* InBuf;
static int InBufSoFar;

int yylex()
{
  return inbuf_lex(InBuf, InBufSoFar);
}

int inbuf_parse(PYYTable aTable, char* aInBuf)
{
  Table = aTable;
  InBuf = aInBuf;
  InBufSoFar = 0;
  return yyparse();
}

static void SetSwitch(int &Switch, int value)
{
  Switch = value;
}

static void SetSwitch(char *&Switch, char *value)
{
  if (Switch) free(Switch);
  Switch = strdup(value);
}

static char *Concat(char *s1, char *s2)
{
  char *s = (char*) calloc(strlen(s1)+strlen(s2)+2, 1);
  strcpy(s, s1);
  strcat(s, ".");
  strcat(s, s2);
  free(s1);
  free(s2);
  return s;
}

static char *Combine(char *s1, char *s2)
{
  char *s = (char*) calloc(strlen(s1)+strlen(s2)+2, 1);
  strcpy(s, s1);
  strcat(s, s2);
  free(s1);
  free(s2);
  return s;
}

static int32 Hash(char *s, int m)
{
  int i;
  int32 result = 0;
  for (i=0; i<strlen(s); i++)
    result += (s[i] * m * (i+1));
  return result;
}

static int32  FieldPadding()
{
  if (Field.Padding == 1
  && Field.Type != ftypeTinyint
  && NullableField(&Field))
    Field.Padding = Table->EightByte==1 ? 8 : 2;
  return Field.Padding;
}

static void AddTableField(char* s)
{
  int32  n;
  Field.Name = s;
  if ((n = Table->Length % FieldPadding()) != 0)
    Field.Filler = FieldPadding() - n;
  else
    Field.Filler = 0;
  Table->Length += Field.Filler;
  Field.Offset = Table->Length;
  Table->Length += Field.Length;
  if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
    Table->Length += 2;
  Table->Token  += Hash(Field.Name, 23) * (Field.Offset+1) * Field.Length;
  Table->ABSize += strlen(Field.Name)+8;
  AddList(Table->Fields, TYYField, Field, Table->noFields, 8);
}

static void AddRowUser(void)
{
  memset(&Field, 0, sizeof(TYYField));
  SetFieldType(ftypeUserStamp, 0);
  AddTableField(strdup("RowUser"));
}

static void AddRowStatus(void)
{
  memset(&Field, 0, sizeof(TYYField));
  SetFieldType(ftypeStatus, 0);
  AddTableField(strdup("RowStatus"));
}

static void AddRowUpdated(void)
{
  memset(&Field, 0, sizeof(TYYField));
  SetFieldType(ftypeTimeStamp, 0);
  AddTableField(strdup("RowUpdated"));
}

static void AddRowSeq(void)
{
  Field.Alias = 0;
  Field.Type = ftypeFloat;
  Field.Length = sizeof(double);
  Field.Padding = sizeof(double);
  Field.Precision = 15;
  Field.Scale = 0;
  Field.isPrimaryKey = 0;
  Field.isNull = 0;
  Field.isInput = 0;
  Field.isBound = 0;
  Field.isOutput = 0;
  Field.isWork = 0;
  Field.isRowSequence = 1;
  AddTableField(strdup("RowSeq"));
}

static void AddInputField(char* s)
{
  int32  n;
  Field.Name = s;
  //Field.Alias = 0;
  if ((n = Proc.Length % FieldPadding()) != 0)
    Field.Filler = FieldPadding() - n;
  else
    Field.Filler = 0;
  Proc.Length += Field.Filler;
  Field.Offset = Proc.Length;
  Field.isInput = 1;
  Proc.Length += Field.Length;
  if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
    Proc.Length += 2;
  Proc.ABSize += (strlen(Field.Name)+8);
  AddList(Proc.Fields, TYYField, Field, Proc.noFields, 8);
}

static void AddOutputField(char* s)
{
  int32  FldPos;
  int32  n;
  for (FldPos=0; FldPos<Proc.noFields; FldPos++)
  {
    if ((stricmp(Proc.Fields[FldPos].Name, s))==0)
      break;
  }
  if (FldPos < Proc.noFields)
  {
    Proc.Fields[FldPos].isOutput = 1;
    free(s);
  }
  else
  {
    Field.Name = s;
    if ((n = Proc.Length % FieldPadding()) != 0)
      Field.Filler = FieldPadding() - n;
    else
      Field.Filler = 0;
    Proc.Length += Field.Filler;
    Field.Offset = Proc.Length;
    Field.isOutput = 1;
    Proc.Length += Field.Length;
    if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
      Proc.Length += 2;
    Proc.ABSize += (strlen(Field.Name)+8);
    AddList(Proc.Fields, TYYField, Field, Proc.noFields, 8);
  }
  AddList(Proc.Binds, int32 , FldPos, Proc.noBinds, 16);
  Proc.noOutputs++;
}

static void AddInOutField(char* s)
{
  int32  n;
  Field.Name = s;
  if ((n = Proc.Length % FieldPadding()) != 0)
    Field.Filler = FieldPadding() - n;
  else
    Field.Filler = 0;
  Proc.Length += Field.Filler;
  Field.Offset = Proc.Length;
  Field.isInput = 1;
  Field.isOutput = 1;
  Proc.Length += Field.Length;
  if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
    Proc.Length += 2;
  Proc.ABSize += (strlen(Field.Name)+8);
  AddList(Proc.Fields, TYYField, Field, Proc.noFields, 8);
}

static void AddStdOutputField(char* s)
{
  int32  FldPos;
  int32  n;
  if ((n = Proc.Length % FieldPadding()) != 0)
    Field.Filler = FieldPadding() - n;
  else
    Field.Filler = 0;
  Proc.Length += Field.Filler;
  for (FldPos=0; FldPos<Proc.noFields; FldPos++)
  {
    if ((stricmp(Proc.Fields[FldPos].Name, s))==0)
      break;
  }
  if (FldPos < Proc.noFields)
  {
    Proc.Length += Field.Length;
    if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
      Proc.Length += 2;
    Proc.Fields[FldPos].isOutput = 1;
    free(s);
  }
  else
  {
    Field.Name = s;
    Field.Offset = Proc.Length;
    Field.isOutput = 1;
    Proc.Length += Field.Length;
    if (Table->isNullEnabled && Field.isNull && NullableField(&Field))
      Proc.Length += 2;
    Proc.ABSize += (strlen(Field.Name)+8);
    AddList(Proc.Fields, TYYField, Field, Proc.noFields, 8);
  }
  AddList(Proc.Binds, int32 , FldPos, Proc.noBinds, 16);
  Proc.noOutputs++;
}

static void UseStdRecOutput()
{
  int t, p, useStd = 1;
  for (p=0; p<Proc.noFields; p++)
  {
    for (t=0; t<Table->noFields; t++)
    {
      if ((stricmp(Proc.Fields[p].Name, Table->Fields[t].Name)) == 0
      && Proc.Fields[p].Type == Table->Fields[t].Type
      && Proc.Fields[p].Length == Table->Fields[t].Length)
        break;
    }
    if (t >= Table->noFields)
      useStd = 0;
  }
  Proc.useStd = useStd;
  if (useStd == 1)
  {
    Proc.Length = 0;
    for (t=0; t<Table->noFields; t++)
    {
      for (p=0; p<Proc.noFields; p++)
      {
        if ((stricmp(Proc.Fields[p].Name, Table->Fields[t].Name)) == 0)
        {
          Proc.Fields[p].Offset  = Table->Fields[t].Offset;
          Proc.Fields[p].Filler  = Table->Fields[t].Filler;
          Proc.Fields[p].Padding = Table->Fields[t].Padding;
        }
      }
    }
  }
  for (t=0; t<Table->noFields; t++)
  {
    Field = Table->Fields[t];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    if (useStd == 1)
      AddStdOutputField(strdup(Field.Name));
    else
      AddOutputField(strdup(Field.Name));
  }
}

static void AddDynamicField(char* s)
{
  int i;
  int32  n;
  Field.Name = s;
  Field.Alias = 0;
  for (i=0; i<Proc.noFields; i++)
  {
    if ((strcmp(Proc.Fields[i].Name, Field.Name))==0) // see sqlapi.c
    {                                                 // using exact match
      if (Proc.Fields[i].Type == ftypeDynamic)
      {
        free(s);
        return;
      }
      yyreport(CODING_ERROR, "Dynamic field already used as non dynamic\n");
    }
  }
  if ((n = Proc.Length % FieldPadding()) != 0)
    Field.Filler = FieldPadding() - n;
  else
    Field.Filler = 0;
  Field.isInput = 0;
  Field.isOutput = 0;
  Proc.Length += Field.Filler;
  Field.Offset = Proc.Length;
  Proc.Length += Field.Length;
  Proc.ABSize += (strlen(Field.Name)+8);
  AddList(Proc.Binds, int32 , Proc.noFields, Proc.noBinds, 16);
  AddList(Proc.Fields, TYYField, Field, Proc.noFields, 8);
}

static void AddWorkField(char* s)
{
  Field.Name = s;
  Field.isWork = 1;
  Proc.ABSize += (strlen(Field.Name)+8);
  AddList(Proc.WorkFields, TYYField, Field, Proc.noWorkFields, 4);
}

static void AddConst(char* s, int i)
{
  Const.Name = s;
  Const.Value = i;
  Const.Type = 0;
  AddList(Field.Consts, TYYConst, Const, Field.noConsts, 8);
}

static void AddCharConst(char* s, int i)
{
  Const.Name = s;
  Const.Value = i;
  Const.Type = 1;
  AddList(Field.Consts, TYYConst, Const, Field.noConsts, 8);
}

static void AddStringConst(char* s, char* d)
{
  int i = atoi(d);
  free(d);
  AddConst(s, i);
}

static void AddConstIdent(char* s)
{
  Const.Name = s;
  Const.Value = Field.noConsts;
  Const.Type = 0;
  AddList(Field.Consts, TYYConst, Const, Field.noConsts, 8);
}

static void AddField(char* s)
{
  switch (FieldState)
  {
  case stateTable:
    AddTableField(s);
    break;
  case stateInput:
    AddInputField(s);
    break;
  case stateInOut:
    AddInOutField(s);
    break;
  case stateOutput:
    AddOutputField(s);
    break;
  case stateWork:
    AddWorkField(s);
    break;
  }
}

static void SetFieldType(eFieldType Type, int Length)
{
  Field.Type = Type;
  Field.Padding = 1;
  switch (Type)
  {
  case ftypeChar:
    Field.Scale = 0;
    Field.Precision = Length;
    Field.Length = Length + 1;
    break;
  case ftypeBinary:
    Field.Scale = 0;
    Field.Precision = Length;
    Field.Length = Length + 1;
    break;
  case ftypeStatus:
    Field.Scale = 0;
    Field.Precision = 1;
    Field.Length = 2;
    break;
  case ftypeDate:
    Field.Scale = 0;
    Field.Length = 9;
    Field.Precision = 8;
    break;
  case ftypeTimeStamp:
    Field.Scale = 0;
    Field.Length = 15;
    Field.Precision = 14;
    break;
  case ftypeDateTime:
    Field.Scale = 0;
    Field.Length = 15;
    Field.Precision = 14;
    break;
  case ftypeUserStamp:
    Field.Scale = 0;
    Field.Length = 9;
    Field.Precision = 8;
    break;
  case ftypeBoolean:
    Field.Scale = 0;
    Field.Length = sizeof(char);
    Field.Precision = 1;
    break;
  case ftypeTinyint:
    Field.Scale = 0;
    Field.Length = 1;
    Field.Precision = 3;
    break;
  case ftypeSmallint:
    Field.Scale = 0;
    Field.Padding = Table->EightByte==1 ? 8 : 2;
    Field.Length = 2;
    Field.Precision = 5;
    break;
  case ftypeInt:
    Field.Scale = 0;
    Field.Padding = Table->EightByte==1 ? 8 : 4;
    Field.Length = 4;
    Field.Precision = 10;
    break;
  case ftypeLong:
    Field.Scale = 0;
    Field.Padding = 8;
    Field.Length = 8;
    Field.Precision = 19;
    break;
  case ftypeFloat:
    if (Field.Precision <= 15)
    {
      Field.Padding = sizeof(double);
      Field.Length = sizeof(double);
      break;
    }
    Field.Type = ftypeMoney;
  case ftypeMoney:
    Field.Length = Field.Precision + 3;
    break;
  case ftypeVarNum:
    Field.Length = 22;
    break;
  case ftypeBLOB:
  case ftypeCLOB:
    if (Length >= 65532 && Table->Version2Bin == 0)
      Table->Version2Bin = 1;
    Field.Padding = Table->EightByte==1 ? 8 : sizeof(int);
    Field.Length = Length+4;
    Field.Precision = Length+4;
    break;
  case ftypeXMLTYPE:
    if (Length >= 65532 && Table->Version2Bin == 0)
      Table->Version2Bin = 1;
    Field.Padding = Table->EightByte==1 ? 8 : sizeof(int);
    Field.Length = Length+4;
    Field.Precision = Length+4;
    break;
  case ftypeZLOB:
    if (Length >= 65528 && Table->Version2Bin == 0)
      Table->Version2Bin = 1;
    Field.Padding = Table->EightByte==1 ? 8 : sizeof(int);
    Field.Length = Length+8;
    Field.Precision = Length+8;
    break;
  case ftypeImage:
    if (Length == 1)
    {
      Field.Type = ftypeImagePtr;
      Field.Padding = Table->EightByte==1 ? 8 : sizeof(void*);
      Field.Length = sizeof(void*);
    }
    else
    {
      Field.Padding = Table->EightByte==1 ? 8 : sizeof(short);
      Field.Length = Length+2;
      Field.Precision = Length+2;
    }
    break;
  case ftypeHugeCHAR:
    Field.Length = 8;
    Field.Padding = 8;
    Field.Precision = 8;
    break;
  case ftypeException:
    Field.Length = 0;
    break;
  case ftypeDynamic:
    Field.Length = Length;
    break;
  }
}

static void SetFromRef(PYYField RefField)
{
  if (RefField)
  {
    if (RefField->Alias && !Field.Alias) Field.Alias = strdup(RefField->Alias);
    Field.Type = RefField->Type;
    Field.Scale = RefField->Scale;
    Field.Precision = RefField->Precision;
    Field.Length = RefField->Length;
    Field.Padding = RefField->Padding;
  }
  else
    yyreport(CODING_ERROR, "Reference Field %s not found\n", RefField);
}

static void SetSameFromTable()
{
  SetFromRef(FindField(Field.Name));
}

static void SetFromTable(char *s)
{
  SetFromRef(FindField(s));
  free(s);
}

static void AddPermission(char* s)
{
  Perm.p = s;
  AddList(Grant.Perms, TYYLine, Perm, Grant.noPerms, 4);
}

static void AddUser(char* s)
{
  User.p = s;
  switch(GrantState)
  {
  case stateTableGrant:
    AddList(Grant.Users, TYYLine, User, Grant.noUsers, 4);
    if (Table->hasExecute)
    {
       User.p = strdup(s);
       AddList(Table->ExecuteUsers, TYYLine, User, Table->noExecuteUsers, 4);
    }
    if (Table->hasSelect)
    {
       User.p = strdup(s);
       AddList(Table->SelectUsers, TYYLine, User, Table->noSelectUsers, 4);
    }
    break;
  case stateProcGrant:
    AddList(Proc.Users, TYYLine, User, Proc.noUsers, 4);
    break;
  case stateViewGrant:
    AddList(View.Users, TYYLine, User, View.noUsers, 4);
    break;
  case stateCountProcGrant:
    AddList(Table->CountUsers, TYYLine, User, Table->noCountUsers, 4);
    break;
  case stateDeleteAllProcGrant:
    AddList(Table->DeleteAllUsers, TYYLine, User, Table->noDeleteAllUsers, 4);
    break;
  case stateDeleteOneProcGrant:
    AddList(Table->DeleteOneUsers, TYYLine, User, Table->noDeleteOneUsers, 4);
    break;
  case stateExistsProcGrant:
    AddList(Table->ExistsUsers, TYYLine, User, Table->noExistsUsers, 4);
    break;
  case stateInsertProcGrant:
    AddList(Table->InsertUsers, TYYLine, User, Table->noInsertUsers, 4);
    break;
  case stateSelectAllProcGrant:
    AddList(Table->SelectAllUsers, TYYLine, User, Table->noSelectAllUsers, 4);
    break;
  case stateSelectOneProcGrant:
    AddList(Table->SelectOneUsers, TYYLine, User, Table->noSelectOneUsers, 4);
    break;
  case stateSelectAllUpdProcGrant:
    AddList(Table->SelectAllUpdUsers, TYYLine, User, Table->noSelectAllUpdUsers, 4);
    break;
  case stateSelectOneUpdProcGrant:
    AddList(Table->SelectOneUpdUsers, TYYLine, User, Table->noSelectOneUpdUsers, 4);
    break;
  case stateSelectPlusProcGrant:
    AddList(Table->SelectPlusUsers, TYYLine, User, Table->noSelectPlusUsers, 4);
    break;
  case stateUpdateProcGrant:
    AddList(Table->UpdateUsers, TYYLine, User, Table->noUpdateUsers, 4);
    break;
  }
}

static void AddProject(char* s)
{
  User.p = s;
  switch(ProjectState)
  {
  case stateProcProject:
    AddList(Proc.Projects, TYYLine, User, Proc.noProjects, 4);
    break;
  case stateCountProcProject:
    AddList(Table->CountProjects, TYYLine, User, Table->noCountProjects, 4);
    break;
  case stateDeleteAllProcProject:
    AddList(Table->DeleteAllProjects, TYYLine, User, Table->noDeleteAllProjects, 4);
    break;
  case stateDeleteOneProcProject:
    AddList(Table->DeleteOneProjects, TYYLine, User, Table->noDeleteOneProjects, 4);
    break;
  case stateExistsProcProject:
    AddList(Table->ExistsProjects, TYYLine, User, Table->noExistsProjects, 4);
    break;
  case stateInsertProcProject:
    AddList(Table->InsertProjects, TYYLine, User, Table->noInsertProjects, 4);
    break;
  case stateSelectAllProcProject:
    AddList(Table->SelectAllProjects, TYYLine, User, Table->noSelectAllProjects, 4);
    break;
  case stateSelectOneProcProject:
    AddList(Table->SelectOneProjects, TYYLine, User, Table->noSelectOneProjects, 4);
    break;
  case stateSelectAllUpdProcProject:
    AddList(Table->SelectAllUpdProjects, TYYLine, User, Table->noSelectAllUpdProjects, 4);
    break;
  case stateSelectOneUpdProcProject:
    AddList(Table->SelectOneUpdProjects, TYYLine, User, Table->noSelectOneUpdProjects, 4);
    break;
  case stateSelectPlusProcProject:
    AddList(Table->SelectPlusProjects, TYYLine, User, Table->noSelectPlusProjects, 4);
    break;
  case stateUpdateProcProject:
    AddList(Table->UpdateProjects, TYYLine, User, Table->noUpdateProjects, 4);
    break;
  }
}

static void AddGrant(void)
{
  AddList(Table->Grants, TYYGrant, Grant, Table->noGrants, 4);
}

static void SetKey(char* s)
{
  memset(&Key, 0, sizeof(TYYKey));
  Key.Name = s;
}

static void AddKey(void)
{
  AddList(Table->Keys, TYYKey, Key, Table->noKeys, 4);
  if (Key.isPrimary)
  {
    Table->PrimaryKeyFields++;
    Table->PrimaryKey = Table->noKeys-1;
  }
}

static void SetLink(char* s)
{
  Link.MasterName = s;
  ColumnState = stateLink;
}

static void DeleteCascadeLink(bool val)
{
  Link.DeleteCascade = val;
}

static void AddLink(void)
{
  AddList(Table->Links, TYYLink, Link, Table->noLinks, 4);
}

static void AddKeyColumn(char* s)
{
  /* const int32  DELTA = 4; */
  short n;
  PYYField Field = FindField(s);
  if (!Field)
    yyreport(CODING_ERROR, "Key Field %s not found\n", s);
  else
  {
    TYYField Copy = *Field;
    if (Copy.Alias)
      Copy.Alias = strdup(Copy.Alias);
    if ((n = Key.Length % Copy.Padding) != 0)
      Copy.Filler = Copy.Padding - n;
    else
      Copy.Filler = 0;
    Key.Length += Copy.Filler;
    Copy.Offset = Key.Length;
    AddList(Key.Fields, TYYField, Copy, Key.noFields, 4);
    Key.Length += Copy.Length;
    Key.ABSize += (strlen(Field->Name)+8);
    if (Key.isPrimary)
      Field->isPrimaryKey = 1;
    if (Field->isNull == 1 && (Key.isPrimary || Key.isUnique))
      Field->isNull = 0;
  }
  free(s);
}

static void AddLinkColumn(char* s)
{
  PYYField Field = FindField(s);
  if (!Field)
    yyreport(CODING_ERROR, "Key Field %s not found\n", s);
  else
  {
    AddList(Link.Fields, TYYField, *Field, Link.noFields, 4);
  }
  free(s);
}

static void AddColumn(char* s)
{
  switch(ColumnState)
  {
  case stateKey:
    AddKeyColumn(s);
    break;
  case stateLink:
    AddLinkColumn(s);
    break;
  }
}

static PYYField FindField(char* s)
{
  int32  i;
  for (i=0; i<Table->noFields; i++)
    if (stricmp(s, Table->Fields[i].Name) == 0)
      return &Table->Fields[i];
  return 0;
}

static void SetFlag(char *s)
{
  if (stricmp(s, "VB5IO") == 0)
    Table->hasVB5IO = 1;
  free(s);
}

static void SetProc(char *s)
{
  memset(&Proc, 0, sizeof(TYYProc));
  Proc.Name = s;
}

static void SetView(char *s)
{
  memset(&View, 0, sizeof(TYYView));
  View.Name = s;
}

static void AddProc(void)
{
  Proc.extProc = ExtState;
  if (Proc.isMultiFetch && !Proc.isSql)
  {
    if (Proc.isMultiFetch == 1)
      yyreport(CODING_ERROR, "Only SQL CODE procs may use (multiple)\n");
    Proc.isMultiFetch = 0;
  }
  AddList(Table->Procs, TYYProc, Proc, Table->noProcs, 4);
}

static void AddView(void)
{
  AddList(Table->Views, TYYView, View, Table->noViews, 4);
}

static void QuestionThis(char *s)
{
  int32  i;

  for (i=0; i<Proc.noFields; i++)
    if (stricmp(s+1, Proc.Fields[i].Name) == 0)
      break;
  if (i >= Proc.noFields)
    yyreport(CODING_ERROR, "Proc :Field %s not found\n", s+1);
  else if (!Proc.isSql)
  {
    if (Table->UnderScore)
    {
      char *w = (char *) malloc(strlen(s)+2);
      strcpy(w, "a_");
      strcat(w, s+1);
      Line.p = w;
      free(s);
    }
    else
    {
      *s = 'a';
      Line.p = s;
    }
    AddList(Proc.Lines, TYYLine, Line, Proc.noLines, 4);
  }
  else
  {
    Line.p = s;
    AddList(Proc.Lines, TYYLine, Line, Proc.noLines, 4);
    if (!Proc.Fields[i].isBound)
      AddList(Proc.Binds, int32 , i, Proc.noBinds, 16);
    Proc.Fields[i].isBound = 1;
  }
}

static void Questions(char *s)
{
  int i, n, len;
  pchar t = s;
  pchar word;
  char w[128];
  while (t[0])
  {
    t = SqlToken(t, w, sizeof(w));
    switch(w[0])
    {
    case ':':
      if (strlen(w) == 1)
      {
        if (t[0] == ':')
          strcpy(t, t+1);
        break;
      }
      t[-strlen(w)] = 0;
      Line.p = strdup(s);
      AddList(Proc.Lines, TYYLine, Line, Proc.noLines, 4);
      word = strdup(w);
      QuestionThis(word);
      s = t;
      break;
    case '$':
      len = 255;
      if (strlen(w) == 1)
      {
        if (t[0] == '$')
          strcpy(t, t+1);
        break;
      }
      if (t[0] == '(')
      {
        for (i = 0; t[i] ; i++)
        {
          if (t[i] == ')')
          {
            for (n = 1; n < i; n++)
              if (isdigit(t[n]))
                break;
            if (n == i)
              break;
            t[i] = 0;
            len = atoi(t+n);
            t[i] = ' ';
            strcpy(t, t+i);
            break;
          }
        }
      }
      word = strdup(w+1);
      SetFieldType(ftypeDynamic, len);
      AddDynamicField(word);
      break;
    }
  }
  Line.p = strdup(s);
  Line.isEOL = 1;
  AddList(Proc.Lines, TYYLine, Line, Proc.noLines, 4);
  Line.isEOL = 0;
}

static void AddLine(char *s)
{                               
  Line.p = s;
  switch(LineState)
  {
  case stateCode:
    Questions(s);
    free(s);
    break;
  case stateData:
    AddList(Proc.Lines, TYYLine, Line, Proc.noLines, 4);
    break;
  case stateView:
    AddList(View.Lines, TYYLine, Line, View.noLines, 4);
    break;
  case stateCursor:
    AddList(Cursor.Lines, TYYLine, Line, Cursor.noLines, 4);
    break;
  }
}

static pchar InputConversion(TYYField Field, int Extended)
{
  // Extended = 0 (Normal) = 1 (Insert) = 2 (Update)
  static char Work[256];
  if (Extended == 1 && Field.isSequence)
  {
    sprintf(Work, "%sSeq.NEXTVAL", Table->Name);
    return Work;
  }
  switch (Field.Type)
  {
  case ftypeDate:
    sprintf(Work, "to_date(:%s, 'YYYYMMDD')", Field.Name);
    return Work;
  case ftypeDateTime:
    sprintf(Work, "to_date(:%s, 'YYYYMMDDHH24MISS')", Field.Name);
    return Work;
  case ftypeTimeStamp:
    if (Extended == 0)
      sprintf(Work, "to_date(:%s, 'YYYYMMDDHH24MISS')", Field.Name);
    else
      sprintf(Work, "SYSDATE");
    return Work;
  case ftypeMoney:
    sprintf(Work, "to_number(:%s)", Field.Name);
    return Work;
  case ftypeUserStamp:
    if (Extended == 0)
      break;
    sprintf(Work, "USER");
    return Work;
  }
  sprintf(Work, ":%s", Field.Name);
  return Work;
}

static pchar OutputConversion(TYYField Field)
{
  static char Work[256];
  switch (Field.Type)
  {
  case ftypeDate:
    sprintf(Work, "to_char(%s, 'YYYYMMDD')", Qualify(Table->Name, Field.Name));
    return Work;
  case ftypeDateTime:
  case ftypeTimeStamp:
    sprintf(Work, "to_char(%s, 'YYYYMMDDHH24MISS')", Qualify(Table->Name, Field.Name));
    return Work;
  case ftypeMoney:
    sprintf(Work, "to_char(%s)", Qualify(Table->Name, Field.Name));
    return Work;
  }
  sprintf(Work, "%s", Field.Name);
  return Work;
}

static void CreateCount(void)
{
  char Work[512];
  SetProc(strdup("Count"));
  memset(&Field, 0, sizeof(TYYField));
  SetFieldType(ftypeInt, 0);
  AddOutputField(strdup("NoOf"));
  Proc.isSql = 1;
  Proc.isFetch = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "select count(*) from %s\n", Table->Name);
  AddLine(strdup(Work));
  AddProc();
}

static void CreateDeleteAll(void)
{
  char Work[512];
  SetProc(strdup("DeleteAll"));
  Proc.isSql = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "delete from %s\n",
          Table->Name);
  AddLine(strdup(Work));
  AddProc();
}

static pchar Qualify(pchar TableName, pchar FieldName)
{
  static char Work[256];
  char Work1[256];
  Work1[0] = ':';
  strncpy(Work1+1, FieldName, sizeof(Work1)-3);
  Work1[sizeof(Work1)-1] = 0;
  strcat(Work1, ":");
  strupr(Work1);
  char* p = strstr((char*)OracleReservedWords, Work1);
  if (p == 0)
  {
    strncpy(Work, FieldName, sizeof(Work)-1);
    Work[sizeof(Work)-1] = 0;
  }
  else
  {
    strncpy(Work, TableName, sizeof(Work)-strlen(FieldName)-2);
    Work[sizeof(Work)-strlen(FieldName)-2] = 0;
    strcat(Work, ".");
    strcat(Work, FieldName);
  }
  return Work;
}

static void CreateDeleteOne(void)
{
  char Work[512];
  int32  i, first=1;
  SetProc(strdup("DeleteOne"));
  Proc.isSql = 1;
  Proc.useKey = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "delete from %s\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.isPrimaryKey)
    {
      if (Field.Alias) Field.Alias = strdup(Field.Alias);
      AddInputField(strdup(Field.Name));
      sprintf(Work, "%s %s = %s\n", first?"where":"and"
         , Qualify(Table->Name, Field.Name), InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  AddProc();
}

static void CreateInsertUsingReturning(void)
{
  char Work[512];
  int32  i, first, hasReturn=0;
  SetProc(strdup("Insert"));
  Proc.isSql = 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];
    Field.noConsts=0;
    if (Field.Alias)
      Field.Alias = strdup(Field.Alias);
    if (Field.Type == ftypeUserStamp
    || Field.Type == ftypeTimeStamp
    || Field.isSequence)
      Field.isOutput = hasReturn = 1;
    AddInputField(strdup(Field.Name));
  }
  sprintf(Work, "insert into %s (\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];
    sprintf(Work, "%s%s\n", first?"":",", Qualify(Table->Name, Field.Name));
    AddLine(strdup(Work));
    first = 0;
  }
  sprintf(Work, ") values (\n");
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];
    sprintf(Work, "%s%s\n", first?"":",", InputConversion(Field, 1));
    AddLine(strdup(Work));
    first = 0;
  }
  if (hasReturn == 1)
  {
    AddLine(strdup(") returning\n"));
    for (i=0, first=1; i<Table->noFields; i++)
    {
      Field = Table->Fields[i];
      if (Field.Type == ftypeUserStamp
      || Field.Type == ftypeTimeStamp
      || Field.isSequence)
      {
        sprintf(Work, "%s%s\n", first?"":",", OutputConversion(Field));
        AddLine(strdup(Work));
        first = 0;
      }
    }
    AddLine(strdup("into\n"));
    for (i=0, first=1; i<Table->noFields; i++)
    {
      Field = Table->Fields[i];
      if (Field.Type == ftypeUserStamp
      || Field.Type == ftypeTimeStamp
      || Field.isSequence)
      {
        sprintf(Work, "%s:%s\n", first?"":",", Qualify(Table->Name, Field.Name));
        AddLine(strdup(Work));
        first = 0;
      }
    }
  }
  else
    AddLine(strdup(")\n"));
  AddProc();
}

static void CreateInsert(void)
{
  char Work[512];
  int32  i, first, done=0;
  SetProc(strdup("Insert"));
  Proc.isSql = 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  AddLine(strdup("BEGIN\n"));
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    AddInputField(strdup(Field.Name));
    done=1;
    if (Field.Type == ftypeUserStamp
     || Field.Type == ftypeTimeStamp
     || Field.isSequence)
    {
      Field.isOutput = 1;
      if (Field.isSequence)
        sprintf(Work, "select %sSeq.NEXTVAL into :%s from dual;\n", Table->Name, Qualify(Table->Name, Field.Name));
      else if (Field.Type == ftypeUserStamp)
        sprintf(Work, "select SUBSTR(USER, 1, 8) into :%s from dual;\n", Qualify(Table->Name, Field.Name));
      else
        sprintf(Work, "select to_char(SYSDATE, 'YYYYMMDDHH24MISS')"
                      " into :%s from dual;\n", Qualify(Table->Name, Field.Name));
      AddLine(strdup(Work));
    }
  }
  sprintf(Work, "insert into %s (\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    sprintf(Work, "%s%s\n", first?"":",", Qualify(Table->Name, Field.Name));
    AddLine(strdup(Work));
    first = 0;
  }
  sprintf(Work, ") values (\n");
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (done == 0)
    {
      if (Field.Alias) Field.Alias = strdup(Field.Alias);
      AddInputField(strdup(Field.Name));
    }
    sprintf(Work, "%s%s\n", first?"":",", InputConversion(Field));
    AddLine(strdup(Work));
    first = 0;
  }
  AddLine(strdup(");\n"));
  AddLine(strdup("END;\n"));
  AddProc();
}

static void CreateSelectAll(int Update)
{
  char Work[512];
  int32  i, first;
  if (Update)
    SetProc(strdup("SelectAllUpd"));
  else
    SetProc(strdup("SelectAll"));
  Proc.isSql = 1;
  Proc.isMultiFetch = (Table->hasImage == 1 || Table->hasLOB == 1) ? 0 : 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "select\n");
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    AddOutputField(strdup(Field.Name));
    sprintf(Work, "%s%s\n", first?"":", " , OutputConversion(Field));
    AddLine(strdup(Work));
    first = 0;
  }
  sprintf(Work, "from %s\n", Table->Name);
  AddLine(strdup(Work));
  if (Update)
  {
    sprintf(Work, "for update\n");
    AddLine(strdup(Work));
  }
  AddProc();
}

static void CreateSelectPlus(int size)
{
  char Work[512];
  int32  i, first;
  SetProc(strdup("SelectPlus"));
  Proc.isSql = 1;
  Proc.isMultiFetch = 1;
  Proc.isStd = 1;
  Proc.hasImage = Table->hasImage;
  Proc.hasLOB = Table->hasLOB;
  LineState = stateCode;
  sprintf(Work, "select\n");
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    AddOutputField(strdup(Field.Name));
    sprintf(Work, "%s%s\n", first?"":", " , OutputConversion(Field));
    AddLine(strdup(Work));
    first = 0;
  }
  sprintf(Work, "from %s $UPlusU\n", Table->Name);
  AddLine(strdup(Work));
  SetFieldType(ftypeDynamic, size < 32 ? 32 : size);
  AddDynamicField(strdup("UPlusU"));
  AddProc();
}

static void CreateSelectOne(int Update)
{
  char Work[512];
  int32  i, first;
  if (Update)
    SetProc(strdup("SelectOneUpd"));
  else
    SetProc(strdup("SelectOne"));
  Proc.isSql = 1;
  Proc.isFetch = 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "select\n");
  AddLine(strdup(Work));
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    if (!Field.isPrimaryKey)
      AddOutputField(strdup(Field.Name));
    else
      AddInputField(strdup(Field.Name));
  }
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (!Field.isPrimaryKey)
    {
      sprintf(Work, "%s%s\n", first?"":", " , OutputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  sprintf(Work, "from %s\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.isPrimaryKey)
    {
      sprintf(Work, "%s %s = %s\n", first?"where":"and"
                  , Qualify(Table->Name, Field.Name), InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  if (Update)
  {
    sprintf(Work, "for update\n");
    AddLine(strdup(Work));
  }
  AddProc();
}

static void CreateUpdateUsingReturning(void)
{
  char Work[512];
  int32  i, first=1, hasReturn=0;
  SetProc(strdup("Update"));
  Proc.isSql = 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];
    Field.noConsts=0;
    if (Field.Alias)
      Field.Alias = strdup(Field.Alias);
    if (Field.Type == ftypeUserStamp
    || Field.Type == ftypeTimeStamp)
      Field.isOutput = hasReturn = 1;
    AddInputField(strdup(Field.Name));
  }
  sprintf(Work, "update %s set\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (!Field.isPrimaryKey)
    {
      sprintf(Work, "%s%s = %s\n", first?"":", ",
                     Qualify(Table->Name, Field.Name), InputConversion(Field, 2));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.isPrimaryKey)
    {
      sprintf(Work, "%s %s = %s\n", first?"where":"and",
                     Qualify(Table->Name, Field.Name), InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  if (hasReturn == 1)
  {
    AddLine(strdup(" returning\n"));
    for (i=0, first=1; i<Table->noFields; i++)
    {
      Field = Table->Fields[i];
      if (Field.Type == ftypeUserStamp
      || Field.Type == ftypeTimeStamp)
      {
        sprintf(Work, "%s%s\n", first?"":",", OutputConversion(Field));
        AddLine(strdup(Work));
        first = 0;
      }
    }
    AddLine(strdup("into\n"));
    for (i=0, first=1; i<Table->noFields; i++)
    {
      Field = Table->Fields[i];
      if (Field.Type == ftypeUserStamp
      || Field.Type == ftypeTimeStamp)
      {
        sprintf(Work, "%s:%s\n", first?"":",", Qualify(Table->Name, Field.Name));
        AddLine(strdup(Work));
        first = 0;
      }
    }
  }
  AddProc();
}

static void CreateUpdate(void)
{
  char Work[512];
  int32  i, first=1, done=0;
  SetProc(strdup("Update"));
  Proc.isSql = 1;
  Proc.useStd = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  AddLine(strdup("BEGIN\n"));
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.Alias) Field.Alias = strdup(Field.Alias);
    AddInputField(strdup(Field.Name));
    done = 1;
    if (Field.Type == ftypeUserStamp
     || Field.Type == ftypeTimeStamp)
    {
      Field.isOutput = 1;
      if (Field.Type == ftypeUserStamp)
        sprintf(Work, "select SUBSTR(USER, 1, 8) into :%s from dual;\n", Qualify(Table->Name, Field.Name));
      else
        sprintf(Work, "select to_char(SYSDATE, 'YYYYMMDDHH24MISS')"
                      " into :%s from dual;\n", Qualify(Table->Name, Field.Name));
      AddLine(strdup(Work));
    }
  }
  if (first == 0)
    AddLine(strdup("from dual;\n"));
  sprintf(Work, "update %s set\n", Table->Name);
  AddLine(strdup(Work));
  if (done == 0)
  {
    for (i=0; i<Table->noFields; i++)
    {
      Field = Table->Fields[i];Field.noConsts=0;
      if (Field.Alias) Field.Alias = strdup(Field.Alias);
      AddInputField(strdup(Field.Name));
    }
  }
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (!Field.isPrimaryKey)
    {
      sprintf(Work, "%s%s = %s\n", first?"":", ",
                     Qualify(Table->Name, Field.Name), InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  for (i=0, first=1; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.isPrimaryKey)
    {
      sprintf(Work, "%s %s = %s\n", first?"where":"and",
                     Qualify(Table->Name, Field.Name), InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  AddLine(strdup(";\nEND;\n"));
  AddProc();
}

static void CreateExists(void)
{
  char Work[512];
  int32  i, first=1;
  SetProc(strdup("Exists"));
  memset(&Field, 0, sizeof(TYYField));
  SetFieldType(ftypeInt, 0);
  AddOutputField(strdup("Count"));
  Proc.isSql = 1;
  Proc.isFetch = 1;
  Proc.isStd = 1;
  LineState = stateCode;
  sprintf(Work, "select count(*) from %s\n", Table->Name);
  AddLine(strdup(Work));
  for (i=0; i<Table->noFields; i++)
  {
    Field = Table->Fields[i];Field.noConsts=0;
    if (Field.isPrimaryKey)
    {
      if (Field.Alias) Field.Alias = strdup(Field.Alias);
      AddInputField(strdup(Field.Name));
      sprintf(Work, "%s %s = %s\n", first?"where":"and"
                  , Qualify(Table->Name, Field.Name)
                  , InputConversion(Field));
      AddLine(strdup(Work));
      first = 0;
    }
  }
  AddProc();
}


