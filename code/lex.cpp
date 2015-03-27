#include "machine.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "xstring.h"
#include "global.h"

int yyInit;
static char TokenBuffer[4096];
char *yytext;

//static int screen(char *text);
#define DIM(a)  (sizeof(a)/sizeof((a)[0]))

TKeywordList SwitchesList[] =
{ {"archive",         switchTARGETARCHIVE   }
, {"archivedir",      switchARCHIVEDIR      }
, {"archiveext",      switchARCHIVEEXT      }
, {"bindir",          switchBINDIR          }
, {"binext",          switchBINEXT          }
, {"blankpadded",     switchBLANKPADDED     }
, {"c",               switchTARGETC         }
, {"charz",           switchCHARZ           }
, {"connreqd",        switchCONNREQD        }
, {"controldb",       switchCONTROLDB       }
, {"csadonet",        switchTARGETCSADONET  }
, {"csadonetdir",     switchCSADONETDIR     }
, {"csadonetext",     switchCSADONETEXT     }
, {"csidl2",          switchTARGETCSIDL2    }
, {"csidl2dir",       switchCSIDL2DIR       }
, {"csidl2ext",       switchCSIDL2EXT       }
, {"csnet",           switchTARGETCSNET7    }
, {"csnetdir",        switchCSNET7DIR       }
, {"csnetext",        switchCSNET7EXT       }
, {"descr",           switchPARMSDESCR      }
, {"eightbyte",       switchEIGHTBYTE       }
, {"exitreqd",        switchEXITREQD        }
, {"extc",            switchEXTENDEDC       }
, {"extendedc",       switchEXTENDEDC       }
, {"extendedpas",     switchEXTENDEDPAS     }
, {"extendedvb",      switchEXTENDEDVB      }
, {"extpas",          switchEXTENDEDPAS     }
, {"extvb",           switchEXTENDEDVB      }
, {"headerdir",       switchHEADERDIR       }
, {"headerext",       switchHEADEREXT       }
, {"idl",             switchTARGETIDL       }
, {"idldir",          switchIDLDIR          }
, {"idlext",          switchIDLEXT          }
, {"idlmodule",       switchIDLMODULE       }
, {"idlubi",          switchIDLUBI          }
, {"internal",        switchINTERNAL        }
, {"littletcsidl2",   switchLITTLETCSIDL2   }
, {"littletrue",      switchLITTLETRUE      }
, {"logdir",          switchLOGDIR          }
, {"logext",          switchLOGEXT          }
, {"lookup",          switchPARMSLOOKUP     }
, {"nodomain",        switchNODOMAIN        }
, {"onesqlscript",    switchONESQLSCRIPT    }
, {"parms",           switchTARGETPARMS     }
, {"parmsdescr",      switchPARMSDESCR      }
, {"parmsdir",        switchPARMSDIR        }
, {"parmsext",        switchPARMSEXT        }
, {"parmslookup",     switchPARMSLOOKUP     }
, {"pas",             switchTARGETPAS       }
, {"pasdir",          switchPASDIR          }
, {"pasext",          switchPASEXT          }
, {"prefixvbclasses", switchPREFIXVBCLASSES }
, {"python",          switchTARGETPYTHON    }
, {"pythondir",       switchPYTHONDIR       }
, {"pythonext",       switchPYTHONEXT       }
, {"show",            switchSHOW            }
, {"sqlauditext",     switchSQLAUDITEXT     }
, {"sqlconext",       switchSQLCONEXT       }
, {"sqldir",          switchSQLDIR          }
, {"sqlext",          switchSQLEXT          }
, {"sqlgrantext",     switchSQLGRANTEXT     }
, {"sqlidxext",       switchSQLIDXEXT       }
, {"sqlprocext",      switchSQLPROCEXT      }
, {"sqlsnapext",      switchSQLSNAPEXT      }
, {"sqltableext",     switchSQLTABLEEXT     }
, {"sqlviewsext",     switchSQLVIEWSEXT     }
, {"targetarchive",   switchTARGETARCHIVE   }
, {"targetc",         switchTARGETC         }
, {"targetcsadonet",  switchTARGETCSADONET  }
, {"targetcsidl2",    switchTARGETCSIDL2    }
, {"targetcsnet7",    switchTARGETCSNET7    }
, {"targetidl",       switchTARGETIDL       }
, {"targetparms",     switchTARGETPARMS     }
, {"targetpas",       switchTARGETPAS       }
, {"targetpython",    switchTARGETPYTHON    }
, {"targetvb",        switchTARGETVB        }
, {"targetvb5",       switchTARGETVB5       }
, {"targetvbcode3",   switchTARGETVBCODE3   }
, {"targetvbforador", switchTARGETVBFORADOR }
, {"targetvbforidl",  switchTARGETVBFORIDL  }
, {"targetvbnet",     switchTARGETVBNET7    }
, {"uconnect",        switchUCONNECT        }
, {"underscore",      switchUNDERSCORE      }
, {"vb",              switchTARGETVB        }
, {"vb5",             switchTARGETVB5       }
, {"vb5dir",          switchVB5DIR          }
, {"vb5ext",          switchVB5EXT          }
, {"vbcode3",         switchTARGETVBCODE3   }
, {"vbcode3dir",      switchVBCODE3DIR      }
, {"vbdir",           switchVBDIR           }
, {"vbext",           switchVBEXT           }
, {"vbforador",       switchTARGETVBFORADOR }
, {"vbforidl",        switchTARGETVBFORIDL  }
, {"vbnet",           switchTARGETVBNET7    }
, {"vbnetdir",        switchVBNET7DIR       }
, {"vbnetext",        switchVBNET7EXT       }
, {"version2bin",     switchVERSION2BIN     }
, {"viewonly",        switchVIEWONLY        }
};

int SwitchesCount = DIM(SwitchesList);

TKeywordList KeywordsList[] =
{ {"all",          sqldefALL          }
, {"audit",        sqldefAUDIT        }
, {"blob",         sqldefBLOB         }
, {"binary",       sqldefBINARY       }
, {"boolean",      sqldefBOOLEAN      }
, {"cascade",      sqldefCASCADE      }
, {"char",         sqldefCHAR         }
, {"check",        sqldefCHECK        }
, {"clob",         sqldefCLOB         }
, {"connect",      sqldefCONNECT      }
, {"count",        sqldefCOUNT        }
, {"data",         sqldefDATA         }
, {"date",         sqldefDATE         }
, {"datetime",     sqldefDATETIME     }
, {"declare",      sqldefDECLARE      }
, {"delete",       sqldefDELETE       }
, {"deleteall",    sqldefDELETEALL    }
, {"deleteone",    sqldefDELETEONE    }
, {"double",       sqldefFLOAT        }
, {"exception",    sqldefEXCEPTION    }
, {"execute",      sqldefEXECUTE      }
, {"exists",       sqldefEXISTS       }
, {"fast",         sqldefFAST         }
, {"float",        sqldefFLOAT        }
, {"freelist",     sqldefFREELIST     }
, {"freelists",    sqldefFREELISTS    }
, {"from",         sqldefFROM         }
, {"full",         sqldefFULL         }
, {"grant",        sqldefGRANT        }
, {"groups",       sqldefGROUPS       }
, {"hugechar",     sqldefHUGECHAR     }
, {"image",        sqldefIMAGE        }
, {"initial",      sqldefINITIAL      }
, {"initrans",     sqldefINITRANS     }
, {"inout",        sqldefINOUT        }
, {"input",        sqldefINPUT        }
, {"insert",       sqldefINSERT       }
, {"int",          sqldefINT          }
, {"k",            sqldefK            }
, {"key",          sqldefKEY          }
, {"link",         sqldefLINK         }
, {"long",         sqldefLONG         }
, {"m",            sqldefM            }
, {"maxextents",   sqldefMAXEXTENTS   }
, {"maxtrans",     sqldefMAXTRANS     }
, {"minextents",   sqldefMINEXTENTS   }
, {"money",        sqldefMONEY        }
, {"multiple",     sqldefMULTIPLE     }
, {"next",         sqldefNEXT         }
, {"noc",          sqldefNOC          }
, {"noext",        sqldefNOEXT        }
, {"noidl",        sqldefNOIDL        }
, {"nopas",        sqldefNOPAS        }
, {"not",          sqldefNOT          }
, {"novb",         sqldefNOVB         }
, {"null",         sqldefNULL         }
, {"optimal",      sqldefOPTIMAL      }
, {"output",       sqldefOUTPUT       }
, {"pctfree",      sqldefPCTFREE      }
, {"pctincrease",  sqldefPCTINCREASE  }
, {"pctused",      sqldefPCTUSED      }
, {"primary",      sqldefPRIMARY      }
, {"proc",         sqldefPROC         }
, {"procedure",    sqldefPROC         }
, {"schema",       sqldefSCHEMA       }
, {"select",       sqldefSELECT       }
, {"selectall",    sqldefSELECTALL    }
, {"selectallupd", sqldefSELECTALLUPD }
, {"selectone",    sqldefSELECTONE    }
, {"selectoneupd", sqldefSELECTONEUPD }
, {"selectplus",   sqldefSELECTPLUS   }
, {"sequence",     sqldefSEQUENCE     }
, {"server",       sqldefSERVER       }
, {"single",       sqldefSINGLE       }
, {"smallint",     sqldefSMALLINT     }
, {"sql",          sqldefSQL          }
, {"table",        sqldefTABLE        }
, {"tablespace",   sqldefTABLESPACE   }
, {"timestamp",    sqldefTIMESTAMP    }
, {"tinyint",      sqldefTINYINT      }
, {"tlob",         sqldefCLOB         }
, {"to",           sqldefTO           }
, {"unique",       sqldefUNIQUE       }
, {"update",       sqldefUPDATE       }
, {"upper",        sqldefUPPER        }
, {"usedby",       sqldefUSEDBY       }
, {"userstamp",    sqldefUSERSTAMP    }
, {"varnum",       sqldefVARNUM       }
, {"view",         sqldefVIEW         }
, {"xmltype",      sqldefXMLTYPE      }
, {"zlob",         sqldefZLOB         }
};

int KeywordsCount = DIM(KeywordsList);

int my_stricmp(const char *a, const char *b)
{
  int n;
  for (;;)
  {
    n = tolower((uchar)*a) - tolower((uchar)*b);
    if (n || !*a)
       break;
    a++;
    b++;
  }
  return n;
}

static int isSwitch(char *yytext)
{
  register int mid;
  int cc, hi, lo;

  lo = 0;
  hi = SwitchesCount - 1;
  while (lo <= hi)
  {
    mid = (lo + hi) / 2;
    if ((cc = my_stricmp(yytext, SwitchesList[mid].name)) == 0)
    {
      return SwitchesList[mid].token_value;
    }
    if (cc < 0)
      hi = mid - 1;
    else
      lo = mid + 1;
  }
  yylval.s = strdup(yytext);
  return switchINVALID;
}

static int screen(char *yytext)
{
  register int mid;
  int cc, hi, lo;

  lo = 0;
  hi = KeywordsCount - 1;
  while (lo <= hi)
  {
    mid = (lo + hi) / 2;
    if ((cc = my_stricmp(yytext, KeywordsList[mid].name)) == 0)
    {
      return KeywordsList[mid].token_value;
    }
    if (cc < 0)
      hi = mid - 1;
    else
      lo = mid + 1;
  }
  yylval.s = strdup(yytext);
  return sqldefIDENT;
}

enum {stateNORMAL, stateCURSOR, stateCODE, stateDATA};
static int state;

int inbuf_lex(char* InBuf, int& BufIdx)
{
  int i,w;
  char work[20];
  if (yyInit == 1)
  {
    yyInit = 0;
    state = stateNORMAL;
  }
  memset(TokenBuffer, 0, sizeof(TokenBuffer));
  char *out = TokenBuffer;
  char *in  = InBuf+BufIdx;
try_again:
  switch(state)
  {
  case stateNORMAL:
    // clear white space
    while (isspace(in[0]) || in[0] == ';')
    {
      if (in[0] == '\n')
      {
        yylineno++;
        yycolumn = 0;
      }
      in++;
      yycolumn++;
    }
    // check for end of data
    if (in[0] == 0)
    {
      return 0;
    }
    // large or line comments
    if (in[0] == '/')
    {
      in++;
      yycolumn++;
      if (in[0] == '/')
      {
        while (in[0] != 0 && in[0] != '\n')
        {
          in++;
          yycolumn++;
        }
        goto try_again;
      }
      if (in[0] == '*')
      {
        in++;
        yycolumn++;
        //while (in[0] != 0 && in[0] != '*' && in[1] != 0 && in[1] != '/')
        while (in[0] != 0)
        {
          if (in[0] == '*' && in[1] == '/')
            break;
          if (in[0] == '\n')
          {
            yylineno++;
            yycolumn = 0;
          }
          in++;
          yycolumn++;
        }
        if (in[0] != 0)
        {
          in+=2;
          yycolumn+=2;
        }
        goto try_again;
      }
      BufIdx = in - InBuf;
      return '/';
    }
    // # line comment
    if (in[0] == '#')
    {
      in++;
      yycolumn++;
      while (in[0] != 0 && in[0] != '\n')
      {
        in++;
        yycolumn++;
      }
      goto try_again;
    }
    // string
    if (in[0] == '\'' || in[0] == '\"')
    {
      char delim = in[0];
      in++;
      yycolumn++;
      for (int i = 0; in[0] != 0 && in[0] != delim; )
      {
        out[i++] = in[0];
        in++;
        yycolumn++;
      }
      in++;
      yycolumn++;
      BufIdx = in - InBuf;
      yylval.s = strdup(out);
      return sqldefSTRING;
    }
    // extended
    if (in[0] == '@' && isalpha(in[1]))
    {
      out[0] = in[0];
      out[1] = in[1];
      in += 2;
      yycolumn += 2;
      for (int i=2; isalnum(in[0])||in[0]=='_'||in[0]==':'; in++, yycolumn++)
        out[i++] = in[0];
      BufIdx = in - InBuf;
      yylval.s = strdup(out);
      return sqldefEXTENDED;
    }
    // literal quoted ident  # allow for ' and " and a closing quote to be present as well
    if ((in[0] == 'c' || in[0] == 'C') && (in[1] == '\'' || in[1] == '\"') && in[1] == in[3])
    {
      out[0] = in[2];
      in += 4;
      yycolumn += 4;
      BufIdx = in - InBuf;
      yylval.i = out[0];
      return sqldefCHARCONST;
    }
    // literal quoted ident  # allow for ' and " and a closing quote to be present as well
    if ((in[0] == 'l' || in[0] == 'L') && (in[1] == '\'' || in[1] == '\"') && isalpha(in[2]))
    {
      char delim = in[1];
      in += 2;
      yycolumn += 2;
      for (int i=0; isalnum(in[0])||in[0]=='_'||in[0]=='#'||in[0]=='$'; in++, yycolumn++)
        out[i++] = in[0];
      if (in[0] == delim)
        in++;
      BufIdx = in - InBuf;
      yylval.s = strdup(out);
      return sqldefIDENT;
    }
    if (in[0]=='$')
    {
      in++;
      yycolumn++;
      for (int i=0; isalnum(in[0])||in[0]=='_'; in++, yycolumn++)
        out[i++] = in[0];
      BufIdx = in - InBuf;
      return isSwitch(out);
    }
    // Ident, Keyword or StateSwitch
    if (isalpha(in[0])||in[0]=='_')
    {
      for (int i=0; isalnum(in[0])||in[0]=='_'; in++, yycolumn++)
        out[i++] = in[0];
      BufIdx = in - InBuf;
      if (stricmp(out, "cursor")==0)
      {
        state = stateCURSOR;
        return sqldefCURSOR;
      }
      if (stricmp(out, "code")==0)
      {
        state = stateCODE;
        return sqldefCODE;
      }
      if (stricmp(out, "sqlcode")==0)
      {
        state = stateCODE;
        return sqldefSQLCODE;
      }
      if (stricmp(out, "data")==0 || stricmp(out, "sqldata")==0)
      {
        state = stateDATA;
        return sqldefDATA;
      }
      return screen(out);
    }
    if (isdigit(in[0]))
    {
      for (i=0; isdigit(in[0]); in++, yycolumn++)
        out[i++] = in[0];
      BufIdx = in - InBuf;
      yylval.i = atol(out);
      return sqldefNUMBER;
    }
    BufIdx = (in - InBuf) + 1;
    return in[0];
  case stateCURSOR:
    // check for end of data
    if (in[0] == 0)
    {
      return 0;
    }
    if (in[0] == '\r')
    {
      in++;
      yycolumn++;
    }
    if (in[0] == '\n')
    {
      yylineno++;
      in++;
      yycolumn = 1;
      goto try_again;
    }
    if (in[0] == ';')
    {
      state = stateNORMAL;
      BufIdx = (in - InBuf) + 1;
      return sqldefENDCURSOR;
    }
    for (i=0; in[0] != '\n' && in[0] != ';' && in[0] != 0; in++, yycolumn++)
    {
      if (in[0] != '\r')
        out[i++] = in[0];
    }
    BufIdx = in - InBuf;
    yylval.s = strdup(out);
    return sqldefLINE;
  case stateCODE:
    // check for end of data
    if (in[0] == 0)
    {
      return 0;
    }
    if (in[0] == '\r')
    {
      in++;
      yycolumn++;
    }
    if (in[0] == '\n')
    {
      yylineno++;
      in++;
      yycolumn = 1;
      goto try_again;
    }
    work[0] = 0;
    for (i=0, w=0; in[0] != '\n' && in[0] != 0; in++, yycolumn++)
    {
      if (in[0] != '\r')
        out[i++] = in[0];
      if (!isspace(in[0]) && w < sizeof(work)-1)
      {
        work[w++] = in[0];
        work[w] = 0;
      }
    }
    if (stricmp(work, "endcode") == 0)
    {
      state = stateNORMAL;
      BufIdx = in - InBuf;
      return sqldefENDCODE;
    }
    BufIdx = in - InBuf;
    yylval.s = strdup(out);
    return sqldefLINE;
  case stateDATA:
    // check for end of data
    if (in[0] == 0)
    {
      return 0;
    }
    if (in[0] == '\r')
    {
      in++;
      yycolumn++;
    }
    if (in[0] == '\n')
    {
      yylineno++;
      in++;
      yycolumn = 1;
      goto try_again;
    }
    work[0] = 0;
    for (int i=0, w=0; in[0] != '\n' && in[0] != 0; in++, yycolumn++)
    {
      if (in[0] != '\r')
        out[i++] = in[0];
      if (!isspace(in[0]) && w < sizeof(work)-1)
      {
        work[w++] = in[0];
        work[w] = 0;
      }
    }
    if (stricmp(work, "enddata") == 0)
    {
      state = stateNORMAL;
      BufIdx = in - InBuf;
      return sqldefENDDATA;
    }
    BufIdx = in - InBuf;
    yylval.s = strdup(out);
    return sqldefLINE;
  }
  return 0;
}


