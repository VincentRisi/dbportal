#!/usr/bin/env python
import os, sys
SRC_DIR = sys.argv[1]
BIN_DIR = sys.argv[2]

cmd1 = 'bison -v --output=%s/sqldef.cpp --defines=%s/sqldef.h %s/sqldef.y' % (BIN_DIR, BIN_DIR, SRC_DIR)
print cmd1
os.system(cmd1)

top = '''\
#include "machine.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "global.h"
#include "yyerror.h"

#define DIM(a) (sizeof(a)/sizeof(a[0]))
#define YYERRCODE 256

char *yyerrsrc;
int yyerrlevel;
int yylineno=1;
int yycolumn=1;
yyhandlerptr yyhandler;

void yyerror(const char* s, ...)
{
  char yyerrbuff[512];
  va_list argptr;
  va_start(argptr, s);
  vsprintf(yyerrbuff, s, argptr);
  if (yyhandler) yyhandler(yyerrbuff);
}

void yyreport(int level, const char* s, ...)
{
  char yyerrbuff[512];
  va_list argptr;
  if (yyerrlevel < level)
     yyerrlevel = level;
  int n=sprintf(yyerrbuff, "%s(%d): %d ", yyerrsrc, yylineno, level);
  va_start(argptr, s);
  vsprintf(yyerrbuff+n, s, argptr);
  if (yyhandler) yyhandler(yyerrbuff);
}

const char *yytoken(int aTokenNo)
{
  const char *Token[] =
'''

bottom = '''\
  };
  static char Buf[32];
  switch (aTokenNo)
  {
  case 0:
    return ("[end of file]");
  case YYERRCODE:
    return ("[error]");
  case '\\a':
    return "'\\\\a'";
  case '\\b':
    return "'\\\\b'";
  case '\\f':
    return "'\\\\f'";
  case '\\n':
    return "'\\\\n'";
  case '\\r':
    return "'\\\\r'";
  case '\\t':
    return "'\\\\t'";
  case '\\v':
    return "'\\\\v'";
  }
  if (aTokenNo > YYERRCODE && aTokenNo < YYERRCODE + DIM(Token))
    return Token[aTokenNo-(YYERRCODE+1)];
  if (isascii(aTokenNo) && isprint(aTokenNo))
    sprintf(Buf, \"'%c'\", aTokenNo);
  else
    sprintf(Buf, \"<%d>\", aTokenNo);
  return Buf;
}
'''
STATE_NONE=0;STATE_ENUM=1
print '%s/sqldef.h' % (BIN_DIR)
ifile = open('%s/sqldef.h' % (BIN_DIR), 'rt')
lines = ifile.readlines()
ifile.close()
comma = '{'
state = STATE_NONE
print '%s/yytok.cpp' % (BIN_DIR)
ofile = open('%s/yytok.cpp' % (BIN_DIR), 'wt')
ofile.write(top);
#print lines
for line in lines:
  if state == STATE_NONE:
    if line.find('yytokentype') > 0:
      state = STATE_ENUM
      print 'enum'
    continue  
  if state == STATE_ENUM:
    if line.find('};') > 0:
      print 'done'
      break;
  token = line.split()[0][6:]   
  if len(line.split()) > 1:
    ofile.write('  %s "%s"\n' % (comma, token))   
    comma = ','
ofile.write(bottom)
ofile.close()  
