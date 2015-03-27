#include "binio.h"

#include <stdio.h>
#include <stdlib.h>

#include "xtalloc.h"
#include "xstring.h"
#include <stdio.h>

char GetSChar(FILE * binFile)
{
  return fgetc(binFile);
}

pchar GetString(FILE * binFile)
{
  int n = fgetc(binFile);
  pchar s = (pchar)calloc(n + 1, 1);
  fread(s, 1, n, binFile);
  return s;
}

pchar GetExact(uint16 Len, FILE * binFile)
{
  pchar s = (pchar)calloc(Len + 1, 1);
  fread(s, 1, Len, binFile);
  return s;
}

uint16 GetUShort(FILE * binFile)
{
  uint16 u;
  fread(& u, sizeof(uint16), 1, binFile);
  Swap(u, (int)sizeof(u));
  return u;
}

int16 GetShort(FILE * binFile)
{
  int16 s;
  fread(& s, sizeof(int16), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

uint32 GetULong(FILE * binFile)
{
  uint32 u;
  fread(& u, sizeof(uint32), 1, binFile);
  Swap(u, (int)sizeof(u));
  return u;
}

int32 GetLong(FILE * binFile)
{
  int32 s;
  fread(& s, sizeof(int32), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

double GetDouble(FILE * binFile)
{
  double s;
  fread(& s, sizeof(double), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

#if defined(WANT_BINIOVERBOSE)
int BinioVerbose = 0;
FILE * BinioLogFile = stdout;

#endif

void PutSChar(char c, FILE * binFile)
{
  fputc(c, binFile);
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %c", c);
#endif
}

void PutString(pchar s, FILE * binFile)
{
  fputc(strlen(s), binFile);
  fputs(s, binFile);
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, "\n%s", s);
#endif
}

void PutExact(pchar s, FILE * binFile)
{
  fputs(s, binFile);
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, "%s", s);
#endif
}

void PutUShort(uint16 i, FILE * binFile)
{
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %hu", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(uint16), 1, binFile);
}

void PutShort(int16 i, FILE * binFile)
{
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %hd", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(int16), 1, binFile);
}

void PutULong(uint32 i, FILE * binFile)
{
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %u", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(uint32), 1, binFile);
}

void PutLong(int32 i, FILE * binFile)
{
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %d", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(int32), 1, binFile);
}

void PutDouble(double i, FILE * binFile)
{
#if defined(WANT_BINIOVERBOSE)
  if (BinioVerbose) fprintf(BinioLogFile, " %0.4f", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(double), 1, binFile);
}


