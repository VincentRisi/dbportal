#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *strltrim(char *s)
{
   char *t = s;

   while (*t == ' ' || *t == '\t')
      t++;

   if (t != s)
      strcpy(s, t);

   return s;
}


char *strcpad(char *s, const int fldlen, const char padchar)
{
   int len = strlen(s), len1, len2;

   if (len < fldlen)
   {
      len1 = (fldlen - len) / 2;
      len2 = fldlen - len - len1;
      memmove(s + len1, s, len);
      memset(s, padchar, len1);
      memset(s + fldlen - len2, padchar, len2);
   }

   return s;
}


int tolower_stricmp(const char *a, const char *b)
{
   int n;

   for (; ;a++, b++)
   {
      n = tolower((unsigned char)*a) - tolower((unsigned char)*b);

      if (n || !*a)
         break;
   }

   return n;
}


char *strlpad(char *s, const int fldlen, const char padchar)
{
   int len = strlen(s);

   if (len < fldlen)
   {
      memmove(s + fldlen - len, s, len);
      memset(s, padchar, fldlen - len);
   }
   return s;
}


char *strncpyz(char *dest, const char *src, const size_t n)
{
   strncpy(dest, src, n);

   dest[n] = 0;

   return dest;
}


int tolower_strnicmp(const char *a, const char *b, const size_t n)
{
   int i, ret;

   for (i = 0; i < (int)n; i++)
   {
      ret = tolower((unsigned char)*a) - tolower((unsigned char)*b);
      if (ret || !*a)
         break;
      a++;
      b++;
   }

   return ret;
}


char *strrpad(char *s, const int fldlen, const char padchar)
{
   int len = strlen(s);

   while (len < fldlen)
      s[len++] = padchar;

   s[fldlen] = 0;

   return s;
}


char *strrtrim(char *s)
{
   int len = strlen(s);

   while (len && (s[len - 1] == ' ' || s[len - 1] == '\t'))
      s[--len] = 0;
   return s;
}


char *strupr(char *s)
{
   char *t = s;

   while (*t)
   {
      *t = toupper(*t);
      t++;
   }

   return s;
}


char *strlwr(char *s)
{
   char *t = s;

   while (*t)
   {
      *t = tolower(*t);
      t++;
   }

   return s;
}

