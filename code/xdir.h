#ifndef _XDIR_H_
#define _XDIR_H_ 

#if defined(M_OS2) || defined(M_DOS)
#include <stdlib.h>
#ifdef __BORLANDC__
#include <dir.h>
#endif
#define DIRMAX_PATH _MAX_PATH
#define DIRMAX_DIR  _MAX_DRIVE + _MAX_DIR
#define DIRMAX_NAME _MAX_FNAME
#define DIRMAX_EXT  _MAX_EXT
#define PATH_DELIM ';'
#else
#include <stdio.h>
#define DIRMAX_PATH FILENAME_MAX
#define DIRMAX_NAME FILENAME_MAX
#define DIRMAX_DIR  FILENAME_MAX
#define DIRMAX_EXT  FILENAME_MAX
#define PATH_DELIM ':'
#endif

void FNameSplit(const char *path, char *dir, char *name, char *ext);
/*  splits path into user supplied memory for dir, name and ext */

void FNameMerge(char *path, const char *dir, const char *name, const char *ext);
/*  merges dir, name and ext into user supplied memory for path */

char *FNameName(char *Path);
/*  returns strdup of name part of path (user must free) */

char *FNameDir(char *Path);
/*  returns strdup of dir part of path (user must free) */

char *FNameExt(char *Path);
/*  returns strdup of ext part of path (user must free) */

char *FNameFind(char *File, char *Env);
/*  returns static area containing path if found else empty string */
/*  is kept between calls to FNameFind. */

#endif

