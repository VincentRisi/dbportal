#ifndef _SQLCONS_H_
#define _SQLCONS_H_ 

#define SQL_CHAR            1
#define SQL_NUMERIC         2
#define SQL_DECIMAL         3
#define SQL_INTEGER         4
#define SQL_SMALLINT        5
#define SQL_FLOAT           6
#define SQL_REAL            7
#define SQL_DOUBLE          8
#define SQL_DATE            9
#define SQL_TIME           10
#define SQL_TIMESTAMP      11
#define SQL_VARCHAR        12
#define SQL_LONG           13
#ifndef SQL_LONGVARCHAR
#define SQL_LONGVARCHAR    -1
#define SQL_BINARY         -2
#define SQL_VARBINARY      -3
#define SQL_LONGVARBINARY  -4
#define SQL_BIGINT         -5
#define SQL_TINYINT        -6
#define SQL_BIT            -7
#endif

#define SQL_C_BINARY       SQL_BINARY
#define SQL_C_BIT          SQL_BIT
#define SQL_C_CHAR         SQL_CHAR
#define SQL_C_DATE         SQL_DATE
#define SQL_C_DOUBLE       SQL_DOUBLE
#define SQL_C_FLOAT        SQL_REAL
#define SQL_C_LONG         SQL_INTEGER
#define SQL_C_LONG64       SQL_LONG
#define SQL_C_SHORT        SQL_SMALLINT
#define SQL_C_TIME         SQL_TIME
#define SQL_C_TIMESTAMP    SQL_TIMESTAMP
#define SQL_C_TINYINT      SQL_TINYINT
#define SQL_C_CLIMAGE      94
#define SQL_C_BLIMAGE      95
#define SQL_C_ZLIMAGE     195
#define SQL_C_HUGECHAR    194 
#define SQL_C_XMLTYPE     208 // use this as a 108 SQLT_NTY (an object type)

#define SQL_OCI_NUMBER      3 // This is actually INT NUMBER is really 2
#define SQL_OCI_FLOAT       4
#define SQL_OCI_VARCHAR2    5 // This is actually null terminated string VARCHAR2 1
#define SQL_OCI_VARNUM      6
#define SQL_OCI_SQLT_LONG   8
#define SQL_OCI_SQLT_VCS    9
#define SQL_OCI_IMAGE      15
#define SQL_OCI_RAW        23
#define SQL_OCI_LONGRAW    24
#define SQL_OCI_CLIMAGE    94
#define SQL_OCI_BLIMAGE    95
#define SQL_OCI_XMLTYPE   208 // use this as a 108 SQLT_NTY (an object type)
#define SQL_OCI_CHARx      96
#define SQL_OCI_CHARz      97
#define SQL_OCI_HUGECHAR  194

#endif
