#include "genproc.h"

static const char* outDirName = "";
static const char* inListName = "";

ARG argtab[] =
{ {'o', STRING,  (int*) &outDirName, "Output Directory"}
, {'f', STRING,  (int*) &inListName, "Input file list"} 
};
#define TABSIZE (sizeof(argtab) / sizeof(ARG))

int main(int argc, char *argv[])
{
  argc = GetArgs(argc, argv, argtab, TABSIZE);
  return 0;
}
