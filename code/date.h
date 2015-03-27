#ifndef _DATE_H_
#define _DATE_H_
#include "machine.h"

#define CUTOFF_DATE 50
#include <time.h>
/*
** Please note this is a Jul date and not a full on Julian Date as the full on
** Date must be defined as a floating point number where the the day starts at
** 12:00 Midday and not 12:00 Midnight.
*/

bool LeapYear(int16 Year);
/*  This function returns a true if the Year Is Leap */

int32 JulDate(int16 day, int16 month, int16 year);
/*  This function returns a int32 of the Number of days since dot. */

void CalDate(int32 jul, pshort day, pshort month, pshort year);
/*  This function returns the day month year from a int32 of the days since */
/*  dot. */

int32 CurrentDate(void);
/*  This function returns a int32 of the current date as 19YYMMDD. */

int32 CurrentTime(void);
/*  This function returns a int32 of the current date as HHMMSS. */

int32 FromOracleDate(uchar *oradate);
/*  This function returns a int32 of the OCI char[7] date as YYYYMMDD. */

void ToOracleDate(uchar *oradate, int32 yyyymmdd, int32 hhmmss);
/*  This function fill in oradate as OCI char[7] from a int32 date YYYYMMDD. */
/*  and int32 time HHMMSS if time is zero a default will be taken */
/*  HH(1-24)MM(1-60)SS(1-60) eg. 1 1 1 = midnight */

pchar JulToOracle(int32 aJul, pchar aOrcleDate);
/*  This function fill in oradate as OCI char[7] YYYYMMDD) from a Jul date. */

int32 OracleToJul(pchar aOrcleDate);
/*  This function returns a int32 of the Number of days since */
/* OracleDate Format YYYYMMDD. */

pchar LocalTimeToOracle(pchar TimeString, int Len=15);
/*  This function returns a char int OracleDate Format YYYYMMDDHHMMSS */
/*  What the function actually does is to take the current date and time */
/*  Format them as mentioned into the string provided with max len of Len */
/*  and I assume it probably returns the string u passed in. */

/*
* This function returns the day type and the day of the week
*
*  Day Types
*/
#define DAY_TYPE_WEEKDAY    1
#define DAY_TYPE_SATURDAY   2
#define DAY_TYPE_HOLIDAY    3
#define DAY_TYPE_SUNDAY     4


void QueryDayType(pchar TimeString,int32 * pDayType,int32 * pDayOfWeek);

typedef struct {
   char Year[4];
   char Month[2];
   char Day[2];
   char Hour[2];
   char Minute[2];
   char Second[2];
} tOracleDate, *pOracleDate;

/*
* this function add one month to a oracle format Date 'YYYYMMDD'
*/
pchar DateAddMonth(pchar Date, pchar NewDate);


//***** Don't comment the following out without speaking to Simon/Diricia *****
/*
* Convert time in sec from 1 Jan 1970 to OracleDate 'YYYYMMDDHHMISS'
*/
void TimetToOracle(time_t UnixTime, pchar aOracleDate );

/*
* Returns the time in sec from 1 Jan 1970 if given an OracleDate 'YYYYMMDDHHMISS'
*/
time_t OracleToTimet( pchar aOracleDate );

#endif

