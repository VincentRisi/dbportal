#ifndef _TBUFFER_H_
#define _TBUFFER_H_

/**
 *  $Author: etienned $ This is actually the last modifier.
 *  Author: Vince Risi
 *  $Date: 2005/05/30 14:21:48 $
 *  $Revision: 1.1.1.1 $
 *
 *  This template class only really works well with scalar type like
 *  char, short, int, long. It works with pointers as well (they
 *  are really only in essence scalars (ie. 4 byte ints). It should
 *  be noted that zero in data is not catered for as it is primarily
 *  used for zero terminated vectors like char* in c. See ByteBuffer.h
 *  for a simple bytebuffer manager that stores disparate data, like
 *  length+data+length+otherData.
 *
 *  The purpose for this class is to supply a memory allocation mechanism
 *  that will deallocate when going out of scope. It is also useful for
 *  passing in as a reference parameter for functions that need to
 *  supply variable sized data that is only known at run time.
 *
 */

#include "xcept.h"

#ifdef size
   #undef size
#endif

static const char *BufferErrors[] =
{ 
   "OK ???", 
   "Memory Allocation Failed"
};

/*-----------------------------------
   xBufferCept
-----------------------------------*/
class xBufferCept : public xCept
{
   public:
   
      enum eError
      {
         eOk, 
         eMemoryError
      };
   
      xBufferCept(const char *aFname, const int aLine, const eError aError)
      : xCept(aFname, aLine, "xBufferCept", aError)
      {
         osErr << "TBuffer: " << BufferErrors[aError] << endl << ends;
      }
   
      xBufferCept(const xBufferCept& aX) : xCept(aX) { }
};


#define XBUFFERXCEPT(err) xBufferCept(__FILE__, __LINE__, xBufferCept::err)


/*-----------------------------------
   TBuffer
-----------------------------------*/
template <class T>
struct TBuffer
{
   T* data;
   size_t size, bytelen, used;
   TBuffer(size_t insize=512)
   : size(insize)
   {
      if (size <= 0) size = 512;
      bytelen = size*sizeof(T);
      used = 0;
      data = (T *)calloc(size, sizeof(T));
      if (data == 0)
         throw XBUFFERXCEPT(eMemoryError);
   }

   ~TBuffer()
   {
      if (data)
         free(data);
   }

   operator T*()
   {
      return data;
   }

   virtual void clear()
   {
      used = 0;
      memset(data, 0, bytelen);
   }

   virtual void resize(const size_t minimum)
   {
      if (minimum > size)
         grow(minimum - size);
   }

   virtual void sizeto(const size_t exact)
   {
      size = exact;
      bytelen = size*sizeof(T);
      if (used > exact)
         used = exact;

      T *tmp = (T *)realloc(data, bytelen);

      if (bytelen != 0 && tmp == 0)
         throw XBUFFERXCEPT(eMemoryError);

      data = tmp;
   }

   virtual void grow(const size_t add)
   {
      size_t newsize = size + add;
      bytelen = newsize*sizeof(T);
      T *tmp = (T *)realloc(data, bytelen);

      if (tmp == 0)
         throw XBUFFERXCEPT(eMemoryError);

      data = tmp;

      char *b = (char *) (data + size);
      size = newsize;
      memset(b, 0, add*sizeof(T));
   }

   void set(const T inp)
   {
      T W[2];
      W[0] = inp;
      W[1] = 0;
      set(W,1);
   }

   virtual void set(const T* inp, const size_t insize = 0)
   {
      clear();
      append(inp, insize);
   }

   virtual void append(const T inp)
   {
      T W[2];
      W[0] = inp;
      W[1] = 0;
      append(W, 1);
   }

   virtual void append(const T* inp, size_t insize = 0)
   {
      // If no size given count non zero elements for input size
      if (insize == 0)
         while (inp[insize] != 0)
            insize++;

      for (;(used < size) && (data[used] != 0);used++)
         ;

      // check if enough space to append data
      size_t left = size - used;
      while (left < insize+1)
      {
         left += size;
         grow(size);
      }

      // Append to end moving endpoint
      memcpy(data+used, inp, insize*sizeof(T));
      used += insize;
   }

   virtual size_t toMallocedCharPointer(char *&dest)
   {
      if (size <= 0)
      {
         dest = 0;
         return 0;
      }

      dest = (char *) malloc(used + 1);

      if (!dest)
         throw XBUFFERXCEPT(eMemoryError);

      memcpy(dest, data, used);
      dest[used] = 0;

      return used;
   }

};

/*-----------------------------------
   TBChar
-----------------------------------*/
typedef TBuffer<char> TBChar;


struct TBAmp : public TBChar
{
   TBAmp(size_t insize=512) : TBChar(insize)
   {
   }

   const char* encode(char inp, char *ch)
   {
      ch[0]=inp; ch[1]=0;
      switch (ch[0])
      {
         case '&' : return "&amp;";
         case '<' : return "&lt;";
         case '>' : return "&gt;";
         case '\"': return "&quot;";
         case '\'': return "&apos;";
      }
      return ch;
   }

   size_t neededSize(char inp)
   {
      char work[32];
      return strlen(encode(inp, work));
   }

   void ampappend(char* inp)
   {
      size_t insize=0, extra=0;
      while (inp[insize] != 0)
      {
         extra += neededSize(inp[insize]) - 1;
         insize++;
      }

      size_t left = 0;
      size_t pos, i;

      for (pos = 0; pos < size; pos++)
         if (data[pos] == 0)
         {
            left = size - pos;
            break;
         }

      while (left < insize+extra+1)
      {
         left += size;
         grow(size);
      }

      for (i=0; i<insize; i++)
      {
         char work[32];
         const char *ch = encode(inp[i], work);
         strcpy(data+pos, ch);
         pos += strlen(ch);
      }
   }
};


/*-----------------------------------
   TBEnvChar
-----------------------------------*/
struct TBEnvChar : public TBChar
{
   TBEnvChar(size_t insize=512) : TBChar(insize)
   {
   }

   void set(const char *aValue, const size_t insize=0)
   {
      clear();
      TBChar work1, work2;
      work1.set(aValue);
      char *p=work1.data, *q, *r;

      while (p[0] && (q = strstr(p, "$(")) != 0)
      {
         r = strstr(q, ")");
         size_t n = r-q-2;
         if (r == 0 || n <= 0)
            break;
         if (p != q)
            append(p, q-p);
         work2.set(q+2, n);
         char *e = getenv(work2);
         if (e)
            append(e);
         p = r+1;
      }

      if (p && *p)
         append(p);
   }
};

typedef TBuffer<unsigned char> TBUChar, TBByte;
typedef TBuffer<pvoid> TBVector;

#undef XBUFFERXCEPT

#endif

