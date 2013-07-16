/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "Rgbe.h"




/* constants ---------------------------------------------------------------- */

static const char MINILIGHT_URI[] = "http://www.hxa.name/minilight";

static const char RADIANCE_ID[] = "#?RADIANCE";
static const char RGBE_ID[]     = "#?RGBE";

static const char ERROR_RGBE_INVALID[] = "RGBE image invalid";
static const char ERROR_RGBE_SIZE[]    = "RGBE image size invalid";




/* implementation ----------------------------------------------------------- */

static Exception nextLine
(
   FILE*      pFileIn,
   const char cp
)
{
   int c;

   if( EOF == ungetc( cp, pFileIn ) ) return ERROR_FILE;

   /* skip until next line */
   while( c = fgetc( pFileIn ), !(('\n' == c) || (EOF == c)) ) {}
   if( EOF == c ) return ERROR_FILE;

   return 0;
}


static bool checkDimensions
(
   const int32u width,
   const int32u height
)
{
   return (width > 0) && (height > 0) &&
      (height <= (INT32U_MAX / 3)) && (width <= (INT32U_MAX / (height * 3)));
}


static Vector3f fromRgbe
(
   const byteu rgbe[4]
)
{
   Vector3f rgb;

   if( 10 < rgbe[3] )
   {
      const real64 a = ldexp( 1.0, (int32)rgbe[3] - (128 + 8) );

      int i;
      for( i = 3;  i-- > 0;  rgb.xyz[i] = ((real64)rgbe[i] + 0.5) * a ) {}
   }
   else
   {
      rgb.xyz[0] = rgb.xyz[1] = rgb.xyz[2] = 0.0;
   }

   return rgb;
}


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




/* read --------------------------------------------------------------------- */

Exception RgbeIsRecognised
(
   FILE* pFileIn,
   bool* pB_o
)
{
   char s[ sizeof(RADIANCE_ID) ];

   rewind( pFileIn );
   if( !fgets( s, sizeof(s), pFileIn ) ) return ERROR_FILE;

   *pB_o = !strncmp( s, RADIANCE_ID, sizeof(RADIANCE_ID) -1 ) ||
      !strncmp( s, RGBE_ID, sizeof(RGBE_ID) -1 );

   return nextLine( pFileIn, s[strlen( s ) - 1] );
}


Exception RgbeReadHeader
(
   FILE*   pFileIn,
   int32u* pWidth_o,
   int32u* pHeight_o,
   int32u* pIterations_o
)
{
   bool b;
   Exception e = RgbeIsRecognised( pFileIn, &b );
   if( e || !b ) return e ? e : ERROR_RGBE_INVALID;

   {
      /* read iterations */
      char s[ 11 ];
      do
      {
         /* find key */
         if( !fgets( s, sizeof(s), pFileIn ) ) return ERROR_FILE;

         if( !strcmp( s, "ITERATION=" ) )
         {
            /* read value */
            if( EOF == fscanf( pFileIn, "%u", pIterations_o ) )
               return ERROR_FILE;
         }

         e = nextLine( pFileIn, s[strlen( s ) - 1] );
         if( e ) return e;

      /* blank line is end of header */
      } while( s[0] != '\n' );

      /* read dimensions */
      if( EOF == fscanf( pFileIn, "-Y %u +X %u\n", pHeight_o, pWidth_o ) )
         return ERROR_FILE;

      /* conditioning */
      if( !checkDimensions( *pWidth_o, *pHeight_o ) ) return ERROR_RGBE_SIZE;
   }

   return 0;
}


Exception RgbeReadPixels
(
   FILE*      pFileIn,
   int32u     width,
   int32u     height,
   Vector3f** paPixels_o
)
{
   int32u i, j;

   /* allocate storage */
   *paPixels_o = (Vector3f*)calloc( width * height, sizeof(Vector3f) );
   if( !*paPixels_o ) return ERROR_ALLOC;

   /* (top left first) */
   for( i = 0;  i < (width * height);  ++i )
   {
      /* read rgbe bytes */
      byteu rgbe[4];
      for( j = 0;  j < 4;  ++j )
      {
         const int b = fgetc( pFileIn );
         if( EOF == b )
         {
            free( *paPixels_o );
            *paPixels_o = 0;
            return ERROR_RGBE_INVALID;
         }

         rgbe[j] = (byteu)b;
      }

      (*paPixels_o)[i] = fromRgbe( rgbe );
   }

   return 0;
}


Exception RgbeRead
(
   FILE*      pFileIn,
   int32u*    pWidth_o,
   int32u*    pHeight_o,
   int32u*    pIterations_o,
   Vector3f** paPixels_o
)
{
   Exception e = RgbeReadHeader( pFileIn, pWidth_o, pHeight_o, pIterations_o );
   if( e ) return e;

   if( paPixels_o )
   {
      e = RgbeReadPixels( pFileIn, *pWidth_o, *pHeight_o, paPixels_o );
      if( e ) return e;
   }

   return 0;
}




/* write -------------------------------------------------------------------- */

Exception RgbeWrite
(
   int32u          width,
   int32u          height,
   int32u          iterations,
   const Vector3f* aPixels,
   FILE*           pFileOut_o
)
{
   /* write header */
   {
      /* precondition */
      if( !checkDimensions( width, height ) ) return ERROR_RGBE_SIZE;

      /* ID */
      if( 0 > fprintf( pFileOut_o, "#?RADIANCE\n" ) ) return ERROR_WRITE_IO;

      /* other header things */
      if( 0 > fprintf( pFileOut_o, "FORMAT=32-bit_rgbe\n" ) )
         return ERROR_WRITE_IO;
      if( 0 > fprintf( pFileOut_o, "SOFTWARE=%s\n", MINILIGHT_URI ) )
         return ERROR_WRITE_IO;
      if( 0 > fprintf( pFileOut_o, "ITERATION=%i\n\n", iterations ) )
         return ERROR_WRITE_IO;

      /* width, height */
      if( 0 > fprintf( pFileOut_o, "-Y %i +X %i\n", height, width ) )
         return ERROR_WRITE_IO;
   }

   /* write pixels */
   {
      int32u i, j;
      /* (top left first) */
      for( i = 0;  i < (width * height);  ++i )
      {
         const int32u rgbe = toRgbe( &aPixels[(int)i] );

         /* write rgbe bytes */
         for( j = 4;  j-- > 0; )
         {
            if( 0 > fprintf( pFileOut_o, "%c",
               (byteu)((rgbe >> (j * 8)) & 0xFFu) ) )
               return ERROR_WRITE_IO;
         }
      }
   }

   return 0;
}
