#ifndef _YYERROR_H_
#define _YYERROR_H_

#define WARNING_ERROR  4
#define CODING_ERROR   8
#define FATAL_ERROR   16

extern char *yyerrsrc;
extern int   yyerrlevel;
extern int   yylineno;
extern int   yycolumn;
typedef void(*yyhandlerptr)(const char*);
extern yyhandlerptr yyhandler;

void yyerror(const char *s, ...);
const char *yytoken(int aTokenNo);
void yyreport(int level, const char* s, ...);

#endif

