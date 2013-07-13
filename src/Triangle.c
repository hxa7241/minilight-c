/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>

#include "Triangle.h"




/* constants ---------------------------------------------------------------- */

/* reasonable for single precision FP */
static const real64 EPSILON = 1.0 / 1048576.0;




/* implementation ----------------------------------------------------------- */

/**
 * The normal vector, unnormalised.
 */
static Vector3f TriangleNormalV
(
   const Triangle* pT
)
{
   const Vector3f edge1 = Vector3fSub( &pT->aVertexs[1], &pT->aVertexs[0] );
   const Vector3f edge3 = Vector3fSub( &pT->aVertexs[2], &pT->aVertexs[1] );
   return Vector3fCross( &edge1, &edge3 );
}




/* initialisation ----------------------------------------------------------- */

Triangle TriangleCreate
(
   FILE*   pIn,
   jmp_buf jmpBuf
)
{
   Triangle t;

   /* read geometry */
   {
      int i;
      for( i = 0;  i < 3;  t.aVertexs[i++] = Vector3fRead( pIn, jmpBuf ) ) {}
   }

   /* read and condition quality */
   {
      t.reflectivity = Vector3fRead( pIn, jmpBuf );
      t.reflectivity = Vector3fClamped( &t.reflectivity,
         &Vector3fZERO, &Vector3fONE );

      t.emitivity = Vector3fRead( pIn, jmpBuf );
      t.emitivity = Vector3fClamped( &t.emitivity,
         &Vector3fZERO, &t.emitivity );
   }

   return t;
}




/* queries ------------------------------------------------------------------ */

void TriangleBound
(
   const Triangle* pT,
   real64          aBound_o[6]
)
{
   int i, j, d, m;

   /* initialise to one vertex */
   for( i = 6;  i-- > 0;  aBound_o[i] = pT->aVertexs[2].xyz[i % 3] ) {}

   /* expand to surround all vertexs */
   for( i = 0;  i < 3;  ++i )
   {
      for( j = 0, d = 0, m = 0;  j < 6;  ++j, d = j / 3, m = j % 3 )
      {
         /* include some tolerance */
         const real64 v = pT->aVertexs[i].xyz[m] + ((d ? 1.0 : -1.0) *
            TOLERANCE);
         aBound_o[j] = (aBound_o[j] > v) ^ d ? v : aBound_o[j];
      }
   }
}


/**
 * @implementation
 * Adapted from:
 * <cite>'Fast, Minimum Storage Ray-Triangle Intersection';
 * Moller, Trumbore;
 * Journal Of Graphics Tools, v2n1p21; 1997.
 * http://www.acm.org/jgt/papers/MollerTrumbore97/</cite>
 */
bool TriangleIntersection
(
   const Triangle* pT,
   const Vector3f* pRayOrigin,
   const Vector3f* pRayDirection,
   real64*         pHitDistance_o
)
{
   /* make vectors for two edges sharing vert0 */
   const Vector3f edge1 = Vector3fSub( &pT->aVertexs[1], &pT->aVertexs[0] );
   const Vector3f edge2 = Vector3fSub( &pT->aVertexs[2], &pT->aVertexs[0] );

   /* begin calculating determinant - also used to calculate U parameter */
   const Vector3f pvec = Vector3fCross( pRayDirection, &edge2 );

   /* if determinant is near zero, ray lies in plane of triangle */
   const real64 det = Vector3fDot( &edge1, &pvec );

   bool isHit = false;
   if( (det <= -EPSILON) | (det >= EPSILON) )
   {
      const real64 inv_det = 1.0 / det;

      /* calculate distance from vertex 0 to ray origin */
      const Vector3f tvec = Vector3fSub( pRayOrigin, &pT->aVertexs[0] );

      /* calculate U parameter and test bounds */
      const real64 u = Vector3fDot( &tvec, &pvec ) * inv_det;
      if( (u >= 0.0) & (u <= 1.0) )
      {
         /* prepare to test V parameter */
         const Vector3f qvec = Vector3fCross( &tvec, &edge1 );

         /* calculate V parameter and test bounds */
         const real64 v = Vector3fDot( pRayDirection, &qvec ) * inv_det;
         if( (v >= 0.0) & (u + v <= 1.0) )
         {
            /* calculate t, ray intersects triangle */
            *pHitDistance_o = Vector3fDot( &edge2, &qvec ) * inv_det;

            /* only allow intersections in the forward ray direction */
            isHit = (*pHitDistance_o >= 0.0);
         }
      }
   }

   return isHit;
}


Vector3f TriangleSamplePoint
(
   const Triangle* pT,
   Random*         pRandom
)
{
   /* get two randoms */
   const real64 sqr1 = sqrt( RandomReal64( pRandom ) );
   const real64 r2   = RandomReal64( pRandom );

   /* make barycentric coords */
   const real64 c0 = 1.0 - sqr1;
   const real64 c1 = (1.0 - r2) * sqr1;
   /*const real64 c2 = r2 * sqr1;*/

   /* make barycentric axes */
   const Vector3f a0 = Vector3fSub( &pT->aVertexs[1], &pT->aVertexs[0] );
   const Vector3f a1 = Vector3fSub( &pT->aVertexs[2], &pT->aVertexs[0] );

   /* scale axes by coords */
   const Vector3f ac0 = Vector3fMulF( &a0, c0 );
   const Vector3f ac1 = Vector3fMulF( &a1, c1 );

   /* sum scaled components, and offset from corner */
   const Vector3f sum = Vector3fAdd( &ac0, &ac1 );
   return Vector3fAdd( &sum, &pT->aVertexs[0] );
}


Vector3f TriangleNormal
(
   const Triangle* pT
)
{
   const Vector3f normalV = TriangleNormalV( pT );
   return Vector3fUnitized( &normalV );
}


Vector3f TriangleTangent
(
   const Triangle* pT
)
{
   const Vector3f edge1 = Vector3fSub( &pT->aVertexs[1], &pT->aVertexs[0] );
   return Vector3fUnitized( &edge1 );
}


real64 TriangleArea
(
   const Triangle* pT
)
{
   /* half area of parallelogram (area = magnitude of cross of two edges) */
   const Vector3f normalV = TriangleNormalV( pT );
   return sqrt( Vector3fDot( &normalV, &normalV ) ) * 0.5;
}
