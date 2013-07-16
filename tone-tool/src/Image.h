/*------------------------------------------------------------------------------

   MiniLightTone C : Tone-mapper tool for MiniLight RGBE renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include "Primitives.h"
#include "Exception.h"
#include "Vector3f.h"




/* types -------------------------------------------------------------------- */

struct Image
{
   int32u width;
   int32u height;

   Vector3f* aPixels;
};

typedef struct Image Image;




/* initialisation ----------------------------------------------------------- */

Exception ImageConstruct
(
   const char* sFilePathName,
   Image**     ppImage_o
);


void ImageDestruct
(
   Image* pI
);




/* queries ------------------------------------------------------------------ */

Exception ImageWrite
(
   const Image* pI,
   const char*  sFilePathName
);




/* commands ----------------------------------------------------------------- */

void ImageTonemap
(
   Image* pI_io,
   int32u displayLuminanceMax
);
