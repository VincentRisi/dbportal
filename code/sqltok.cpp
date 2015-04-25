
#include "sqltok.h"

#define SQLTOK_ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SQLTOK_DIGIT "0123456789"
#define SQLTOK_ALPHA_DIGIT SQLTOK_ALPHA SQLTOK_DIGIT
#define SQLTOK_ALPHA_DIGIT_PLUS SQLTOK_ALPHA_DIGIT "_$#"
#define SQLTOK_DIGIT_PLUS SQLTOK_DIGIT ".+-eE"

inline int alpha(char ch)
{
  return (ch && strchr(SQLTOK_ALPHA, ch) != 0) ? 1 : 0;
}

inline int alphaNum(char ch)
{
  return (ch && strchr(SQLTOK_ALPHA_DIGIT, ch) != 0) ? 1 : 0;
}

inline int alphaNumPlus(char ch)
{
  return (ch && strchr(SQLTOK_ALPHA_DIGIT_PLUS, ch) != 0) ? 1 : 0;
}

inline int digit(char ch)
{
  return (ch && strchr(SQLTOK_DIGIT, ch) != 0) ? 1 : 0;
}

inline int digitPlus(char ch)
{
  return (ch && strchr(SQLTOK_DIGIT_PLUS, ch) != 0) ? 1 : 0;
}

pchar SqlToken(pchar source, pchar token, int token_length)
{
  int n;
  pchar result = source+1;
  switch(*source)
  {
  case ' ':
  case '\t':
    while (*result == ' ' || *result == '\t')
      result++;
    break;
  case ':':
  case '$':
    if (*result == *source)
      break;
    if (alpha(*result))
    {
      result++;
      while (alphaNumPlus(*result))
        result++;
    }
    break;
  case '\'':
  case '\"':
    while (*result && *result != *source)
      result++;
    if (*result == *source)
      result++;
    break;
  case '-':
    if (*result == '-')
    {
      result++;
      while (*result && *result != '\n')
        result++;
      if (*result)
        result++;
      source = result;
      break;
    }
    break;  
  case '/':
    if (*result == '*')
    {
      result++;
      while (result[0] && result[0] != '*' && result[1] && result[1] != '/')
        result++;
      if (result[0] && result[1])
        result += 2;
      source = result;
      break;
    }
    break;  
  default:
    if (alpha(*source))
    {
      while (alphaNumPlus(*result))
        result++;
      break;
    }
    if (digit(*source))
    {
      while (digitPlus(*result))
        result++;
      break;
    }
    break;
  }
  n = result - source;
  if (n > token_length-1)
    n = token_length-1;
  strncpy(token, source, n);
  token[n] = 0;
  return result;
}
