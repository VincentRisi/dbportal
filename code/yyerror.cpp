#include "yyerror.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


char *yyerrsrc;
FILE *yyerrfile;
int yyerrlevel;
int yylineno=1;
int yycolumn;

void yyerror(const char *s, ...)
{
  va_list argptr;
  va_start(argptr, s);
  vfprintf(yyerrfile, s, argptr);
}

void yyreport(int level, const char* s, ...)
{
  va_list argptr;
  if (yyerrlevel < level)
     yyerrlevel = level;
  fprintf(yyerrfile, "%s %d(%d) - ", yyerrsrc, yylineno, level);
  va_start(argptr, s);
  vfprintf(yyerrfile, s, argptr);
  if (level >= FATAL_ERROR)
     exit(level);
}

