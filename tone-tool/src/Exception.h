/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Exception_h
#define Exception_h


#include <stdio.h>

#include "Primitives.h"




/**
 * Functions that decide, from their arguments, whether to warn or exit.
 */


/* types -------------------------------------------------------------------- */

typedef const char* Exception;




/* functions ---------------------------------------------------------------- */

void warning
(
   const Exception,
   const char* sFilePathName
);

void error
(
   const Exception,
   const char* sFilePathName
);




/* constants ---------------------------------------------------------------- */

extern const char* NO_ERROR;

extern const char ERROR_UNSPECIFIED[];

extern const char ERROR_FILE[];

extern const char ERROR_READ_IO[];
extern const char ERROR_READ_TRUNC[];
extern const char ERROR_READ_INVAL[];

extern const char ERROR_WRITE_IO[];

extern const char ERROR_ALLOC[];




#endif
