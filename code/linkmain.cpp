#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#if defined(M_W32)
#include <io.h>
#endif
#include "xstring.h"
#include "xtalloc.h"

#include "xdir.h"
#include "getargs.h"
#include "binio.h"
#include "sqlbin.h"

#include "slink.h"

#pragma package(smart_init)

static FILE *LogFile      = 0;

static const char *ProjBinFileName = "sqlapi";
static const char *ProjDir         = "";
static const char *ProjExt         = ".bin";
static int         Verbose         = 0;
static const char *FileInList      = "";

static ARG argtab[] =
{
  {'v', BOOLEAN,      &Verbose,         "Verbose Compile" }
 ,{'b', STRING, (int*)&ProjDir,         "Project Directory"}
 ,{'B', STRING, (int*)&ProjExt,         "Project Extension"}
 ,{'n', STRING, (int*)&ProjBinFileName, "Project FileName"}
 ,{'f', STRING, (int*)&FileInList,      "FileName of inputs"}
};

#define TABSIZE (sizeof(argtab) / sizeof(ARG))

int binio_verbose = Verbose;

const int BUFF_LEN = 1024;
static char* loadFiles(char* buffer, int &len, ushort sign)
{
  char* fp = buffer;
  char work[BUFF_LEN];
  while (true)
  {
    char* semi = strchr(fp, ';');
    if (semi != 0)
    {
      strncpyz(work, fp, semi-fp);
      fp = semi + 1;
      fprintf(LogFile, "Loading in SO: %s\n", work);
      LoadInFile(work, sign);
      continue;
    }
    char *nl = strchr(fp, '\n');
    if (nl != 0)
    {
      char *cr = strchr(fp, '\r');
      if (cr != 0) 
        nl = cr;
      strncpyz(work, fp, nl-fp);
      fprintf(LogFile, "Loading in SO: %s\n", work);
      LoadInFile(work, sign);
      len = BUFF_LEN;
      return buffer;
    }
    strcpy(buffer, fp);
    len = BUFF_LEN - strlen(buffer);
    return buffer + strlen(buffer);
  }
}

int main(int argc, char *argv[])
{
  int i;
  argc = GetArgs(argc, argv, argtab, TABSIZE);
  LogFile = stdout;
  if (argc < 2 && strlen(FileInList) == 0)
  {
    fprintf(LogFile, "Release(%s) Compiled %s\n", "15.03.0.0", __DATE__);
    fprintf(LogFile, "Usage: pocilink files\n"
                     "       where files is the name of sqldef files to be"
                     " linked\n");
    PrUsage(argtab, TABSIZE);
    return 1;
  }
  int start = 1;
  ushort sign = slinkSign;
  if (!ProjBinFileName)
    ProjBinFileName = argv[start++];
  fprintf(LogFile, "Making BinFile: %s%s\n", ProjBinFileName, ProjExt);
  OpenBinFile(ProjBinFileName, ProjDir, ProjExt);
  if (strlen(FileInList) > 0 && access(FileInList, 0) == 0)
  {
    FILE *infile = fopen(FileInList, "rt");
    char buffer[BUFF_LEN+1];
    buffer[BUFF_LEN] = 0;
    char *fp = buffer;
    int len = sizeof(buffer);
    while (!feof(infile))
    {
      memset(fp, 0, len);
      fgets(fp, len, infile);
      if (strlen(buffer) < BUFF_LEN)
        buffer[strlen(buffer)] = '\n';
      fp = loadFiles(buffer, len, sign);
    }
    fclose(infile);
  }
  for (i=start; i<argc; i++)
  {
    fprintf(LogFile, "Loading in SO: %s\n", argv[i]);
    LoadInFile(argv[i], sign);
  }
  qsort(Queries, NoQueries, sizeof(PSqlQuery), (fptr)SortComp);
  StoreBinFile(sign);
  fprintf(LogFile, "Done\n");
  LogFile = 0;
  return 0;
}
