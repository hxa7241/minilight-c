/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>
#include <stdlib.h>

#include "Exceptions.h"

#include "Image.h"




/* constants ---------------------------------------------------------------- */

/* image file comment */
static const char MINILIGHT_URI[] = "http://www.hxa.name/minilight";

/* guess of average screen maximum brightness */
static const real64 DISPLAY_LUMINANCE_MAX = 200.0;

/* ITU-R BT.709 standard RGB luminance weighting */
static const Vector3f RGB_LUMINANCE = {{ 0.2126, 0.7152, 0.0722 }};

/* ITU-R BT.709 standard gamma */
static const real64 GAMMA_ENCODE = 0.45;




/* implementation ----------------------------------------------------------- */

/**
 * Convert FP RGB into 32bit RGBE format.
 */
static int32u toRgbe
(
   const Vector3f* pRgbIn
)
{
   int32u rgbe = 0;

   const Vector3f rgb  = Vector3fClamped( pRgbIn, &Vector3fZERO, pRgbIn );
   const real64*  aRgb = rgb.xyz;
   const real64   rgbLargest = (aRgb[0] >= aRgb[1]) ? (aRgb[0] >= aRgb[2] ?
      aRgb[0] : aRgb[2]) : (aRgb[1] >= aRgb[2] ? aRgb[1] : aRgb[2]);

   if( rgbLargest >= 1e-9 )
   {
      int    exponentLargest = 0;
      real64 mantissaLargest = frexp( rgbLargest, &exponentLargest );

      /* has been needed in the past, sadly...
      if( 1.0 == mantissaLargest  )
      {
         mantissaLargest = 0.5;
         exponentLargest++;
      }*/

      const real64 amount = mantissaLargest * 256.0 / rgbLargest;

      int i;
      for( i = 3;  i-- > 0; )
      {
         rgbe |= (int32u)floor( aRgb[i] * amount ) << ((3 - i) * 8);
      }
      rgbe |= (int32u)(exponentLargest + 128);
   }

   return rgbe;
}


/**
 * Calculate tone-mapping scaling factor.
 */
static real64 calculateToneMapping
(
   const Image* pI,
   const real64 divider
)
{
   /* calculate estimate of world-adaptation luminance
      as log mean luminance of scene */
   real64 adaptLuminance = 1e-4;
   {
      real64 sumOfLogs = 0.0;
      int32 i;
      for( i = (pI->width * pI->height);  i-- > 0; )
      {
         const real64 Y = Vector3fDot( &pI->aPixels[i], &RGB_LUMINANCE ) *
            divider;
         /* clamp luminance to a perceptual minimum */
         sumOfLogs += log10( Y > 1e-4 ? Y : 1e-4 );
      }

      adaptLuminance = pow( 10.0, sumOfLogs /
         (real64)(pI->width * pI->height) );
   }

   /* make scale-factor from:
      ratio of minimum visible differences in luminance, in display-adapted
      and world-adapted perception (discluding the constant that cancelled),
      divided by display max to yield a [0,1] range */
   {
      const real64 a = 1.219 + pow( DISPLAY_LUMINANCE_MAX * 0.25, 0.4 );
      const real64 b = 1.219 + pow( adaptLuminance, 0.4 );

      return pow( a / b, 2.5 ) / DISPLAY_LUMINANCE_MAX;
   }
}


static void ImageFormattedRgbe
(
   const Image* pI,
   const real64 divider,
   jmp_buf      jmpBuf,
   FILE*        pOut_o
)
{
   /* write Radiance RGBE format */

   /* write header */
   {
      /* write ID */
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "#?RADIANCE\n" ) );

      /* write other header things */
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "FORMAT=32-bit_rgbe\n" ) );
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "SOFTWARE=%s\n\n", MINILIGHT_URI ) );

      /* write width, height */
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "-Y %i +X %i\n", pI->height, pI->width ) );
   }

   /* write pixels */
   {
      int32 i, b;
      for( i = 0;  i < (pI->width * pI->height);  ++i )
      {
         const Vector3f pd   = Vector3fMulF( &pI->aPixels[i], divider );
         const int32u   rgbe = toRgbe( &pd );

         /* write rgbe bytes */
         for( b = 4;  b-- > 0; )
         {
            throwWriteExceptions( pOut_o, jmpBuf,
               fprintf( pOut_o, "%c", (byteu)((rgbe >> (b * 8)) & 0xFFu) ) );
         }
      }
   }
}


static void ImageFormattedPpm
(
   const Image* pI,
   const real64 divider,
   jmp_buf      jmpBuf,
   FILE*        pOut_o
)
{
   const real64 tonemapScaling = calculateToneMapping( pI, divider );

   /* write PPM P6 format */

   /* write header */
   {
      /* write ID and comment */
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "P6\n# %s\n\n", MINILIGHT_URI ) );

      /* write width, height, maxval */
      throwWriteExceptions( pOut_o, jmpBuf,
         fprintf( pOut_o, "%i %i\n%i\n", pI->width, pI->height, 255 ) );
   }

   /* write pixels */
   {
      int32 i, c;
      for( i = 0;  i < (pI->width * pI->height);  ++i )
      {
         for( c = 0;  c < 3;  ++c )
         {
            /* tonemap */
            const real64 mapped = pI->aPixels[i].xyz[c] * divider *
               tonemapScaling;

            /* gamma encode */
            const real64 gammaed = pow( (mapped > 0.0 ? mapped : 0.0),
               GAMMA_ENCODE );

            /* quantize */
            const real64 quantized = floor( (gammaed * 255.0) + 0.5 );

            /* output as byte */
            throwWriteExceptions( pOut_o, jmpBuf, fprintf( pOut_o, "%c",
               (byteu)(quantized <= 255.0 ? quantized : 255.0) ) );
         }
      }
   }
}




/* initialisation ----------------------------------------------------------- */

Image* ImageConstruct
(
   FILE*   pIn,
   jmp_buf jmpBuf,
   bool    isHdri
)
{
   Image* pI = (Image*)throwAllocExceptions( jmpBuf,
      calloc( 1, sizeof(Image) ) );

   /* read width and height */
   throwReadExceptions( pIn, jmpBuf, 2,
      fscanf( pIn, "%i %i", &pI->width, &pI->height ) );

   /* condition width and height */
   pI->width  = pI->width  < 1 ? 1 :
      (pI->width  > IMAGE_DIM_MAX ? IMAGE_DIM_MAX : pI->width );
   pI->height = pI->height < 1 ? 1 :
      (pI->height > IMAGE_DIM_MAX ? IMAGE_DIM_MAX : pI->height);

   /* allocate pixels */
   pI->aPixels = (Vector3f*)throwAllocExceptions( jmpBuf,
      calloc( pI->width * pI->height, sizeof(Vector3f) ) );

   pI->isHdri = isHdri;

   return pI;
}


void ImageDestruct
(
   Image* pI
)
{
   /* free pixels */
   free( pI->aPixels );

   free( pI );
}




/* commands ----------------------------------------------------------------- */

void ImageAddToPixel
(
   Image*          pI,
   int32           x,
   int32           y,
   const Vector3f* pRadiance
)
{
   /* only inside image bounds */
   if( (x >= 0) & (x < pI->width) & (y >= 0) & (y < pI->height) )
   {
      const int32 index = x + ((pI->height - 1 - y) * pI->width);
      pI->aPixels[index] = Vector3fAdd( &pI->aPixels[index], pRadiance );
   }
}




/* queries ------------------------------------------------------------------ */

void ImageFormatted
(
   const Image* pI,
   int32        iteration,
   jmp_buf      jmpBuf,
   FILE*        pOut_o
)
{
   const real64 divider = 1.0 / (real64)(iteration >= 1 ? iteration : 1);

   if( pI->isHdri )
   {
      ImageFormattedRgbe( pI, divider, jmpBuf, pOut_o );
   }
   else
   {
      ImageFormattedPpm( pI, divider, jmpBuf, pOut_o );
   }
}
