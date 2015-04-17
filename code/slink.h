#ifndef _slink_h_
#define _slink_h_

#include "sqlbin.h"

extern ushort sqldefSign;
extern ushort slinkSign;
extern ushort sqldefSign2;
extern ushort slinkSign2;
extern ushort sqldefTailMark;

extern ushort NoQueries;
extern PSqlQuery *Queries;

typedef int (*fptr)(const void*, const void*);
int SortComp(const PSqlQuery *p1, PSqlQuery *p2);

void LoadInFile(const char *infile, ushort &sign);
void OpenBinFile(const char *path, const char *BinDir, const char *BinExt);
void StoreBinFile(ushort sign);

#endif