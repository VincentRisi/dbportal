#ifndef ocinumberH
#define ocinumberH

#include <iostream>
#include "ocimachine.h"

class ODecimal;
class OFormat
{
public:
  OFormat(int digit, int decimal);
  OFormat(const char* format); 
  char * operator() (ODecimal &); 


  char* setFormat(int digit, int decimal);
  char* setFormat(const char* format); 

  char *numberFormat;
  char *Textformat;
};

class ODecimal
{
  static OCIEnv* ociEnv;
  static OCIError* ociError;
  void Setup();
  int result;
  void setResult(int value, char* file, int line);
  void setResult(int &result, int value, char* file, int line);
  OCINumber number;
  char *numberFormat;
  char *text;
public:
  char *Textformat;
  // sizeof copy should be one less with a set of 0 for null terminator
  char* setFormat(const char* value);
  char* setFormat(int digit, int decimal);
  ODecimal(const char *str);
  ODecimal();
  ODecimal(int input);
  ODecimal(unsigned int input);
  ODecimal(long input);
  ODecimal(unsigned long input);
  ODecimal(short input);
  ODecimal(unsigned short input);
  ODecimal(char input);
  ODecimal(unsigned char input);
  ODecimal(float input);
  ODecimal(double input);
  ODecimal(long double input);
  ODecimal(const ODecimal &input);
  virtual ~ODecimal();
  ODecimal abs();
  ODecimal arcCos();
  ODecimal arcSin();
  ODecimal arcTan();
  ODecimal arcTan2(ODecimal& rhs);
  ODecimal cos();
  ODecimal sin();
  ODecimal tan();
  ODecimal hypCos();
  ODecimal hypSin();
  ODecimal hypTan();
  ODecimal ceil();
  ODecimal floor();
  ODecimal exp();
  ODecimal ln();
  ODecimal neg();
  ODecimal sqrt();
  ODecimal power(int exp);
  ODecimal power(const ODecimal& exp);
  ODecimal log(const ODecimal& arb);
  //ODecimal prec(int exp);
  ODecimal round(int places);
  ODecimal trunc(int places);
  ODecimal shift(int places);
  int compare(const ODecimal& other);
  int sign();
  int isZero();
  void setZero();
  char getChar();
  short getShort();
  int getInt();
  long getLong();
  unsigned char getUChar();
  unsigned int getUInt();
  unsigned long getULong();
  unsigned short getUShort();
  float getFloat();
  double getDouble();
  long double getLongDouble();
  char* getText(char *value, ub4 &length);
  char* getText(char *value, ub4 &length, const char *fmt);
  char* getText(const char *fmt = 0);
  char* getText(ODecimal & number);
  char* getText(OFormat & format);

  operator double () {
     return getDouble();
  }

//  operator const char *() const;           // conversion to const char * op.
//  operator const char *();                 // conversion to const char * op.
//  operator char *();                       // conversion to const char * op.

 // operator const char *() {return getText();};                       // conversion to const char * op.   

//  ODecimal operator =(const char *& rhs);
  ODecimal operator =(const ODecimal &rhs);
  ODecimal operator +=(const ODecimal &rhs);
  ODecimal operator +(const ODecimal &rhs);
  ODecimal operator +(const int &rhs);
  ODecimal operator +(const double &rhs);
  ODecimal operator -(const ODecimal &rhs);
  ODecimal operator -=(const ODecimal &rhs);
  ODecimal operator *(const ODecimal &rhs);
  ODecimal operator *(const int &rhs);
  ODecimal operator *(const double &rhs);
  ODecimal operator /(const ODecimal &rhs);
  ODecimal operator /(const int &rhs);
  ODecimal operator /(const double &rhs);
  ODecimal operator %(const ODecimal &rhs);

  bool operator ==(const ODecimal &rhs);
  bool operator !=(const ODecimal &rhs);
  bool operator >(const ODecimal &rhs);
  bool operator <(const ODecimal &rhs);
  bool operator >=(const ODecimal &rhs);
  int getResult() { return result; }
  
  friend std::ostream& operator<<(std::ostream& strm, ODecimal& str);
  bool testMe(void);
};

inline std::ostream& operator<<(std::ostream& os, ODecimal& str)
{
   char jotter[1024], *p= jotter;
   ub4 len= sizeof(jotter);
   char * tmp = str.getText(p, len);
   os << tmp;
  return os;
}

inline ODecimal& round(ODecimal& number, int size) {
   number.round(size);
   return number;
}

inline ODecimal& abs(ODecimal& number) {
   number.abs();
   return number;
}

inline ODecimal& trunc(ODecimal& number, int size) {
   number.trunc(size);
   return number;
}

inline ODecimal& shift(ODecimal& number, int size) {
   number.shift(size);
   return number;
}

inline ODecimal& SetFmt(int digit, int decimal) {
   static ODecimal number;
   number.setFormat(digit, decimal);
   return number;
}

#endif

