
#include "machine.h"

#include <stdlib.h>
#include <string.h>
#include "xdir.h"

#if defined(M_OS2) || defined(M_DOS) || defined(M_W32)
void FNameMerge(char *path, const char *dir, const char *name, const char *ext)
{
  _makepath(path, "", dir, name, ext);
  for (size_t i=0; i<strlen(path); i++)
    if (path[i] == '/') path[i] = '\\';
}
#endif

#if defined(M_AIX) || defined(M_GNU)
void FNameMerge(char *path, const char *dir, const char *name, const char *ext)
{
  int i;
  if (dir)
  {
    strcpy(path, dir);
    if ((i=strlen(dir)) && (dir[i-1] != '/'))
      strcat(path, "/");
  }
  if (name)
    strcat(path, name);
  if (ext)
  {
    if (strlen(ext) && *ext != '.')
      strcat(path, ".");
    strcat(path, ext);
  }
}
#endif


