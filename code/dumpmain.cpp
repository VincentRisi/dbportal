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

static char *ProjBinFileName = "sqlapi";
static char *FileInList      = "";

static ARG argtab[] =
{
  {'f', STRING, (int*)&FileInList,      "FileName of inputs"}
};

#define TABSIZE (sizeof(argtab) / sizeof(ARG))

ushort sqldefSign  = 0x0C1A;
ushort slinkSign   = 0x0C1B;
ushort sqldefSign2 = 0x0C2A;
ushort slinkSign2  = 0x0C2B;
ushort sqldefTailMark = 0xBEEF;

void DumpSOFile(pchar InFileName, ushort &slink)
{
  FILE* InFile;
  long FileSize;
  tSqlQuery Query;
  ushort i, j, NoProcs, sign;
  printf("%-13s: %s\n", "Input", InFileName);
  InFile = fopen(InFileName, "rb");
  if (InFile == 0)
  {
    printf("LoadinFile Access for file [%s] failed!\n", InFileName);
    goto Return;
  }
  if (setvbuf(InFile, 0, _IOFBF, 131072))
    printf("Not enough memory to buffer file!\n");
  FileSize = fseek(InFile, 0L, SEEK_END);
  fseek(InFile, 0L, SEEK_SET);
  sign = GetUShort(InFile);
  if (sign != sqldefSign && sign != sqldefSign2)
  {
    printf("Not a valid SO file!\n");
    goto Return;
  }
  else if (sign == sqldefSign2) // if anywhere a binary is sign2
    slink = slinkSign2;         // then the whole bin must be sign2
  printf("Server    : %s\n", GetString(InFile));
  printf("Schema    : %s\n", GetString(InFile));
  printf("Table     : %s\n", GetString(InFile));
  NoProcs = GetUShort(InFile);
  printf("NoProcs   : %d\n\n", NoProcs);  
  for (i=0; i<NoProcs; i++)
  {
    Query.Name = GetString(InFile);
    Query.NoFields = GetUShort(InFile);
    Query.isSql = Query.NoFields & 0x8000;
    Query.isFetch = Query.NoFields & 0x4000;
    Query.isMultiFetch = Query.NoFields & 0x2000;
    Query.isNullEnabled = Query.NoFields & 0x1000;
    Query.NoFields &= 0x0FFF;
    printf("%2d: %-32s %d %s%s%s\n"
          , i+1, Query.Name, Query.NoFields
          , Query.isFetch ? " Fetch" : ""
          , Query.isMultiFetch ? " MultiFetch" : ""
          , Query.isNullEnabled ? " NullEnabled" : ""
          );
    if (Query.NoFields)
    {
      tSqlField Field;
      for (j=0; j<Query.NoFields; j++)
      {
        Field.Name = GetString(InFile);
        Field.CType = GetShort(InFile);
        Field.SqlType = GetShort(InFile);
        if (sign == sqldefSign2)
        {
          Field.Size = GetULong(InFile);
          Field.Offset = GetULong(InFile);
          Field.Precision = GetULong(InFile);
          Field.Scale = GetULong(InFile);
        }
        else
        {
          Field.Size = GetUShort(InFile);
          Field.Offset = GetUShort(InFile);
          Field.Precision = GetUShort(InFile);
          Field.Scale = GetUShort(InFile);
        }
        Field.isBind = GetUShort(InFile);
        Field.isDefine = GetUShort(InFile);
        printf(" %2d: %-32s c:%3d s:%3d (%d, %d, %d, %d) %s%s\n"
              , j+1
              , Field.Name
              , Field.CType
              , Field.SqlType
              , Field.Size
              , Field.Offset
              , Field.Precision
              , Field.Scale
              , Field.isBind ? " Bind" : ""
              , Field.isDefine ? " Define" : ""
              );
      }
    }
    Query.Size = GetUShort(InFile);
    Query.Command = GetExact(Query.Size, InFile);
    printf("________________________________________________\n"
           "%s\n"
           "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n", Query.Command);
  }
  if (ftell(InFile) < FileSize)
  {
    sign = GetUShort(InFile);
    if (sign == sqldefTailMark)
      goto Return;
    fseek(InFile, -2L, SEEK_CUR);
    printf("Token (%s=%s)\n", GetString(InFile), GetString(InFile));
  }
Return:
  if (InFile) fclose(InFile);
}

const int BUFF_LEN = 1024;
static char* dumpFiles(char* buffer, int &len, ushort sign)
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
      DumpSOFile(work, sign);
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
      DumpSOFile(work, sign);
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
    fprintf(LogFile, "Usage: pocidump files\n"
                     "       where files is the name of sqldef files to be"
                     " dumped\n");
    PrUsage(argtab, TABSIZE);
    return 1;
  }
  int start = 1;
  ushort sign = slinkSign;
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
      fp = dumpFiles(buffer, len, sign);
    }
    fclose(infile);
  }
  for (i=start; i<argc; i++)
  {
    fprintf(LogFile, "Loading in SO: %s\n", argv[i]);
    DumpSOFile(argv[i], sign);
  }
  fprintf(LogFile, "Done\n");
  return 0;
}
