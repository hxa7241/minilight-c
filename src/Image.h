/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Image_h
#define Image_h


#include <stdio.h>
#include <setjmp.h>

#include "Primitives.h"
#include "Vector3f.h"




/**
 * Pixel sheet with simple tone-mapping and file formatting.<br/><br/>
 *
 * Uses Ward simple tonemapper:
 * <cite>'A Contrast Based Scalefactor For Luminance Display';
 * Ward;
 * Graphics Gems 4, AP; 1994.</cite><br/><br/>
 *
 * Uses PPM image format:
 * <cite>http://netpbm.sourceforge.net/doc/ppm.html</cite><br/><br/>
 *
 * Uses RGBE image format:
 * <cite>http://radsite.lbl.gov/radiance/refer/filefmts.pdf</cite>
 * <cite>'Real Pixels'; Ward; Graphics Gems 2, AP; 1991.</cite><br/><br/>
 *
 * Mutable.
 *
 * @invariants
 * * width  >= 1 and <= IMAGE_DIM_MAX
 * * height >= 1 and <= IMAGE_DIM_MAX
 * * aPixels length == (width * height)
 */

struct Image
{
   int32     width;
   int32     height;
   Vector3f* aPixels;

   bool      isHdri;
};

typedef struct Image Image;




/* initialisation ----------------------------------------------------------- */

Image* ImageConstruct
(
   FILE*   pIn,
   jmp_buf jmpBuf,
   bool    isHdri
);

void ImageDestruct
(
   Image* pI
);




/* commands ----------------------------------------------------------------- */

/**
 * Accumulate (add, not just assign) a value to the image.
 */
void ImageAddToPixel
(
   Image*          pI,
   int32           x,
   int32           y,
   const Vector3f* pRadiance
);




/* queries ------------------------------------------------------------------ */

/**
 * Write the image to a serialised format.
 */
void ImageFormatted
(
   const Image* pI,
   int32        iteration,
   jmp_buf      jmpBuf,
   FILE*        pOut_o
);




/* constants ---------------------------------------------------------------- */

/**
 * Image dimension max.
 */
#define IMAGE_DIM_MAX ((int32)4000)




#endif
