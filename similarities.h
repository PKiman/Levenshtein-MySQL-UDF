//
// Created by Paul Kiman on 17.06.17.
//

//#ifndef MYSQL_LEVENSHTEIN_UDF_SIMILARITIES_H
//#define MYSQL_LEVENSHTEIN_UDF_SIMILARITIES_H
//
//#endif //MYSQL_LEVENSHTEIN_UDF_SIMILARITIES_H

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong; /* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else

#include <my_global.h>
#include <my_sys.h>

#if defined(MYSQL_SERVER)
#include <m_string.h>
#else
/* when compiled as standalone */
#include <string.h>

#endif
#endif

#include <mysql.h>
#include <ctype.h>

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
