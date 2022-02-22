#ifndef _MACHINE_H_
#define _MACHINE_H_

typedef struct
{
  char p[2];
} byte2;

typedef struct
{
  char p[4];
} byte4;

typedef struct
{
  char p[8];
} byte8;

#if defined(__WIN32__) && defined(__BORLANDC__)
   #define _WINDOWS
   #define M_NT
   #define M_W32
   #define M_HASBOOL
   #define APPLAPI __export __stdcall
   #define APC
   #if __BORLANDC__ > 0x530
      #ifndef _WINSOCKAPI_
         #include <winsock2.h>
      #endif
   #endif
   typedef __int8  int8;
   typedef __int16 int16;
   typedef __int32 int32;
   typedef __int64 int64;
   typedef unsigned __int8  uint8;
   typedef unsigned __int16 uint16;
   typedef unsigned __int32 uint32;
   typedef unsigned __int64 uint64;
   #define atoint64 _atoi64
   #define _strupr strupr
   #define _strlwr strlwr
#elif defined(__MSVC__) || _MSC_VER >= 1200
  #if _MSC_VER >= 1500
    #define _CRT_NONSTDC_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #elif _MSC_VER >= 1400
    #define _CRT_NONSTDC_NO_WARNINGS
   #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #elif _MSC_VER >= 1300
    #define access    _access
    #define unlink    _unlink 
    #define snprintf  _snprintf
    #define vsnprintf _vsnprintf
    #define strdup    _strdup
    #define strupr    _strupr
    #define strlwr    _strlwr
    #define tempnam   _tempnam
  #else
    #define access    _access
    #define unlink    _unlink 
    #define snprintf  _snprintf
    #define vsnprintf _vsnprintf
    #define strdup    _strdup
    #define strupr    _strupr
    #define strlwr    _strlwr
    #define tempnam   _tempnam
  #endif
  #ifndef _WINDOWS
    #define _WINDOWS
  #endif
  #ifndef M_STDLIB
    #define M_STDLIB
  #endif
   #ifndef _WINDOWS
      #define _WINDOWS
   #endif

   #ifndef M_STDLIB
      #define M_STDLIB
   #endif
   #define M_NT
   #define M_W32
   #define M_HASBOOL
   #define APPLAPI
   #define APC
   #include <winsock2.h>


   typedef __int8  int8;
   typedef __int16 int16;
   typedef __int32 int32;
   typedef __int64 int64;
   typedef unsigned __int8  uint8;
   typedef unsigned __int16 uint16;
   typedef unsigned __int32 uint32;
   typedef unsigned __int64 uint64;
   #define atoint64 _atoi64

/* --------------------- GNUC on Linux-------------------------- */
#elif defined(__GNUC__)
   #include <sys/types.h>
   #include <stdint.h>
   #define int8  __int8_t  
   #define int16 __int16_t 
   #define int32 __int32_t 
   #define int64 __int64_t 
   #define uint8  uint8_t
   #define uint16 uint16_t
   #define uint32 uint32_t
   #define uint64 uint64_t

   #define atoint64 atoll
   #define M_GNU
   #ifndef M_STDLIB
      #define M_STDLIB
   #endif

   #define APPLAPI
   #define APC
   #define O_BINARY 0
   #define M_HASBOOL
   #include <stdlib.h>
   #ifdef __cplusplus
   extern "C" {
   #endif
   #include <unistd.h>
   #ifdef __cplusplus
   }
   #endif

/* -------------------------- AIX ------------------------------
// EUREKA WE HAVE FOUNDED IT - AIX51 == VISUALAGE
   ------------------------------------------------------------- */
#elif defined(_AIX)
  typedef __int8 uint8;
  #define atoint64 atoll
  #define M_AIX
  #ifndef M_STDLIB
  #define M_STDLIB
  #endif

  #ifdef _AIX51
  #define M_HASBOOL
  #endif

   #define uint8  u_int8
   #define uint16 u_int16
   #define uint32 u_int32
   #define uint64 u_int64
   
   #define NO_CPP_NEW_ARRAY
   #define APPLAPI
   #define APC
   #define O_BINARY 0

   #define SYSLOG(priority, msg) syslog(priority, "%s FILE - %s LINE - %d", msg, __FILE__, __LINE__)

   #include <pthread.h>
   #include <stdlib.h>
   #include <sys/inttypes.h>
   #ifdef __cplusplus
      extern "C" {
   #endif
   #include <unistd.h>

   #ifdef __cplusplus
      }
   #endif
#else
   #error "UNKNOWN MACHINE"
#endif
#define M_STDLIB  
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
using namespace std;

#ifndef M_HASBOOL
typedef enum bool
{
   false, true
} bool, *pbool;
#else
typedef bool                 *pbool;
#endif

typedef signed   char  schar,  *pschar;
typedef char                   *pchar;
typedef void                   *pvoid;
typedef short                  *pshort;
typedef int                    *pint;
typedef long                   *plong;
typedef float                  *pfloat;
typedef double                 *pdouble;

#ifdef M_AIX
#define ENDIAN 'M'
typedef unsigned char          *puchar;
typedef unsigned short  ushort,*pushort;
typedef unsigned int           *puint;
typedef unsigned long          *pulong;
#elif defined(M_GNU)
#define ENDIAN 'I'
typedef unsigned char   uchar, *puchar;
typedef unsigned short  ushort,*pushort;
typedef unsigned int    uint,  *puint;
typedef unsigned long   ulong, *pulong;
#else
#define ENDIAN 'I'
typedef unsigned char  uchar,  *puchar;
typedef unsigned short ushort, *pushort;
typedef unsigned int   uint,   *puint;
typedef unsigned long  ulong,  *pulong;
typedef long double  ldouble,  *pldouble;
#endif

#if !defined(M_AIX) && !defined(M_GNU)
   #define True  true
   #define False false
   #include <windows.h>
#endif

#endif

