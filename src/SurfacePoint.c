/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>

#include "SurfacePoint.h"




/* constants ---------------------------------------------------------------- */

static const real64 PI = 3.14159265358979;




/* initialisation ----------------------------------------------------------- */

SurfacePoint SurfacePointCreate
(
   const Triangle* pTriangle,
   const Vector3f* pPosition
)
{
   SurfacePoint s;
   s.pTriangle = pTriangle;
   s.position  = *pPosition;

   return s;
}




/* queries ------------------------------------------------------------------ */

Vector3f SurfacePointEmission
(
   const SurfacePoint* pS,
   const Vector3f*     pToPosition,
   const Vector3f*     pOutDirection,
   bool                isSolidAngle
)
{
   const Vector3f ray       = Vector3fSub( pToPosition, &pS->position );
   const real64   distance2 = Vector3fDot( &ray, &ray );
   const Vector3f normal    = TriangleNormal( pS->pTriangle );
   const real64   cosOut    = Vector3fDot( pOutDirection, &normal );
   const real64   area      = TriangleArea( pS->pTriangle );

   /* emit from front face of surface only */
   const real64 solidAngle = (real64)(cosOut > 0.0) * (isSolidAngle ?
      /* with infinity clamped-out */
      (cosOut * area) / (distance2 >= 1e-6 ? distance2 : 1e-6) : 1.0);

   return Vector3fMulF( &pS->pTriangle->emitivity, solidAngle );
}


Vector3f SurfacePointReflection
(
   const SurfacePoint* pS,
   const Vector3f*     pInDirection,
   const Vector3f*     pInRadiance,
   const Vector3f*     pOutDirection
)
{
   const Vector3f normal = TriangleNormal( pS->pTriangle );
   const real64   inDot  = Vector3fDot( pInDirection,  &normal );
   const real64   outDot = Vector3fDot( pOutDirection, &normal );

   /* directions must be on same side of surface (no transmission) */
   const bool isSameSide = !( (inDot < 0.0) ^ (outDot < 0.0) );

   /* ideal diffuse BRDF:
      radiance scaled by reflectivity, cosine, and 1/pi  */
   const Vector3f r = Vector3fMulV( pInRadiance, &pS->pTriangle->reflectivity );
   return Vector3fMulF( &r, (fabs( inDot ) / PI) * (real64)isSameSide );
}


bool SurfacePointNextDirection
(
   const SurfacePoint* pS,
   Random*             pRandom,
   const Vector3f*     pInDirection,
   Vector3f*           pOutDirection_o,
   Vector3f*           pColor_o
)
{
   const real64 reflectivityMean =
      Vector3fDot( &pS->pTriangle->reflectivity, &Vector3fONE ) / 3.0;

   /* russian-roulette for reflectance 'magnitude' */
   const bool isAlive = RandomReal64( pRandom ) < reflectivityMean;

   if( isAlive )
   {
      /* cosine-weighted importance sample hemisphere */

      const real64 _2pr1 = PI * 2.0 * RandomReal64( pRandom );
      const real64 sr2   = sqrt( RandomReal64( pRandom ) );

      /* make coord frame coefficients (z in normal direction) */
      const real64 x = cos( _2pr1 ) * sr2;
      const real64 y = sin( _2pr1 ) * sr2;
      const real64 z = sqrt( 1.0 - (sr2 * sr2) );

      /* make coord frame */
      const Vector3f t = TriangleTangent( pS->pTriangle );
      Vector3f       n = TriangleNormal( pS->pTriangle );
      Vector3f       c;
      /* put normal on inward ray side of surface (preventing transmission) */
      if( Vector3fDot( &n, pInDirection ) < 0.0 )
      {
         n = Vector3fNegative( &n );
      }
      c = Vector3fCross( &n, &t );

      {
         /* scale frame by coefficients */
         const Vector3f tx = Vector3fMulF( &t, x );
         const Vector3f cy = Vector3fMulF( &c, y );
         const Vector3f nz = Vector3fMulF( &n, z );

         /* make direction from sum of scaled components */
         const Vector3f sum = Vector3fAdd( &tx, &cy );
         *pOutDirection_o = Vector3fAdd( &sum, &nz );
      }

      /* make color by dividing-out mean from reflectivity */
      *pColor_o = Vector3fMulF( &pS->pTriangle->reflectivity,
         1.0 / reflectivityMean );
   }

   /* discluding degenerate result direction */
   return isAlive && !Vector3fIsZero( pOutDirection_o );
}
