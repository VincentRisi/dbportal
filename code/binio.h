#ifndef __BINIO_H_
#define __BINIO_H_
#include "machine.h"

inline void _Swap(char &a, char &b) {a ^= b ^= a ^= b;}
const int32 doSwap=0x03020100;

#ifdef __BORLANDC__
#pragma option -w-ccc -w-rch
#endif
template <class T>
void Swap(T &no, int size)
{
  if (*(char*)&doSwap == 0) // seen as 0x00010203 (perhaps)
  {
    char* p = (char*)&no;
    if (size == 2)
      _Swap(p[0], p[1]);
    else if (size == 4)
    {
      _Swap(p[0], p[3]);
      _Swap(p[1], p[2]);
    }
    else if (size == 8)
    {
      _Swap(p[0], p[7]);
      _Swap(p[1], p[6]);
      _Swap(p[2], p[5]);
      _Swap(p[3], p[4]);
    }
  }
}
#ifdef __BORLANDC__
#pragma option -wccc -wrch
#endif

char GetSChar(FILE* BinFile);
pchar GetString(FILE* BinFile);
pchar GetExact(ushort Len, FILE* BinFile);
uint16 GetUShort(FILE* BinFile);
int16 GetShort(FILE* BinFile);
uint32 GetULong(FILE* BinFile);
int32 GetLong(FILE* BinFile);
double GetDouble(FILE* BinFile);

extern int BinioVerbose;   /*  set to 1 in order to display Put.... to stdout */
extern FILE *BinioLogFile;

void PutSChar(char c, FILE* BinFile);
void PutString(pchar s, FILE* BinFile);
void PutExact(pchar s, FILE* BinFile);
void PutUShort(uint16 i, FILE* BinFile);
void PutShort(int16 i, FILE* BinFile);
void PutULong(uint32 i, FILE* BinFile);
void PutLong(int32 i, FILE* BinFile);
void PutDouble(double i, FILE* BinFile);

#endif

