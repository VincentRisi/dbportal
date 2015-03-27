/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SQLDEF_H_INCLUDED
# define YY_YY_SQLDEF_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     sqldefALL = 258,
     sqldefAUDIT = 259,
     sqldefBINARY = 260,
     sqldefBLOB = 261,
     sqldefBOOLEAN = 262,
     sqldefCASCADE = 263,
     sqldefCHAR = 264,
     sqldefCHARCONST = 265,
     sqldefCHECK = 266,
     sqldefCLOB = 267,
     sqldefCODE = 268,
     sqldefCONNECT = 269,
     sqldefCOUNT = 270,
     sqldefCURSOR = 271,
     sqldefDATETIME = 272,
     sqldefDATA = 273,
     sqldefDATE = 274,
     sqldefDECLARE = 275,
     sqldefDELETE = 276,
     sqldefDELETEALL = 277,
     sqldefDELETEONE = 278,
     sqldefENDCODE = 279,
     sqldefENDCURSOR = 280,
     sqldefENDDATA = 281,
     sqldefEXCEPTION = 282,
     sqldefEXECUTE = 283,
     sqldefEXISTS = 284,
     sqldefEXTENDED = 285,
     sqldefFAST = 286,
     sqldefFLOAT = 287,
     sqldefFREELIST = 288,
     sqldefFREELISTS = 289,
     sqldefFROM = 290,
     sqldefFULL = 291,
     sqldefGRANT = 292,
     sqldefGROUPS = 293,
     sqldefHUGECHAR = 294,
     sqldefIDENT = 295,
     sqldefINITIAL = 296,
     sqldefIMAGE = 297,
     sqldefINITRANS = 298,
     sqldefINOUT = 299,
     sqldefINPUT = 300,
     sqldefINSERT = 301,
     sqldefINT = 302,
     sqldefK = 303,
     sqldefKEY = 304,
     sqldefLINE = 305,
     sqldefLINK = 306,
     sqldefLONG = 307,
     sqldefM = 308,
     sqldefMAXEXTENTS = 309,
     sqldefMAXTRANS = 310,
     sqldefMINEXTENTS = 311,
     sqldefMONEY = 312,
     sqldefMULTIPLE = 313,
     sqldefNEXT = 314,
     sqldefNOC = 315,
     sqldefNOEXT = 316,
     sqldefNOIDL = 317,
     sqldefNOPAS = 318,
     sqldefNOT = 319,
     sqldefNOVB = 320,
     sqldefNULL = 321,
     sqldefNUMBER = 322,
     sqldefOPTIMAL = 323,
     sqldefOUTPUT = 324,
     sqldefPCTFREE = 325,
     sqldefPCTINCREASE = 326,
     sqldefPCTUSED = 327,
     sqldefPRIMARY = 328,
     sqldefPROC = 329,
     sqldefSCHEMA = 330,
     sqldefSELECT = 331,
     sqldefSELECTALL = 332,
     sqldefSELECTALLUPD = 333,
     sqldefSELECTONE = 334,
     sqldefSELECTONEUPD = 335,
     sqldefSELECTPLUS = 336,
     sqldefSEQUENCE = 337,
     sqldefSERVER = 338,
     sqldefSINGLE = 339,
     sqldefSMALLINT = 340,
     sqldefSQL = 341,
     sqldefSQLCODE = 342,
     sqldefSTRING = 343,
     sqldefTABLE = 344,
     sqldefTABLESPACE = 345,
     sqldefTIMESTAMP = 346,
     sqldefTINYINT = 347,
     sqldefTO = 348,
     sqldefUNIQUE = 349,
     sqldefUPDATE = 350,
     sqldefUPPER = 351,
     sqldefUSEDBY = 352,
     sqldefUSERSTAMP = 353,
     sqldefVARNUM = 354,
     sqldefVIEW = 355,
     sqldefXMLTYPE = 356,
     sqldefZLOB = 357,
     switchBINDIR = 358,
     switchBINEXT = 359,
     switchBLANKPADDED = 360,
     switchCDIR = 361,
     switchCEXT = 362,
     switchCHARZ = 363,
     switchCONNREQD = 364,
     switchCONTROLDB = 365,
     switchCPPHEADER = 366,
     switchEXITREQD = 367,
     switchEIGHTBYTE = 368,
     switchEXTENDEDC = 369,
     switchEXTENDEDPAS = 370,
     switchEXTENDEDVB = 371,
     switchHEADERDIR = 372,
     switchHEADEREXT = 373,
     switchIDLDIR = 374,
     switchIDLEXT = 375,
     switchIDLMODULE = 376,
     switchIDLUBI = 377,
     switchINTERNAL = 378,
     switchVERSION2BIN = 379,
     switchLITTLETRUE = 380,
     switchLOGDIR = 381,
     switchLOGEXT = 382,
     switchNODOMAIN = 383,
     switchONESQLSCRIPT = 384,
     switchPASDIR = 385,
     switchPASEXT = 386,
     switchPYTHONDIR = 387,
     switchPYTHONEXT = 388,
     switchSHOW = 389,
     switchSODIR = 390,
     switchSOEXT = 391,
     switchSQLAUDITEXT = 392,
     switchSQLCONEXT = 393,
     switchSQLDIR = 394,
     switchSQLEXT = 395,
     switchSQLGRANTEXT = 396,
     switchSQLIDXEXT = 397,
     switchSQLPROCEXT = 398,
     switchSQLSNAPEXT = 399,
     switchSQLTABLEEXT = 400,
     switchSQLVIEWSEXT = 401,
     switchTARGETC = 402,
     switchTARGETIDL = 403,
     switchTARGETPAS = 404,
     switchARCHIVEDIR = 405,
     switchARCHIVEEXT = 406,
     switchPARMSDIR = 407,
     switchPARMSDESCR = 408,
     switchPARMSEXT = 409,
     switchPARMSLOOKUP = 410,
     switchTARGETARCHIVE = 411,
     switchTARGETPARMS = 412,
     switchTARGETPYTHON = 413,
     switchTARGETVB = 414,
     switchTARGETVB5 = 415,
     switchUCONNECT = 416,
     switchUNDERSCORE = 417,
     switchVB5DIR = 418,
     switchVB5EXT = 419,
     switchVBDIR = 420,
     switchVBEXT = 421,
     switchVBFORIDLDIR = 422,
     switchINVALID = 423,
     switchPREFIXVBCLASSES = 424,
     switchTARGETVBCODE3 = 425,
     switchTARGETVBNET7 = 426,
     switchTARGETCSNET7 = 427,
     switchTARGETCSIDL2 = 428,
     switchTARGETCSADONET = 429,
     switchTARGETCSRW = 430,
     switchVBCODE3DIR = 431,
     switchVBCODE3EXT = 432,
     switchVBNET7DIR = 433,
     switchVBNET7EXT = 434,
     switchCSNET7DIR = 435,
     switchCSADONETDIR = 436,
     switchCSRWDIR = 437,
     switchCSIDL2DIR = 438,
     switchCSNET7EXT = 439,
     switchCSADONETEXT = 440,
     switchCSRWEXT = 441,
     switchCSIDL2EXT = 442,
     switchVIEWONLY = 443,
     switchTARGETVBFORIDL = 444,
     switchTARGETVBFORADOR = 445,
     switchLITTLETCSIDL2 = 446
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


  int   i;
  char* s;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_SQLDEF_H_INCLUDED  */
