/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Ppm_h
#define Ppm_h


#include <stdio.h>

#include "Primitives.h"
#include "Exception.h"
#include "Vector3f.h"




/**
 * IO for a subset of the nice-and-simple PPM P6 format.
 *
 * http://netpbm.sourceforge.net/doc/ppm.html
 */




/* read --------------------------------------------------------------------- */

Exception PpmIsRecognised
(
   FILE* pFileIn,
   bool* pB_o
);


Exception PpmReadHeader
(
   FILE*   pFileIn,
   int32u* pWidth_o,
   int32u* pHeight_o
);


Exception PpmReadPixels
(
   FILE*      pFileIn,
   int32u     width,
   int32u     height,
   Vector3f** paPixels_o
);


Exception PpmRead
(
   FILE*      pFileIn,
   int32u*    pWidth_o,
   int32u*    pHeight_o,
   Vector3f** paPixels_o
);




/* write -------------------------------------------------------------------- */

Exception PpmWrite
(
   int32u          width,
   int32u          height,
   const Vector3f* aPixels,
   FILE*           pFileOut_o
);




#endif
