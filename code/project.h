#ifndef ProjectH
#define ProjectH
#if !defined(CONSOLE_APP)
#include <synedit.hpp>
#endif

#include "machine.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include "global.h"
#include "binio.h"

enum EFileType
{
  ftFIRST,
  ftSource=ftFIRST,
  ftSQL,
  ftBinary,
  ftView,
  ftLog,
  ftProject,
  ftParm,
  ftIDL,
  ftPython,
  ftLAST
};

class TFile : public TObject
{
protected:
  EFileType  FFileType;
  AnsiString FName;
  AnsiString FExt;
  int        FWritten;
  bool       FLoaded;
  bool       FModified;
  int        FFirstVisibleLine;
  int        Lock;
  HANDLE     FLock;
  bool       FReadOnly;
  EFileType  GetFileType();
public:
  TFile(EFileType AFileType, AnsiString AName);
  virtual __fastcall ~TFile() {}
  void LockFile();
  void UnlockFile();
  bool IsLocked();
  bool Changed();
  bool NewerVersion();
  bool IsSource();
  bool IsBinary();
  bool IsReadOnly();
  __property AnsiString Name =
  {
    read  = FName,
    write = FName
  };
  __property EFileType FileType =
  {
    read  = GetFileType,
    write = FFileType
  };
  __property bool Modified =
  {
    read  = FModified,
    write = FModified
  };
  __property int FirstVisibleLine =
  {
    read  = FFirstVisibleLine,
    write = FFirstVisibleLine
  };
  __property int Written =
  {
    read  = FWritten,
    write = FWritten
  };
  __property bool Locked =
  {
    read = IsLocked
  };
  __property bool ReadOnly =
  {
    read = IsReadOnly
  };
};

#if !defined(CONSOLE_APP)
class TWatchDog;
class TSource; // forward def
class TEditor : public TSynEdit
{
protected:
  AnsiString FFileName;
public:
  TWatchDog* Watcher;
  TSource* Source;
  TEditor(TObject* AOwner, TSource* ASource);
  __property AnsiString FileName =
  {
    read  = FFileName,
    write = FFileName
  };
};
#endif

class TTarget : public TFile
{
public:
  TTarget(EFileType AFileType, AnsiString AName);
  virtual __fastcall ~TTarget() {}
  void Save(FILE* AOutFile);
  virtual void __fastcall Load() {}
};

class TBinField : public TObject
{
  ushort sign;
public:
  AnsiString FName;
  short FCType;
  short FSqlType;
  ulong FSize;
  ulong FOffset;
  ulong FPrecision;
  ulong FScale;
  ushort FIsBind;
  ushort FIsDefine;
  TBinField(FILE *InFile, ushort sign);
  virtual __fastcall ~TBinField();
};

class TBinProc : public TObject
{
  ushort sign;
  ushort pos;
public:
  AnsiString FName;
  TStringList* FCommand;
  TList* FFields;
  TBinProc(FILE *InFile, ushort sign, ushort pos);
  virtual __fastcall ~TBinProc();
};

class TBinary : public TTarget
{
  ushort sign;
public:
  AnsiString FTableName;
  TList* FProcs;
  void __fastcall Load();
  TBinary(AnsiString AName);
  virtual __fastcall ~TBinary();
};

class TProject;
class TSource : public TFile
{
protected:
  TList* FTargets;
public:
  TProject& Project;
  TYYTable Table;
#if !defined(CONSOLE_APP)
  TEditor* Editor;
#endif
  TDate  LastUsed;
  TSource(AnsiString AName, TProject &aProject);
  virtual __fastcall ~TSource()
  {
    Table.Cleanup();
    FTargets->Free();
  }
  void Save(FILE* AOutFile);
  int GetNoTargets();
  TTarget* GetTarget(int AIndex);
  bool IsOutOfDate();
  bool IsModified();
  void DropTargets();
  void AddTarget(AnsiString AName);
  AnsiString GetBinFileName();
  AnsiString GetParameterFileName();
  AnsiString GetIDLFileName();
  void LoadSource();
  void NewSource();
  void CloseSource();
  void SaveSource(bool Prompt=false);
  void SetSwitches();
  void Compile(bool ClearMemo=true);
  __property int NoTargets =
  {
    read = GetNoTargets
  };
  __property TTarget* Target[int AIndex] =
  {
    read = GetTarget
  };
  __property bool OutOfDate =
  {
    read = IsOutOfDate
  };
  __property bool Modified =
  {
    read = IsModified
  };
};

class TProject : public TFile
{
private:
  void AddSwitch(AnsiString ALine);
  bool LockProject;
protected:
  TList*       FSources;
  TStringList* FSwitches;
public:
  TProject(AnsiString AName);
  virtual __fastcall ~TProject();
  void New();
  void Old();
  int GetNoSources();
  void SetProjectSwitches();
  TSource* GetSource(int AIndex);
  void SetStrSwitch(AnsiString AIndex, AnsiString AValue);
  AnsiString GetStrSwitch(AnsiString AIndex);
  void SetIntSwitch(AnsiString AIndex, int AValue);
  int GetIntSwitch(AnsiString AIndex);
  int GetIntSwitch1(AnsiString AIndex);
  void Load();
  void Save();
  void Link(bool ClearMemo=true);
#if !defined(CONSOLE_APP)
  void CombineParms(TSynEdit* Edit);
  void CombineIDL(TSynEdit* Edit);
#endif
  void Reload();
  TSource* FindSource(AnsiString AName);
  void AddSource(AnsiString AName, int FirstVisibleLine);
  void Remove(TSource *ASource);
  __property int NoSources =
  {
    read = GetNoSources
  };
  __property TSource* Source[int AIndex] =
  {
    read = GetSource
  };
  __property AnsiString StrSwitch[AnsiString AIndex] =
  {
    read  = GetStrSwitch,
    write = SetStrSwitch
  };
  __property int IntSwitch[AnsiString AIndex] =
  {
    read  = GetIntSwitch,
    write = SetIntSwitch
  };
  __property int IntSwitch1[AnsiString AIndex] =
  {
    read  = GetIntSwitch1,
    write = SetIntSwitch
  };
};

#endif
