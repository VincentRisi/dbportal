#include <stdlib.h>
#include <stdarg.h>

#include "xcept.h"
#include "xstring.h"

xCept::xCept(const char       *aFname, 
             const int         aLine, 
             const char       *aName,
             const int         aError, 
             const eSeverity   aSeverity) 
      :line(aLine),
       error(aError),
       severity(aSeverity)
#if defined(M_STDLIB)
       //,osErr()
#else
       ,osErr(errorStr, sizeof(errorStr))
#endif
{
   strncpyz(fname, aFname, sizeof(fname) - 1);
   strncpyz(name,  aName,  sizeof(name) - 1);
   *errorStr = 0;
}

xCept::xCept(const xCept& x) 
      :line(x.line),
       error(x.error),
       severity(x.severity),
#if defined(M_STDLIB)
       osErr(x.osErr.str())
#else
       osErr(errorStr, sizeof(errorStr))
#endif
{
   strcpy(fname,    x.fname);
   strcpy(name,     x.name);
   strcpy(errorStr, x.errorStr);
}

void xCept::Display(std::ostream& os, const eMode mode)
{
#if !defined(M_STDLIB)
   // add null terminator
   if (errorStr[osErr.pcount() - 1])
      osErr << ends;
#endif
   if (mode == modeVerbose)
   {
      if (severity == sevFatal)
         os << "Fatal Error ";
      else
         os << "Error ";
      os << name << " in "<< fname << "(" << line << ") " << error << ": ";
   }

   os << errorStr << endl << ends;
}


/*------------------------------------------------------------------------------
   xBase Constructor
------------------------------------------------------------------------------*/
xBase::xBase(const char *file, const int line, const char *fmt, ...)
      :xCept(file, line, "xBase", -1)
{
   va_list ap;
   char    jotter[1024];

   va_start(ap, fmt);

   vsnprintf(jotter, sizeof(jotter) - 1, fmt, ap);

   va_end(ap);

   osErr << jotter << endl << ends;
}



