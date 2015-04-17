#ifndef _SQLAPI_H_
#define _SQLAPI_H_ 
#include "machine.h"

#include "sqlbin.h"

extern uint16 ImageMax;

typedef struct SqlCB     TSqlCB,     *PSqlCB;
typedef struct SqlCursor TSqlCursor, *PSqlCursor;

const int32 CBSignature     = 0x0FF0BEAD;
const int32 CursorSignature = 0x0FF0DEAD;

enum ESqlErr
{
  SqlApiOK = 0, SqlApiFirstErr = 8001,    //    0
  SqlApiMemErr = SqlApiFirstErr,          //    8001
  SqlApiBinFileErr,                       //    8002
  SqlApiConnectErr,                       //    8003
  SqlApiNoQueryErr,                       //    8004
  SqlApiOpenErr,                          //    8005
  SqlApiParseErr,                         //    8006
  SqlApiBindErr,                          //    8007
  SqlApiDefineErr,                        //    8008
  SqlApiExecErr,                          //    8009
  SqlApiFetchErr,                         //    8010
  SqlApiCancelErr,                        //    8011
  SqlApiCloseErr,                         //    8012
  SqlApiCommitErr,                        //    8013
  SqlApiRollbackErr,                      //    8014
  SqlApiNoMoreRows,                       //    8015
  SqlApiCBErr,                            //    8016
  SqlApiCursorErr,                        //    8017
  SqlApiStateErr,                         //    8018
  SqlApiAllocEnvErr,                      //    8019
  SqlApiAllocConnectErr,                  //    8020
  SqlApiUnsupportedErr,                   //    8021
  SqlApiNoFldname,                        //    8022
  SqlApiKeyFileErr,                       //    8023
  SqlApiNoMoreCBHandles,                  //    8024
  SqlApiNoMoreCursorHandles,              //    8025
  SqlApiLogoffErr,                        //    8026
  SqlApiLastErr                           // please leave this one as the last
};

pchar GetSqlError(int errNo);

#ifndef __STDC__
#define __STDC__  1
#endif
//typedef void Lda_Def;
#include "oci.h"

#ifdef min
#undef min
#undef max
#endif

struct SqlCB
{
  int32     Signature;
  PSqlBin   SqlBin;
  char      isVB;
  int       Error;
  sword     RC;
  sb4       ociErrCode;
  pchar     ErrorMsg;
  text      *ociBuf;
  OCIEnv    *ociEnv;
  OCIError  *ociError;
  OCISvcCtx *ociSvcCtx;
  int       timeout; // in seconds
};

struct SqlCursor
{
  int32     Signature;
  PSqlCB    SqlCB;
  PSqlQuery SqlQuery;
  int       Error;
  int       NoBinds;
  int       NoDefines;
  int       NoLobLocators;
  int32     DataSize;
  int32     RowsCount;
  int32     RowIndex;
  int32     RowsDone;
  int32     RowsRead;
  sword     RC;
  sb4       ociErrCode;
  pchar     ErrorMsg;
  pchar     Command;
  void      *Data;
  void      *Buffer;
  int16     *Indicators;
  OCIBind   **ociBinds;
  OCIDefine **ociDefines;
  OCILobLocator **ociLobLocators;
  OCIStmt   *ociStmt;
  int32     BufferSize;
};

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#if defined(M_WIN_31) || defined (M_WIN_BC4) || defined (M_W32)
#define SqlLogon              DBLogon
#define SqlVBLogon            DBVBLogon
#define SqlNetLogon           DBNetLogon
#define SqlLogonCopy          DBLogonCopy
#define SqlLogoff             DBLogoff
#define SqlCommit             DBCommit
#define SqlRollback           DBRollback
#define SqlRunProc            DBRunProc
#define SqlOpen               DBOpen
#define SqlOpenWQ             DBOpenWQ
#define SqlOpenExec           DBOpenExec
#define SqlExec               DBExec
#define SqlFetch              DBFetch
#define SqlFetchData          DBFetchData
#define SqlFetchMany          DBFetchMany
#define SqlSetNull            DBSetNull
#define SqlTestNull           DBTestNull
#define SqlCancel             DBCancel
#define SqlClose              DBClose
#define SqlEOD                DBEOD
#define SqlCBEOD              DBCBEOD
#define SqlDuplErr            DBDuplErr
#define SqlCBDuplErr          DBCBDuplErr
#define SqlCBError            DBCBError
#define SqlCursorError        DBCursorError
#define SqlCBErrorCode        DBCBErrorCode
#define SqlCursorErrorCode    DBCursorErrorCode
#define SqlVBCursorError      DBVBCursorError
#define SqlVBCBError          DBVBCBError
#define SqlVBCommand          DBVBCommand
#define SqlNetCursorError     DBNetCursorError
#define SqlNetCBError         DBNetCBError
#define SqlNetCommand         DBNetCommand
#define SqlRowsFetched        DBRowsFetched
#define SqlRowExists          DBRowExists
#define SqlXTAlloc            DBXTalloc
#define SqlOneWayCipher       DBOneWayCipher
#endif

int APPLAPI SqlLogon(PSqlCB *SqlCB, pchar FileName, pchar User);
int APPLAPI SqlLogoff(PSqlCB SqlCB);
int APPLAPI SqlCommit(PSqlCB SqlCB);
int APPLAPI SqlRollback(PSqlCB SqlCB);
int APPLAPI SqlRunProc(PSqlCB SqlCB, pchar Query, void* Data);
int APPLAPI SqlOpenExec(PSqlCB SqlCB, PSqlCursor *SqlCursor, pchar Query, void* Data);
int APPLAPI SqlOpenWQ(PSqlCB SqlCB, PSqlCursor *SqlCursor, PSqlQuery Query, void* Data);
int APPLAPI SqlSetTimeout(PSqlCB SqlCB, int seconds);
bool APPLAPI SqlCBEOD(PSqlCB SqlCB);
bool APPLAPI SqlCBDuplErr(PSqlCB SqlCB);

int APPLAPI SqlOpen(PSqlCB SqlCB, PSqlCursor *SqlCursor, pchar Query, void* Data);
int APPLAPI SqlExec(PSqlCursor SqlCursor);
int APPLAPI SqlFetch(PSqlCursor SqlCursor);

int APPLAPI SqlSetNull(PSqlCursor SqlCursor, pchar aName, bool aHow);
int APPLAPI SqlTestNull(PSqlCursor SqlCursor, pchar aName, bool *aHow);
int APPLAPI SqlFetchMany(PSqlCursor SqlCursor, uint16 *aNo, void *Data);
int APPLAPI SqlCancel(PSqlCursor SqlCursor);
int APPLAPI SqlClose(PSqlCursor SqlCursor);
bool APPLAPI SqlEOD(PSqlCursor SqlCursor);
bool APPLAPI SqlDuplErr(PSqlCursor SqlCursor);

pchar APPLAPI SqlCBError(PSqlCB SqlCB);
pchar APPLAPI SqlCursorError(PSqlCursor SqlCursor);

uint16 APPLAPI SqlCBErrorCode(PSqlCB SqlCB);
uint16 APPLAPI SqlCursorErrorCode(PSqlCursor SqlCursor);

int16 APPLAPI SqlOneWayCipher(pchar Input, uint16 InpLen,
                              pchar Output, uint16 OutLen,
                              uint16 Seed);

#if defined(M_WIN_31) || defined (M_WIN_BC4) || defined (M_W32)
int APPLAPI SqlVBLogon(PSqlCB *SqlCB, pchar FileName, pchar User);
int APPLAPI SqlNetLogon(PSqlCB *SqlCB, pchar FileName, pchar User);
int APPLAPI SqlVBCursorError(PSqlCursor SqlCursor, char * Message, int16 cbMessage);
int APPLAPI SqlVBCommand(PSqlCursor SqlCursor, char * Message, int16 cbMessage);
int APPLAPI SqlVBCBError(PSqlCB SqlCB, char * Message, int16 cbMessage);
int APPLAPI SqlNetCursorError(PSqlCursor SqlCursor, char * Message, int16 cbMessage);
int APPLAPI SqlNetCommand(PSqlCursor SqlCursor, char * Message, int16 cbMessage);
int APPLAPI SqlNetCBError(PSqlCB SqlCB, char * Message, int16 cbMessage);
int APPLAPI SqlXTAlloc(bool aHow);
#endif

#if defined(M_W32)
int APPLAPI SqlFetchData(PSqlCursor SqlCursor, void *Data);
#endif

void After(TSqlCursor *Cursor);
void Before(TSqlCursor *Cursor);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

