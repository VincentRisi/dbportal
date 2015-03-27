#ifndef __ADDLIST_H__
#define __ADDLIST_H__

#include "xcept.h"

struct XAddList : public xCept
{
  XAddList(const char *aFile, int aLine, int aErrNo, const char *aError, uint32 aNumber)
  : xCept(aFile, aLine, "XAddList", aErrNo)
  { osErr << aError << " " << aNumber << endl << ends; }
};
#define XADDLIST(errno, error, number) XAddList(__FILE__, __LINE__, errno, error, number)

typedef int (*fptr)(const void*, const void*);
template <class TELEMENT, class TINDEX>
struct TAddList
{
  int (*compare)(TELEMENT *a, TELEMENT *b);
  TAddList(uint32 aAllocCount=32) {list = 0; count=0; compare=0; allocCount=aAllocCount>0?aAllocCount:32;}
  virtual ~TAddList() {if (list) free(list);}
  void add(TELEMENT& rec)
  {
    if (count % allocCount == 0)    // if count == 0 || count == allocCount
    {
      if (count > 0)                // if count = 0 then allocCount else count == allocCount and allocCount doubles
        allocCount += count;
      if (count > 0 || list == 0)   // if allocation is required
        list = (TELEMENT*) realloc(list, sizeof(rec)*(allocCount));
      if (list == 0)                // failure to allocate
        throw XADDLIST(1, "Memory Allocation Failure", sizeof(rec)*allocCount);
    }
    list[count++] = rec;            // will shallow copy; if you have a copy constructor then beware of leaks
  }
  TINDEX getCount() {return count;}
  TELEMENT* getList() {return list;}
  void remove(TINDEX index)
  {
    if (index < count)
    {
      count--;
      TINDEX i;
      for (i=index; i<count; i++)
        list[i] = list[i+1];
    }
    else
      throw XADDLIST(3, "Deletion of non existing item", index);
  }
  void clear()
  {
    count = 0;
  }
  int search(TELEMENT* lookup)
  {
    if (compare == 0)
      throw XADDLIST(4, "No sort/search compare function defined", count);
    if (list == 0)
      return -1;
    TELEMENT* found = (TELEMENT*)bsearch(lookup, list, (int)count, sizeof(TELEMENT), (fptr)compare);
    if (found)
      return found - list;
    return -1;
  }
  void sort()
  {
    if (compare == 0)
      throw XADDLIST(4, "No sort/search compare function defined", count);
    if (list != 0 && count > 1)
      qsort(list, (int)count, sizeof(TELEMENT), (fptr)compare);
  }
  TELEMENT& operator [](TINDEX i)
  {
    if (list != 0 && (i >= count || i < 0))
      throw XADDLIST(2, "Accessing out of range", i);
    return list[i];
  }
private:
  TINDEX allocCount;
  TELEMENT* list;
  TINDEX count;
};

#endif

