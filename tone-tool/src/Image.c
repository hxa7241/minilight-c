/*------------------------------------------------------------------------------

   MiniLightTone C : Tone-mapper tool for MiniLight RGBE renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Image.h"

#include "Rgbe.h"
#include "Ppm.h"




/* constants ---------------------------------------------------------------- */

/* ITU-R BT.709 standard RGB luminance weighting */
static const Vector3f RGB_LUMINANCE = {{ 0.2126, 0.7152, 0.0722 }};

/* guess of average screen maximum brightness */
static const real64 DISPLAY_LUMINANCE_MAX = 200.0;




/* implementation ----------------------------------------------------------- */

static Exception ImageRead
(
   const char* sFilePathName,
   Image*      pImage_o
)
{
   Exception e;
   int32u    iterations;

   FILE* pFileIn = fopen( sFilePathName, "r" );
   if( !pFileIn ) return ERROR_FILE;

   e = RgbeRead( pFileIn, &pImage_o->width, &pImage_o->height, &iterations,
      &pImage_o->aPixels );
   if( e ) return fclose( pFileIn ), e;

   if( EOF == fclose( pFileIn ) ) return ERROR_FILE;
   
   return 0;
}


static real64 ImageCalculateToneMapping
(
   const Image* pI,
   const int32u displayLumMax
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
         const real64 Y = Vector3fDot( &pI->aPixels[i], &RGB_LUMINANCE );
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
      const real64 displayLuminanceMax = displayLumMax ?
         (real64)displayLumMax : DISPLAY_LUMINANCE_MAX;

      const real64 a = 1.219 + pow( displayLuminanceMax * 0.25, 0.4 );
      const real64 b = 1.219 + pow( adaptLuminance, 0.4 );

      return pow( a / b, 2.5 ) / DISPLAY_LUMINANCE_MAX;
   }
}


static void ImageMultiplyByScalar
(
   Image*       pI_io,
   const real64 m
)
{
   int32u i;
   for( i = pI_io->width * pI_io->height;  i-- > 0; )
   {
      pI_io->aPixels[ i ] = Vector3fMulF( pI_io->aPixels + i, m );
   }
}




/* initialisation ----------------------------------------------------------- */

Exception ImageConstruct
(
   const char* sFilePathName,
   Image**     ppImage_o
)
{
   Exception e;

   *ppImage_o = (Image*)calloc( 1, sizeof(Image) );
   if( !*ppImage_o ) return ERROR_ALLOC;

   e = ImageRead( sFilePathName, *ppImage_o );
   if( e ) return free( *ppImage_o ), e;

   return 0;
}


void ImageDestruct
(
   Image* pI
)
{
   free( pI->aPixels );
   free( pI );
}




/* queries ------------------------------------------------------------------ */

Exception ImageWrite
(
   const Image* pI,
   const char*  sFilePathName
)
{
   Exception e;

   FILE* pFileOut = fopen( sFilePathName, "wb" );
   if( !pFileOut ) return ERROR_FILE;

   e = PpmWrite( pI->width, pI->height, pI->aPixels, pFileOut );
   if( e ) return fclose( pFileOut ), e;

   if( EOF == fclose( pFileOut ) ) return ERROR_FILE;

   return 0;
}




/* commands ----------------------------------------------------------------- */

void ImageTonemap
(
   Image* pI_io,
   int32u displayLuminanceMax
)
{
   const real64 tonemapScaling = ImageCalculateToneMapping( pI_io,
      displayLuminanceMax );

   ImageMultiplyByScalar( pI_io, tonemapScaling );
}
