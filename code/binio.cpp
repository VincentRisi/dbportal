#include "binio.h"

#include <stdio.h>
#include <stdlib.h>

#include "xtalloc.h"
#include "xstring.h"
#include <stdio.h>

char getInt8(FILE * binFile)
{
  return fgetc(binFile);
}

pchar getString(FILE * binFile)
{
  int n = fgetc(binFile);
  pchar s = (pchar)calloc(n + 1, 1);
  fread(s, 1, n, binFile);
  return s;
}

pchar getExact(uint16 Len, FILE * binFile)
{
  pchar s = (pchar)calloc(Len + 1, 1);
  fread(s, 1, Len, binFile);
  return s;
}

uint16 getUInt16(FILE * binFile)
{
  uint16 u;
  fread(& u, sizeof(uint16), 1, binFile);
  Swap(u, (int)sizeof(u));
  return u;
}

int16 getInt16(FILE * binFile)
{
  int16 s;
  fread(& s, sizeof(int16), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

uint32 getUInt32(FILE * binFile)
{
  uint32 u;
  fread(& u, sizeof(uint32), 1, binFile);
  Swap(u, (int)sizeof(u));
  return u;
}

int32 getInt32(FILE * binFile)
{
  int32 s;
  fread(& s, sizeof(int32), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

double getDouble(FILE * binFile)
{
  double s;
  fread(& s, sizeof(double), 1, binFile);
  Swap(s, (int)sizeof(s));
  return s;
}

#if defined(WANT_binio_verbose)
int binio_verbose = 0;
FILE * binio_log_file = stdout;

#endif

void putInt8(char c, FILE * binFile)
{
  fputc(c, binFile);
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %c", c);
#endif
}

void putString(pchar s, FILE * binFile)
{
  fputc(strlen(s), binFile);
  fputs(s, binFile);
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, "\n%s", s);
#endif
}

void putExact(pchar s, FILE * binFile)
{
  fputs(s, binFile);
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, "%s", s);
#endif
}

void putUInt16(uint16 i, FILE * binFile)
{
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %hu", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(uint16), 1, binFile);
}

void putInt16(int16 i, FILE * binFile)
{
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %hd", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(int16), 1, binFile);
}

void putUInt32(uint32 i, FILE * binFile)
{
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %u", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(uint32), 1, binFile);
}

void putInt32(int32 i, FILE * binFile)
{
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %d", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(int32), 1, binFile);
}

void putDouble(double i, FILE * binFile)
{
#if defined(WANT_binio_verbose)
  if (binio_verbose) fprintf(binio_log_file, " %0.4f", i);
#endif
  Swap(i, (int)sizeof(i));
  fwrite(& i, sizeof(double), 1, binFile);
}


