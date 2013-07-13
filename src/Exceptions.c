/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include "Exceptions.h"




/* functions ---------------------------------------------------------------- */

void throwReadExceptions
(
   FILE*   pIn,
   jmp_buf jmpBuf,
   int     argCount,
   int     readCount
)
{
   const int code = ferror( pIn ) ? ERROR_READ_IO : ( feof( pIn ) ?
      ERROR_READ_TRUNC : ((readCount < argCount) ? ERROR_READ_INVAL : 0) );

   clearerr( pIn );

   throwExceptions( jmpBuf, (bool)code, code );
}


void throwWriteExceptions
(
   FILE*   pOut,
   jmp_buf jmpBuf,
   int     c
)
{
   clearerr( pOut );

   throwExceptions( jmpBuf, (c < 0), ERROR_WRITE_IO );
}


void* throwAllocExceptions
(
   jmp_buf jmpBuf,
   void*   p
)
{
   throwExceptions( jmpBuf, !p, ERROR_ALLOC );

   return p;
}


void throwExceptions
(
   jmp_buf jmpBuf,
   bool    trueToThrow,
   int     code
)
{
   if( trueToThrow )
   {
      longjmp( jmpBuf, code );
   }
}
