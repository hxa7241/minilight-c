/*------------------------------------------------------------------------------

   MiniLightMerge C : Accumulation tool for MiniLight renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <stdlib.h>

#include "Rgbe.h"

#include "Image.h"




/* implementation ----------------------------------------------------------- */

static Exception ImageRead
(
   const char*  sFilePathName,
   ImageHeader* pHeader_o,
   int32u*      pIterations_o,
   Vector3f**   paPixels_o
)
{
   Exception e;

   FILE* pFileIn = fopen( sFilePathName, "r" );
   if( !pFileIn ) return ERROR_FILE;

   e = RgbeRead( pFileIn, &pHeader_o->width, &pHeader_o->height, pIterations_o,
      paPixels_o );
   if( e ) return fclose( pFileIn ), e;

   if( EOF == fclose( pFileIn ) ) return ERROR_FILE;
   
   return 0;
}




/* initialisation ----------------------------------------------------------- */

Exception ImageConstruct
(
   int32u  width,
   int32u  height,
   Image** ppImage_o
)
{
   *ppImage_o = (Image*)calloc( 1, sizeof(Image) );
   if( !*ppImage_o ) return ERROR_ALLOC;

   (*ppImage_o)->head.width  = width;
   (*ppImage_o)->head.height = height;

   (*ppImage_o)->iterations = 0;
   (*ppImage_o)->aPixels = (Vector3f*)calloc( (*ppImage_o)->head.width *
      (*ppImage_o)->head.height, sizeof(Vector3f) );
   if( !(*ppImage_o)->aPixels )
   {
      free( *ppImage_o );
      *ppImage_o = 0;
      return ERROR_ALLOC;
   }

   return 0;
}


Exception ImageReadHeader
(
   const char*  sFilePathName,
   ImageHeader* pHeader_o
)
{
   int32u iterations;

   return ImageRead( sFilePathName, pHeader_o, &iterations, 0 );
}


Exception ImageConstructRead
(
   const char* sFilePathName,
   Image**     ppImage_o
)
{
   Exception e;

   *ppImage_o = (Image*)calloc( 1, sizeof(Image) );
   if( !*ppImage_o ) return ERROR_ALLOC;

   e = ImageRead( sFilePathName, &(*ppImage_o)->head,
      &(*ppImage_o)->iterations, &(*ppImage_o)->aPixels );
   if( e )
   {
      free( *ppImage_o );
      *ppImage_o = 0;
      return e;
   }

   return 0;
}


void ImageDestruct
(
   Image* pI
)
{
   if( pI )
   {
      if( pI->aPixels ) free( pI->aPixels );
      free( pI );
   }
}




/* queries ------------------------------------------------------------------ */

bool ImageCheckHeader
(
   const Image*       pI,
   const ImageHeader* pIh
)
{
   return
      (pI->head.width  == pIh->width)  &&
      (pI->head.height == pIh->height);
}


int32u ImageGetIterations
(
   const Image* pI
)
{
   return pI->iterations;
}


Exception ImageWrite
(
   const Image* pI,
   const char*  sFilePathName
)
{
   Exception e;

   FILE* pFileOut = fopen( sFilePathName, "wb" );
   if( !pFileOut ) return ERROR_FILE;

   e = RgbeWrite( pI->head.width, pI->head.height, pI->iterations, pI->aPixels,
      pFileOut );
   if( e ) return fclose( pFileOut ), e;

   if( EOF == fclose( pFileOut ) ) return ERROR_FILE;

   return 0;
}





/* commands ----------------------------------------------------------------- */

void ImageMultiplyByScalar
(
   Image* pI_io,
   real64 m
)
{
   int32u i;
   for( i = pI_io->head.width * pI_io->head.height;  i-- > 0; )
   {
      pI_io->aPixels[ i ] = Vector3fMulF( pI_io->aPixels + i, m );
   }
}


void ImageAddToImage
(
   Image*       pI_io,
   const Image* pOther
)
{
   int32u i;
   for( i = pI_io->head.width * pI_io->head.height;  i-- > 0; )
   {
      pI_io->aPixels[ i ] =
         Vector3fAdd( pI_io->aPixels + i, pOther->aPixels + i );
   }

   pI_io->iterations += pOther->iterations;
}
