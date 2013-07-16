/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "Ppm.h"




/* constants ---------------------------------------------------------------- */

static const char MINILIGHT_URI[] = "http://www.hxa.name/minilight";

static const char PPM_ID[] = "P6";

static const char ERROR_PPM_INVALID[] = "PPM image invalid";
static const char ERROR_PPM_SIZE[]    = "PPM image size invalid";

/* ITU-R BT.709 standard gamma */
static const real64 GAMMA_ENCODE = 0.45;





/* implementation ----------------------------------------------------------- */

static bool checkDimensions
(
   const int32u width,
   const int32u height
)
{
   return (width > 0) && (height > 0) &&
      (height <= (INT32U_MAX / 3)) && (width <= (INT32U_MAX / (height * 3)));
}




/* read --------------------------------------------------------------------- */

Exception PpmIsRecognised
(
   FILE* pFileIn,
   bool* pB_o
)
{
   char s[ sizeof(PPM_ID) ];

   rewind( pFileIn );
   if( !fgets( s, sizeof(s), pFileIn ) ) return ERROR_FILE;

   *pB_o = !strcmp( s, PPM_ID );

   return 0;
}


Exception PpmReadHeader
(
   FILE*   pFileIn,
   int32u* pWidth_o,
   int32u* pHeight_o
)
{
   bool b;
   Exception e = PpmIsRecognised( pFileIn, &b );
   if( e || !b ) return e ? e : ERROR_PPM_INVALID;

   {
      /* read four integers */
      int32u       maxval;
      unsigned int aIntu[ 4 ];
      int i;
      for( i = 0;  i < 3;  ++i )
      {
         /* skip blanks and comments */
         while( true )
         {
            int c = fgetc( pFileIn );

            if( EOF == c )
            {
               return feof(pFileIn) ? ERROR_PPM_INVALID : ERROR_FILE;
            }
            else if( '#' == c )
            {
               /* skip until end of line */
               while( c = fgetc( pFileIn ), ('\n' != c) ) {}
            }
            else if( !isspace(c) )
            {
               if( (EOF == ungetc( c, pFileIn )) ) return ERROR_FILE;
               break;
            }
         }

         /* read integer */
         if( EOF == fscanf( pFileIn, "%u", &aIntu[i] ) ) return ERROR_FILE;
      }
      /* skip final single blank */
      if( !isspace( fgetc( pFileIn ) ) ) return ERROR_PPM_INVALID;

      *pWidth_o  = (int32)aIntu[0];
      *pHeight_o = (int32)aIntu[1];
      maxval     = (int32)aIntu[2];

      /* conditioning */
      if( maxval != 255 ) return ERROR_PPM_INVALID;
      if( !checkDimensions( *pWidth_o, *pHeight_o ) ) return ERROR_PPM_SIZE;
   }

   return 0;
}


Exception PpmReadPixels
(
   FILE*      pFileIn,
   int32u     width,
   int32u     height,
   Vector3f** paPixels_o
)
{
   int32u i, c;

   /* allocate storage */
   *paPixels_o = (Vector3f*)calloc( width * height, sizeof(Vector3f) );
   if( !*paPixels_o ) return ERROR_ALLOC;

   /* (top left first) */
   for( i = 0;  i < (width * height);  ++i )
   {
      /* (R then G then B) */
      for( c = 0;  c < 3;  ++c )
      {
         /* read value */
         const int b = fgetc( pFileIn );
         if( EOF == b )
         {
            free( *paPixels_o );
            return ERROR_PPM_INVALID;
         }

         /* convert to real and gamma decode */
         (*paPixels_o)[i].xyz[c] = pow(
            ((real64)b / 255.0), (1.0f / GAMMA_ENCODE) );
      }
   }

   return 0;
}


Exception PpmRead
(
   FILE*      pFileIn,
   int32u*    pWidth_o,
   int32u*    pHeight_o,
   Vector3f** paPixels_o
)
{
   Exception e = PpmReadHeader( pFileIn, pWidth_o, pHeight_o );
   if( e ) return e;

   if( paPixels_o )
   {
      e = PpmReadPixels( pFileIn, *pWidth_o, *pHeight_o, paPixels_o );
      if( e ) return e;
   }

   return 0;
}




/* write -------------------------------------------------------------------- */

Exception PpmWrite
(
   int32u          width,
   int32u          height,
   const Vector3f* aPixels,
   FILE*           pFileOut_o
)
{
   /* write header */
   {
      /* precondition */
      if( !checkDimensions( width, height ) ) return ERROR_PPM_SIZE;

      /* ID and comment */
      if( 0 > fprintf( pFileOut_o, "P6\n# %s\n\n", MINILIGHT_URI ) )
         return ERROR_WRITE_IO;
      /* width, height, maxval */
      if( 0 > fprintf( pFileOut_o, "%i %i\n%i\n", width, height, 255 ) )
         return ERROR_WRITE_IO;
   }

   /* write pixels */
   {
      int32u i, c;
      /* (top left first) */
      for( i = 0;  i < (width * height);  ++i )
      {
         /* (R then G then B) */
         for( c = 0;  c < 3;  ++c )
         {
            const real64 channel = aPixels[(int)i].xyz[(int)c];

            /* gamma encode */
            const real64 gammaed = pow( (channel > 0.0 ? channel : 0.0),
               GAMMA_ENCODE );

            /* quantize */
            const real64 quantized = floor( (gammaed * 255.0) + 0.5 );

            /* output as byte */
            if( 0 > fprintf( pFileOut_o, "%c",
               (byteu)(quantized <= 255.0 ? quantized : 255.0) ) )
               return ERROR_WRITE_IO;
         }
      }
   }

   return 0;
}
