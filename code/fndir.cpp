
#include "machine.h"

#include <string.h>
#include "xdir.h"
#include "xtalloc.h"

char *FNameDir(char *Path)
{
  char Dir[DIRMAX_DIR];
  char Name[DIRMAX_NAME];
  char Ext[DIRMAX_EXT];

  FNameSplit(Path, Dir, Name, Ext);
  return strdup(Dir);
}


