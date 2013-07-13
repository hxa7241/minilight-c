/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Exceptions_h
#define Exceptions_h


#include <stdio.h>
#include <setjmp.h>

#include "Primitives.h"




/**
 * Functions that decide, from their arguments, whether to jongjmp.
 */


/* functions ---------------------------------------------------------------- */

void throwReadExceptions
(
   FILE*   pIn,
   jmp_buf jmpBuf,
   int     argCount,
   int     readCount
);

void throwWriteExceptions
(
   FILE*   pOut,
   jmp_buf jmpBuf,
   int     c
);

void* throwAllocExceptions
(
   jmp_buf jmpBuf,
   void*   p
);

void throwExceptions
(
   jmp_buf jmpBuf,
   bool    trueToThrow,
   int     code
);




/* constants ---------------------------------------------------------------- */

#define ERROR_UNSPECIFIED  -1

#define ERROR_READ_IO     256
#define ERROR_READ_TRUNC  257
#define ERROR_READ_INVAL  258

#define ERROR_WRITE_IO    384

#define ERROR_ALLOC       512




#endif
