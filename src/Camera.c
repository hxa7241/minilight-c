/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>

#include "Exceptions.h"
#include "RayTracer.h"

#include "Camera.h"




/* initialisation ----------------------------------------------------------- */

Camera CameraCreate
(
   FILE*   pIn,
   jmp_buf jmpBuf
)
{
   Camera c;

   const Vector3f Y = {{ 0.0, 1.0, 0.0 }};
   const Vector3f Z = {{ 0.0, 0.0, 1.0 }};

   /* read and condition view definition */
   {
      real32 viewAngleF = 0.0f;
      c.viewPosition  = Vector3fRead( pIn, jmpBuf );
      c.viewDirection = Vector3fRead( pIn, jmpBuf );
      throwReadExceptions( pIn, jmpBuf, 1, fscanf( pIn, "%g", &viewAngleF ) );
      c.viewAngle = (real64)viewAngleF;

      c.viewDirection = Vector3fUnitized( &c.viewDirection );
      /* if degenerate, default to Z */
      if( Vector3fIsZero( &c.viewDirection ) )
      {
         c.viewDirection = Z;
      }

      /* clamp and convert to radians */
      c.viewAngle = (c.viewAngle < VIEW_ANGLE_MIN ? VIEW_ANGLE_MIN :
         (c.viewAngle > VIEW_ANGLE_MAX ? VIEW_ANGLE_MAX : c.viewAngle)) *
         (3.14159265358979 / 180.0);
   }

   /* make other directions of view coord frame */
   {
      /* make trial 'right', using viewDirection and assuming 'up' is Y */
      const Vector3f uxv = Vector3fCross( &Y, &c.viewDirection );
      c.up    = Y;
      c.right = Vector3fUnitized( &uxv );

      /* check 'right' is valid
         -- i.e. viewDirection was not co-linear with 'up' */
      if( !Vector3fIsZero( &c.right ) )
      {
         /* use 'right', and make 'up' properly orthogonal */
         const Vector3f vxr = Vector3fCross( &c.viewDirection, &c.right );
         c.up = Vector3fUnitized( &vxr );
      }
      /* else, assume a different 'up' and redo */
      else
      {
         /* 'up' is Z if viewDirection is down, otherwise -Z */
         const Vector3f z = c.viewDirection.xyz[1] < 0.0 ?
            Z : Vector3fNegative( &Z );
         /* remake 'right' */
         const Vector3f uxv = Vector3fCross( &z, &c.viewDirection );
         c.up    = z;
         c.right = Vector3fUnitized( &uxv );
      }
   }

   return c;
}




/* queries ------------------------------------------------------------------ */

void CameraFrame
(
   const Camera* pC,
   const Scene*  pScene,
   Random*       pRandom,
   Image*        pImage_o
)
{
   const RayTracer rayTracer = RayTracerCreate( pScene );

   const real64 width  = (real64)pImage_o->width;
   const real64 height = (real64)pImage_o->height;

   /* step through image pixels, sampling them */
   int32 y, x;
   for( y = pImage_o->height;  y-- > 0; )
   {
      for( x = pImage_o->width;  x-- > 0; )
      {
         /* make sample ray direction, stratified by pixels */
         Vector3f sampleDirection;
         {
            const real64 tanView = tan( pC->viewAngle * 0.5 );

            /* make image plane XY displacement vector [-1,+1) coefficients,
               with sub-pixel jitter */
            const real64 cx = (( ((real64)x + RandomReal64( pRandom )) *
               2.0 / width  ) - 1.0) * tanView;
            const real64 cy = (( ((real64)y + RandomReal64( pRandom )) *
               2.0 / height ) - 1.0) * tanView * (height / width);

            /* make image plane offset vector,
               by scaling the view definition by the coefficients */
            const Vector3f rcx    = Vector3fMulF( &pC->right, cx );
            const Vector3f ucy    = Vector3fMulF( &pC->up,    cy );
            const Vector3f offset = Vector3fAdd( &rcx, &ucy );

            /* add image offset vector to view direction */
            const Vector3f sdv = Vector3fAdd( &pC->viewDirection, &offset );
            sampleDirection = Vector3fUnitized( &sdv );
         }

         {
            /* get radiance from RayTracer */
            const Vector3f radiance = RayTracerRadiance( &rayTracer,
               &pC->viewPosition, &sampleDirection, pRandom, 0 );

            /* add radiance to image */
            ImageAddToPixel( pImage_o, x, y, &radiance );
         }
      }
   }
}
