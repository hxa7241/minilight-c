/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Rgbe_h
#define Rgbe_h


#include <stdio.h>

#include "Primitives.h"
#include "Exception.h"
#include "Vector3f.h"




/**
 * IO for a subset of the HDR Radiance RGBE format.
 *
 * http://radsite.lbl.gov/radiance/refer/filefmts.pdf
 * 'Real Pixels'; Ward; 1991.
 */




/* read --------------------------------------------------------------------- */

Exception RgbeIsRecognised
(
   FILE* pFileIn,
   bool* pB_o
);


Exception RgbeReadHeader
(
   FILE*   pFileIn,
   int32u* pWidth_o,
   int32u* pHeight_o,
   int32u* pIterations_o
);


Exception RgbeReadPixels
(
   FILE*      pFileIn,
   int32u     width,
   int32u     height,
   Vector3f** paPixels_o
);


Exception RgbeRead
(
   FILE*      pFileIn,
   int32u*    pWidth_o,
   int32u*    pHeight_o,
   int32u*    pIterations_o,
   Vector3f** paPixels_o
);




/* write -------------------------------------------------------------------- */

Exception RgbeWrite
(
   int32u          width,
   int32u          height,
   int32u          iterations,
   const Vector3f* aPixels,
   FILE*           pFileOut_o
);




#endif
