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

char getInt8(FILE* BinFile);
pchar getString(FILE* BinFile);
pchar getExact(ushort Len, FILE* BinFile);
uint16 getUInt16(FILE* BinFile);
int16 getInt16(FILE* BinFile);
uint32 getUInt32(FILE* BinFile);
int32 getInt32(FILE* BinFile);
double getDouble(FILE* BinFile);

extern int binio_verbose;   /*  set to 1 in order to display Put.... to stdout */
extern FILE *binio_log_file;

void putInt8(char c, FILE* BinFile);
void putString(pchar s, FILE* BinFile);
void putExact(pchar s, FILE* BinFile);
void putUInt16(uint16 i, FILE* BinFile);
void putInt16(int16 i, FILE* BinFile);
void putUInt32(uint32 i, FILE* BinFile);
void putInt32(int32 i, FILE* BinFile);
void putDouble(double i, FILE* BinFile);

#endif

