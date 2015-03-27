
#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if defined(M_OS2) || defined(M_DOS) || defined(M_W32)
#include <process.h>
#include <io.h>
#endif

#define _XTALLOC_NOMACS_
#include "xtalloc.h"
int record_xtalloc=0;

static FILE *xtalloc = NULL;
static void open_xtalloc(void)
{
  char name[12] = "xtalloc.000";
  int no = 0;

  if (xtalloc)
    return;
  while(access(name,0)==0)
    sprintf(name, "xtalloc.%03d", ++no);
  xtalloc = fopen(name, "wt");
}

static void record_alloc(char *tag, void *area, char *file, int lineno, size_t size)
{
  if (record_xtalloc)
  {
    open_xtalloc();
    fprintf(xtalloc, "alloc %-12s %p %-20s %5d %5d\n", tag, area, file, lineno, (int)size);
    fflush(xtalloc);
  }
}

static void record_free(char *tag, void *area, char *file, int lineno)
{
  if (record_xtalloc)
  {
    open_xtalloc();
    fprintf(xtalloc, "free  %-12s %p %-20s %5d\n", tag, area, file, lineno);
    fflush(xtalloc);
  }
}

static void record_abort(char *tag, char *msg, char *file, int lineno, size_t size)
{
  open_xtalloc();
  fprintf(xtalloc, "ERROR %s : %s %-20s %5d %5d\n", tag, msg, file, lineno, (int)size);
  fflush(xtalloc);
  exit(100);
}

void * extended_malloc(size_t size, char *file, int lineno)
{
  void *area = malloc(size);
  record_alloc("malloc", area, file, lineno, size);
  return area;
}

void * extended_calloc(size_t nitems, size_t size, char *file, int lineno)
{
  void *area = calloc(nitems, size);
  record_alloc("calloc", area, file, lineno, nitems*size);
  return area;
}

void * extended_realloc(void *block, size_t size, char *file, int lineno)
{
  void *area;
  if (block)
    record_free("realloc", block, file, lineno);
  area = realloc(block, size);
  record_alloc("realloc", area, file, lineno, size);
  return area;
}

char * extended_strdup(const char *s, char *file, int lineno)
{
  char *area = strdup(s);
  record_alloc("strdup", (void *)area, file, lineno, (size_t)strlen(s)+1);
  return area;
}

#ifndef __WATCOMC__
char* extended_tempnam(char *dir, char *prefix, char *file, int lineno)
{
  char *area = tempnam(dir, prefix);
  record_alloc("tempnam", (void *)area, file, lineno, (size_t)strlen(area));
  return area;
}
#endif

void extended_free(void *block, char *file, int lineno)
{
  record_free("free", block, file, lineno);
  free(block);
}

