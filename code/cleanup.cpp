#include "machine.h"

#include <stdio.h>
#include "xstring.h"
#include <stdlib.h>

#include "sqldef.h"
#include "global.h"

void TYYConst::Cleanup()
{
  if (Name) free(Name);
  Clear();
}

void TYYField::Cleanup()
{
  if (Name)    free(Name);
  if (Alias)   free(Alias);
  if (Comment) free(Comment);
  if (Check)   free(Check);
  for (int32 i=0; i<noConsts; i++)
    Consts[i].Cleanup();
  if (Consts)  free(Consts);
  Clear();
}

void TYYTable::Cleanup()
{
  Count.Cleanup();
  int32 i;
  for (i=0; i<noFields; i++)            Fields[i].Cleanup();
  for (i=0; i<noGrants; i++)            Grants[i].Cleanup();
  for (i=0; i<noKeys; i++)              Keys[i].Cleanup();
  for (i=0; i<noLinks; i++)             Links[i].Cleanup();
  for (i=0; i<noInsertUsers; i++)       InsertUsers[i].Cleanup();
  for (i=0; i<noInsertProjects; i++)    InsertProjects[i].Cleanup();
  for (i=0; i<noSelectAllUsers; i++)    SelectAllUsers[i].Cleanup();
  for (i=0; i<noSelectAllProjects; i++) SelectAllProjects[i].Cleanup();
  for (i=0; i<noSelectOneUsers; i++)    SelectOneUsers[i].Cleanup();
  for (i=0; i<noSelectOneProjects; i++) SelectOneProjects[i].Cleanup();
  for (i=0; i<noSelectAllUpdUsers; i++) SelectAllUpdUsers[i].Cleanup();
  for (i=0; i<noSelectAllUpdProjects; i++) SelectAllUpdProjects[i].Cleanup();
  for (i=0; i<noSelectOneUpdUsers; i++) SelectOneUpdUsers[i].Cleanup();
  for (i=0; i<noSelectOneUpdProjects; i++) SelectOneUpdProjects[i].Cleanup();
  for (i=0; i<noDeleteAllUsers; i++)    DeleteAllUsers[i].Cleanup();
  for (i=0; i<noDeleteAllProjects; i++) DeleteAllProjects[i].Cleanup();
  for (i=0; i<noDeleteOneUsers; i++)    DeleteOneUsers[i].Cleanup();
  for (i=0; i<noDeleteOneProjects; i++) DeleteOneProjects[i].Cleanup();
  for (i=0; i<noExistsUsers; i++)       ExistsUsers[i].Cleanup();
  for (i=0; i<noExistsProjects; i++)    ExistsProjects[i].Cleanup();
  for (i=0; i<noUpdateUsers; i++)       UpdateUsers[i].Cleanup();
  for (i=0; i<noUpdateProjects; i++)    UpdateProjects[i].Cleanup();
  for (i=0; i<noCountUsers; i++)        CountUsers[i].Cleanup();
  for (i=0; i<noCountProjects; i++)     CountProjects[i].Cleanup();
  for (i=0; i<noExecuteUsers; i++)      ExecuteUsers[i].Cleanup();
  for (i=0; i<noExecuteProjects; i++)   ExecuteProjects[i].Cleanup();
  for (i=0; i<noSelectUsers; i++)       SelectUsers[i].Cleanup();
  for (i=0; i<noSelectProjects; i++)    SelectProjects[i].Cleanup();
  for (i=0; i<noUsers; i++)             Users[i].Cleanup();
  for (i=0; i<noProjects; i++)          Projects[i].Cleanup();
  for (i=0; i<noViews; i++)             Views[i].Cleanup();
  for (i=0; i<noProcs; i++)             Procs[i].Cleanup();
  if (Server)                           free(Server);
  if (UserID)                           free(UserID);
  if (PassWD)                           free(PassWD);
  if (Name)                             free(Name);
  if (Alias)                            free(Alias);
  if (Check)                            free(Check);
  if (Fields)                           free(Fields);
  if (Grants)                           free(Grants);
  if (Keys)                             free(Keys);
  if (Links)                            free(Links);
  if (InsertUsers)                      free(InsertUsers);
  if (InsertProjects)                   free(InsertProjects);
  if (SelectAllUsers)                   free(SelectAllUsers);
  if (SelectAllProjects)                free(SelectAllProjects);
  if (SelectOneUsers)                   free(SelectOneUsers);
  if (SelectOneProjects)                free(SelectOneProjects);
  if (SelectAllUpdUsers)                free(SelectAllUpdUsers);
  if (SelectAllUpdProjects)             free(SelectAllUpdProjects);
  if (SelectOneUpdUsers)                free(SelectOneUpdUsers);
  if (SelectOneUpdProjects)             free(SelectOneUpdProjects);
  if (DeleteAllUsers)                   free(DeleteAllUsers);
  if (DeleteAllProjects)                free(DeleteAllProjects);
  if (DeleteOneUsers)                   free(DeleteOneUsers);
  if (DeleteOneProjects)                free(DeleteOneProjects);
  if (ExistsUsers)                      free(ExistsUsers);
  if (ExistsProjects)                   free(ExistsProjects);
  if (UpdateUsers)                      free(UpdateUsers);
  if (UpdateProjects)                   free(UpdateProjects);
  if (CountUsers)                       free(CountUsers);
  if (CountProjects)                    free(CountProjects);
  if (ExecuteUsers)                     free(ExecuteUsers);
  if (ExecuteProjects)                  free(ExecuteProjects);
  if (SelectUsers)                      free(SelectUsers);
  if (SelectProjects)                   free(SelectProjects);
  if (Users)                            free(Users);
  if (Projects)                         free(Projects);
  if (Views)                            free(Views);
  if (Procs)                            free(Procs);
  if (Space)                            free(Space);
  if (ControlDB)                        free(ControlDB);
  if (SODir)                            free(SODir);
  if (CDir)                             free(CDir);
  if (VBDir)                            free(VBDir);
  if (VB5Dir)                           free(VB5Dir);
  if (VBCode3Dir)                       free(VBCode3Dir);
  if (VBCode3Ext)                       free(VBCode3Ext);
  if (VBNet7Dir)                        free(VBNet7Dir);
  if (VBNet7Ext)                        free(VBNet7Ext);
  if (CSNet7Dir)                        free(CSNet7Dir);
  if (CSNet7Ext)                        free(CSNet7Ext);
  if (VBforIDLDir)                      free(VBforIDLDir);
  if (VBforIDLExt)                      free(VBforIDLExt);
  if (PASDir)                           free(PASDir);
  if (PARMSDir)                         free(PARMSDir);
  if (PARMSDescr)                       free(PARMSDescr);
  if (SqlDir)                           free(SqlDir);
  if (LogDir)                           free(LogDir);
  if (IDLDir)                           free(IDLDir);
  if (PythonDir)                        free(PythonDir);
  if (UConnect)                         free(UConnect);
  if (IDLModule)                        free(IDLModule);
  if (SOExt)                            free(SOExt);
  if (CExt)                             free(CExt);
  if (VBExt)                            free(VBExt);
  if (VB5Ext)                           free(VB5Ext);
  if (PASExt)                           free(PASExt);
  if (PARMSExt)                         free(PARMSExt);
  if (PARMSLookup)                      free(PARMSLookup);
  if (Show)                             free(Show);
  if (SqlExt)                           free(SqlExt);
  if (SqlTableExt)                      free(SqlTableExt);
  if (SqlViewsExt)                      free(SqlViewsExt);
  if (SqlConExt)                        free(SqlConExt);
  if (SqlIdxExt)                        free(SqlIdxExt);
  if (SqlGrantExt)                      free(SqlGrantExt);
  if (SqlProcExt)                       free(SqlProcExt);
  if (SqlSnapExt)                       free(SqlSnapExt);
  if (SqlAuditExt)                      free(SqlAuditExt);
  if (LogExt)                           free(LogExt);
  if (IDLExt)                           free(IDLExt);
  if (PythonExt)                        free(PythonExt);
  if (InputFileName)                    free(InputFileName);
  if (ShortName)                        free(ShortName);
  if (SqlFileName)                      free(SqlFileName);
  if (SqlTableFileName)                 free(SqlTableFileName);
  if (SqlConFileName)                   free(SqlConFileName);
  if (SqlIdxFileName)                   free(SqlIdxFileName);
  if (SqlGrantFileName)                 free(SqlGrantFileName);
  if (SqlProcFileName)                  free(SqlProcFileName);
  if (SqlSnapFileName)                  free(SqlSnapFileName);
  if (SqlAuditFileName)                 free(SqlAuditFileName);
  if (SqlViewsFileName)                 free(SqlViewsFileName);
  if (HeaderFileName)                   free(HeaderFileName);
  if (VBFileName)                       free(VBFileName);
  if (VB5FileName)                      free(VB5FileName);
  if (VBCode3FileName)                  free(VBCode3FileName);
  if (VBNet7FileName)                   free(VBNet7FileName);
  if (CSNet7FileName)                   free(CSNet7FileName);
  if (VBforIDLFileName)                 free(VBforIDLFileName);
  if (PASFileName)                      free(PASFileName);
  if (PARMSFileName)                    free(PARMSFileName);
  if (BinFileName)                      free(BinFileName);
  if (LogFileName)                      free(LogFileName);
  if (IDLFileName)                      free(IDLFileName);
  if (PythonFileName)                   free(PythonFileName);
  Clear();
}

void TYYGrant::Cleanup()
{
  int32 i;
  for (i=0; i<noPerms; i++)
    Perms[i].Cleanup();
  for (i=0; i<noUsers; i++)
    Users[i].Cleanup();
  if (Perms) free(Perms);
  if (Users) free(Users);
  Clear();
}

void TYYKey::Cleanup()
{
  if (Name)   free(Name);
  if (Fields) free(Fields);
  if (Space)  free(Space);
  Clear();
}

void TYYView::Cleanup()
{
  int32 i;
  if (Name) free(Name);
  for (i=0; i<noLines; i++)
    Lines[i].Cleanup();
  if (Lines) free(Lines);
  for (i=0; i<noUsers; i++)
    Users[i].Cleanup();
  if (Users) free(Users);
  Clear();
}

void TYYProc::Cleanup()
{
  int32 i;
  if (Name) free(Name);
  for (i=0; i<noFields; i++)
    Fields[i].Cleanup();
  if (Fields) free(Fields);
  if (Binds) free(Binds);
  for (i=0; i<noWorkFields; i++)
    WorkFields[i].Cleanup();
  if (WorkFields) free(WorkFields);
  for (i=0; i<noCursors; i++)
    Cursors[i].Cleanup();
  if (Cursors) free(Cursors);
  for (i=0; i<noLines; i++)
    Lines[i].Cleanup();
  if (Lines) free(Lines);
  for (i=0; i<noUsers; i++)
    Users[i].Cleanup();
  for (i=0; i<noProjects; i++)
    Projects[i].Cleanup();
  if (Users) free(Users);
  if (Projects) free(Projects);
  Clear();
}

void TYYCursor::Cleanup()
{
  for (int32 i=0; i<noLines; i++)
    Lines[i].Cleanup();
  if (Lines) free(Lines);
  Clear();
}

void TYYLink::Cleanup()
{
  if (MasterName) free(MasterName);
  if (Fields)     free(Fields);
  Clear();
}


