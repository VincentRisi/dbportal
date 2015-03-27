#include "date.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <stdlib.h>

/* * Calculates the julian date of a given date * */
/* * Collected Algorithms of the ACM, Algorithm #199. * */
/* * Robert B. Tantzen. New York: ACM, 1980. * */

/* * works for dates from 01/01/0001 to 31/12/9999 * */

bool LeapYear(int16 Year)
{
  if ((Year % 400) == 0)
    return (true);
  if ((Year % 100) == 0)
    return (false);
  if ((Year % 4) == 0)
    return (true);
  return (false);
}

int32 JulDate(int16 day, int16 month, int16 year)
{
  int32 lYear, lMonth, lDay, lCentury, lCy;

  lDay = day;
  if (year < 1)
    year = 1;
  lYear = year;

  if (month > 2)
    lMonth = month - 3;
  else
  {
    lMonth = month + 9;
    lYear = year - 1;
  }
  lCentury = lYear / 100;
  lCy = lYear - 100 * lCentury;
  return (146097 * lCentury) / 4 + (1461 * lCy) / 4 + (153 * lMonth + 2) / 5 + lDay + 1721119;
}

void CalDate(int32 jul, pshort day, pshort month, pshort year)
{
  int32 lTemp = jul - 1721119;
  int32 lYear, lMonth, lDay;
  lYear = (4 * lTemp - 1) / 146097;
  lTemp = 4 * lTemp - 1 - 146097 * lYear;
  lDay = lTemp / 4;
  lTemp = (4 * lDay + 3) / 1461;
  lDay = 4 * lDay + 3 - 1461 * lTemp;
  lDay = (lDay + 4) / 4;
  lMonth = (5 * lDay - 3) / 153;
  lDay = 5 * lDay - 3 - 153 * lMonth;
  lDay = (lDay + 5) / 5;
  lYear = 100 * lYear + lTemp;

  if (lMonth < 10)
    lMonth += 3;
  else
  {
    lMonth -= 9;
    lYear++;
  }

  * day = (int)lDay;
  * month = (int)lMonth;
  * year = (int)lYear;
}

int32 CurrentDate()
{
  time_t tt;
  struct tm * lt;

  time(& tt);
  lt = localtime(& tt);
  return (19000000L + 10000L * (int32)lt->tm_year + 100L * ((int32)lt->tm_mon + 1) + (int32)lt->tm_mday);
}

int32 CurrentTime()
{
  time_t tt;
  struct tm * lt;

  time(& tt);
  lt = localtime(& tt);
  return (10000L * (int32)lt->tm_hour + 100L * ((int32)lt->tm_min) + (int32)lt->tm_sec);
}

int32 FromOracleDate(uchar * oradate)
{
  int32 date;
  date = (int32)(oradate[0] - 100) * 1000000L + (int32)(oradate[1] - 100) * 10000L;
  if (date >= 0)
    date += ((int32)oradate[2] * 100L + (int32)oradate[3]);
  else
    date -= ((int32)oradate[2] * 100L + (int32)oradate[3]);
  return date;
}

void ToOracleDate(uchar * oradate, int32 yyyymmdd, int32 hhmmss)
{
  int BC = yyyymmdd < 0;
  if (hhmmss)
  {
    oradate[6] = (uchar)(hhmmss % 100);
    hhmmss /= 100;
    oradate[5] = (uchar)(hhmmss % 100);
    hhmmss /= 100;
    oradate[4] = (uchar)(hhmmss % 100);
  }
  else
    oradate[6] = oradate[5] = oradate[4] = 1;
  if (BC)
    yyyymmdd = -yyyymmdd;
  oradate[3] = (uchar)(hhmmss % 100);
  yyyymmdd /= 100;
  oradate[2] = (uchar)(hhmmss % 100);
  yyyymmdd /= 100;
  oradate[1] = (uchar)(hhmmss % 100);
  yyyymmdd /= 100;
  oradate[0] = (uchar)(hhmmss % 100);
  if (BC)
  {
    oradate[1] = 100 - oradate[1];
    oradate[0] = 100 - oradate[0];
  }
  else
  {
    oradate[1] += 100;
    oradate[0] += 100;
  }
}

pchar JulToOracle(int32 aJul, pchar aOracleDate)
{
  int16 day, month, year;
  CalDate(aJul, & day, & month, & year);
  sprintf(aOracleDate, "%04d%02d%02d", (int)year, (int)month, (int)day);
  return aOracleDate;
}

int32 OracleToJul(pchar aOracleDate)
{
  int day, month, year;
  int32 Jul;
  sscanf(aOracleDate, "%4d%2d%2d", & year, & month, & day);
  Jul = JulDate(day, month, year);
  return Jul;
}

/* This function returns a char int OracleDate Format YYYYMMDDHHMMSS */
pchar LocalTimeToOracle(pchar TimeString, int len)
{
  time_t TimeInSecs;
  struct tm * TM;

  time(& TimeInSecs);
  TM = localtime(& TimeInSecs);
  if (len == 8)
    sprintf(TimeString, "%04d%02d%02d", 1900 + TM->tm_year, TM->tm_mon + 1, TM->tm_mday);
  else
    sprintf(TimeString, "%04d%02d%02d%02d%02d%02d", 1900 + TM->tm_year, TM->tm_mon + 1, TM->tm_mday, TM->tm_hour,
       TM->tm_min, TM->tm_sec);
  return TimeString;
}

/* * This function returns the day type and the day of the week */
void QueryDayType(pchar TimeString, int32 * pDayType, int32 * pDayOfWeek)
{
  //time_t TimeInSecs;
  struct tm TM;
  char Jotter[20];

  memset(& TM, 0, sizeof(TM));
  memcpy(Jotter, & (TimeString[0]), 4);
  Jotter[4] = 0x00;
  TM.tm_year = atoi(Jotter); //Use 4 digit year
  TM.tm_year -= 1900; //Subtract 1900 - Y2K compliant
  memcpy(Jotter, & (TimeString[4]), 2);
  Jotter[2] = 0x00;
  TM.tm_mon = atoi(Jotter);
  memcpy(Jotter, & (TimeString[6]), 2);
  Jotter[2] = 0x00;
  TM.tm_mday = atoi(Jotter);
  TM.tm_mon--;

  //TimeInSecs = mktime(& TM);

  switch (TM.tm_wday)
  {
    case 0:
      * pDayType = DAY_TYPE_SUNDAY;
      break;
    case 1:
      * pDayType = DAY_TYPE_WEEKDAY;
      break;
    case 2:
      * pDayType = DAY_TYPE_WEEKDAY;
      break;
    case 3:
      * pDayType = DAY_TYPE_WEEKDAY;
      break;
    case 4:
      * pDayType = DAY_TYPE_WEEKDAY;
      break;
    case 5:
      * pDayType = DAY_TYPE_WEEKDAY;
      break;
    case 6:
      * pDayType = DAY_TYPE_SATURDAY;
      break;
  }
  * pDayOfWeek = TM.tm_wday;
}

/* * this function add one mont to a oracle format Date 'YYYYMMDD' */
pchar DateAddMonth(pchar Date, pchar NewDate)
{
  int Day;
  int Month;
  int Year;
  char Jotter[20];
  int Days;
  // 1 2 3 4 5 6 7 8 9 10 11 12
  int MonthDays[] =
  {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  };

  memcpy(Jotter, & Date[4], 2);
  Jotter[2] = 0x00;
  Month = atoi(Jotter);
  memcpy(Jotter, & Date[6], 2);
  Jotter[2] = 0x00;
  Day = atoi(Jotter);
  memcpy(Jotter, & Date[0], 4);
  Jotter[4] = 0x00;
  Year = atoi(Jotter);

  int32 Jul = OracleToJul(Date);

  if (Month == 2)
  {
    if (Year % 4 == 0)
    {
      MonthDays[Month] = 29;
    }
  }

  if (Day == MonthDays[Month])
  {
    Month++;
    if (Month > 12)
    {
      Month = 1;
    }
    Days = 0;
    Day = MonthDays[Month];
  }
  else
  {
    Days = MonthDays[Month] - Day;
  }
  Jul += Days + Day;

  JulToOracle(Jul, NewDate);
  return NewDate;
}

void TimetToOracle(time_t UnixTime, pchar aOracleDate)
{
  tm * UnixTimeStruct;
  UnixTimeStruct = localtime(& UnixTime);
  sprintf(aOracleDate, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", (UnixTimeStruct->tm_year + 1900), (UnixTimeStruct->tm_mon + 1),
     UnixTimeStruct->tm_mday, UnixTimeStruct->tm_hour, UnixTimeStruct->tm_min, UnixTimeStruct->tm_sec);

}

time_t OracleToTimet(pchar aOracleDate)
{
  tm UnixTimeStruct;

  char YYYY[5];
  char MM[3];
  char DD[3];
  char HH[3];
  char MI[3];
  char SS[3];

  YYYY[sizeof(YYYY) - 1] = 0;
  MM[sizeof(MM) - 1] = 0;
  DD[sizeof(DD) - 1] = 0;
  HH[sizeof(HH) - 1] = 0;
  MI[sizeof(MI) - 1] = 0;
  SS[sizeof(SS) - 1] = 0;

  strncpy(YYYY, aOracleDate, 4);
  strncpy(MM, aOracleDate + 4, 2);
  strncpy(DD, aOracleDate + 6, 2);
  strncpy(HH, aOracleDate + 8, 2);
  strncpy(MI, aOracleDate + 10, 2);
  strncpy(SS, aOracleDate + 12, 2);

  UnixTimeStruct.tm_year = (atoi(YYYY) - 1900);
  UnixTimeStruct.tm_mon = (atoi(MM) - 1);
  UnixTimeStruct.tm_mday = atoi(DD);
  UnixTimeStruct.tm_hour = atoi(HH);
  UnixTimeStruct.tm_min = atoi(MI);
  UnixTimeStruct.tm_sec = atoi(SS);

  return mktime(& UnixTimeStruct);

}


