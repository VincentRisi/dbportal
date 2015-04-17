#ifndef _SQLTOK_H_
#define _SQLTOK_H_ 
#include "machine.h"

#include <string.h>

pchar SqlToken(pchar Source, pchar Token, int TokenSize);
// Returns point in source after token. Null indicates completed
// Fills Token with the token conforming to SQL and additionally
// :vars (placeholders) and $vars (dynamic substitutions)

#endif
