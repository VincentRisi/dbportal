#ifndef __PORTAL_EXTRACT_H
#define __PORTAL_EXTRACT_H  "$Revision: 92 $ $Date: 2005-04-21 11:34:40 +0200 (Thu, 21 Apr 2005) $"
#include "versions.h"
HEADER_VERSION(__PORTAL_EXTRACT_H);

#ifndef _PORTAL_EXTRACT_VALUE_LEN
   #define _PORTAL_EXTRACT_VALUE_LEN 256
#endif

#include "appendlist.h"
#include "xcept.h"

/*-----------------------------------
   Forward Declarations
-----------------------------------*/
class tDBConnect;


/*-----------------------------------
   Exception Class
-----------------------------------*/
class xPortalExtract : public xCept
{
   public :
      xPortalExtract(const char *file, const int line, const char *fmt, ...);
};


/*-----------------------------------
   Data Mapper
-----------------------------------*/
class PortalExtract
{
   public :

      typedef struct
      {
         char Name[128];
         char Value[_PORTAL_EXTRACT_VALUE_LEN];

         union
         {
            short   shortValue;
            long    longValue;
            double  doubleValue;
         };
      } MapField;

      PortalExtract(tDBConnect &dbConnect);
      ~PortalExtract();

      void Extract(const void           *portalStruct, 
                   const char           *SqlQuery, 
                   AppendList<MapField> *Fields);

      void Fill(void                       *portalStruct,
                const char                 *SqlQuery,
                const AppendList<MapField> *Fields);

	  void Convert(const void *fromPortalStruct, 
                  const char *fromSqlQuery,
                  void       *toPortalStruct,
				      const char *toSqlQuery);

   private :

      tDBConnect       &m_DBConnect;
};



#endif
