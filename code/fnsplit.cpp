
#include "machine.h"

#include <stdlib.h>
#include "xdir.h"
#include "xstring.h"

#if defined(M_OS2) || defined(M_DOS) || defined(M_W32)

void FNameSplit(const char *path, char *dir, char *name, char *ext)
{
  char drive[_MAX_DRIVE], dir2[_MAX_DIR];
  _splitpath(path, drive, dir2, name, ext);
  strcpy(dir, drive);
  strcat(dir, dir2);
}
#endif

#if defined(M_AIX) || defined(M_GNU)
void FNameSplit(const char *path, char *dir, char *name, char *ext)
{
  char *p = strrchr((char*)path, '/');
  char *p2;

  if (p)
  {
    strncpyz(dir, path, (p-path)+1);
    p++;
  }
  else
  {
    dir[0] = 0;
    p = (char*)path;
  }
  p2 = strrchr(p+1, '.');
  if (p2)
  {
    strncpyz(name, p, p2-p);
    strcpy(ext, p2);
  }
  else
  {
    strcpy(name, p);
    ext[0] = 0;
  }
}

#endif


