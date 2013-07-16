/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "Exception.h"




/* constants ---------------------------------------------------------------- */

const char* NO_ERROR           = 0;

const char ERROR_UNSPECIFIED[] = "unspecified error";

const char ERROR_FILE[]        = "file error";

const char ERROR_READ_IO[]     = "I/O read error";
const char ERROR_READ_TRUNC[]  = "file truncated";
const char ERROR_READ_INVAL[]  = "file invalid";

const char ERROR_WRITE_IO[]    = "I/O write error";

const char ERROR_ALLOC[]       = "allocation error";




/* implementation ----------------------------------------------------------- */

static void print
(
   const Exception e,
   const char*     sHead,
   const char*     sFilePathName
)
{
   fprintf( stderr, "\n%s\ncause: %s\n", sHead, e );
   if( errno ) fprintf( stderr, "error: %s\n", strerror( errno ) );
   if( sFilePathName ) fprintf( stderr, "file:  %s\n", sFilePathName );
}




/* functions ---------------------------------------------------------------- */

void warning
(
   const Exception e,
   const char*     sFilePathName
)
{
   if( e )
   {
      fprintf( stderr, "+++ warning: %s: %s\n", e, sFilePathName );
      /*print( e, "+++ warning", sFilePathName );*/
   }
}


void error
(
   const Exception e,
   const char*     sFilePathName
)
{
   if( e )
   {
      print( e, "*** execution failed", sFilePathName );

      exit( EXIT_FAILURE );
   }
}
