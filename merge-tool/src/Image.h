/*------------------------------------------------------------------------------

   MiniLightMerge C : Accumulation tool for MiniLight renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Image_h
#define Image_h


#include "Primitives.h"
#include "Exception.h"
#include "Vector3f.h"




struct ImageHeader
{
   int32u width;
   int32u height;
};

typedef struct ImageHeader ImageHeader;


struct Image
{
   ImageHeader head;
   int32u      iterations;

   Vector3f*   aPixels;
};

typedef struct Image Image;




/* initialisation ----------------------------------------------------------- */

Exception ImageConstruct
(
   int32u  width,
   int32u  height,
   Image** ppImage_o
);


Exception ImageReadHeader
(
   const char*  sFilePathName,
   ImageHeader* pHeader_o
);


Exception ImageConstructRead
(
   const char* sFilePathName,
   Image**     ppImage_o
);


void ImageDestruct
(
   Image* pI
);




/* queries ------------------------------------------------------------------ */

bool ImageCheckHeader
(
   const Image*       pI,
   const ImageHeader* pIh
);


int32u ImageGetIterations
(
   const Image* pI
);


Exception ImageWrite
(
   const Image* pI,
   const char*  sFilePathName
);




/* commands ----------------------------------------------------------------- */

void ImageMultiplyByScalar
(
   Image* pI,
   real64 m
);


void ImageAddToImage
(
   Image*       pI,
   const Image* pOther
);




#endif
