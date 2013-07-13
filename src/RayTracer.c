/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include "SurfacePoint.h"

#include "RayTracer.h"




/* implementation ----------------------------------------------------------- */

/**
 * Radiance from an emitter sample.
 */
static Vector3f sampleEmitters
(
   const RayTracer*    pR,
   const Vector3f*     pRayBackDirection,
   const SurfacePoint* pSurfacePoint,
   Random*             pRandom
)
{
   Vector3f radiance = Vector3fZERO;

   /* single emitter sample, ideal diffuse BRDF:
         reflected = (emitivity * solidangle) * (emitterscount) *
            (cos(emitdirection) / pi * reflectivity)
      -- SurfacePoint does the first and last parts (in separate methods) */

   /* get position on an emitter */
   Vector3f        emitterPosition;
   const Triangle* emitterId = 0;
   SceneEmitter( pR->pScene, pRandom, &emitterPosition, &emitterId );

   /* check an emitter was found */
   if( emitterId )
   {
      /* make direction to emit point */
      const Vector3f emitVector    = Vector3fSub( &emitterPosition,
         &pSurfacePoint->position );
      const Vector3f emitDirection = Vector3fUnitized( &emitVector );

      /* send shadow ray */
      const Triangle* pHitObject = 0;
      Vector3f        hitPosition;
      SceneIntersection( pR->pScene, &pSurfacePoint->position, &emitDirection,
         SurfacePointHitId( pSurfacePoint ), &pHitObject, &hitPosition );

      /* check if unshadowed */
      if( !pHitObject | (emitterId == pHitObject) )
      {
         /* get inward emission value */
         const SurfacePoint sp            = SurfacePointCreate( emitterId,
            &emitterPosition );
         const Vector3f backEmitDirection = Vector3fNegative( &emitDirection );
         const Vector3f emissionIn        = SurfacePointEmission( &sp,
            &pSurfacePoint->position, &backEmitDirection, true );
         const Vector3f emissionAll       = Vector3fMulF( &emissionIn,
            (real64)SceneEmittersCount( pR->pScene ) );

         /* get amount reflected by surface */
         radiance = SurfacePointReflection( pSurfacePoint, &emitDirection,
            &emissionAll, pRayBackDirection );
      }
   }

   return radiance;
}




/* initialisation ----------------------------------------------------------- */

RayTracer RayTracerCreate
(
   const Scene* pScene
)
{
   RayTracer r;
   r.pScene = pScene;

   return r;
}




/* queries ------------------------------------------------------------------ */

Vector3f RayTracerRadiance
(
   const RayTracer* pR,
   const Vector3f*  pRayOrigin,
   const Vector3f*  pRayDirection,
   Random*          pRandom,
   const void*      lastHit
)
{
   Vector3f radiance;

   const Vector3f rayBackDirection = Vector3fNegative( pRayDirection );

   /* intersect ray with scene */
   const Triangle* pHitObject = 0;
   Vector3f        hitPosition;
   SceneIntersection( pR->pScene, pRayOrigin, pRayDirection, lastHit,
      &pHitObject, &hitPosition );

   if( pHitObject )
   {
      /* make surface point of intersection */
      const SurfacePoint surfacePoint = SurfacePointCreate( pHitObject,
         &hitPosition );

      /* local emission (only for first-hit) */
      const Vector3f localEmission = lastHit ? Vector3fZERO :
         SurfacePointEmission( &surfacePoint, pRayOrigin, &rayBackDirection,
         false );

      /* emitter sample */
      const Vector3f emitterSample = sampleEmitters( pR, &rayBackDirection,
         &surfacePoint, pRandom );

      /* recursed reflection */
      Vector3f recursedReflection = Vector3fZERO;
      {
         /* single hemisphere sample, ideal diffuse BRDF:
               reflected = (inradiance * pi) * (cos(in) / pi * color) *
                  reflectance
            -- reflectance magnitude is 'scaled' by the russian roulette,
            cos is importance sampled (both done by SurfacePoint),
            and the pi and 1/pi cancel out -- leaving just:
               inradiance * reflectance color */
         Vector3f nextDirection;
         Vector3f color;
         /* check surface reflects ray */
         if( SurfacePointNextDirection( &surfacePoint, pRandom,
            &rayBackDirection, &nextDirection, &color ) )
         {
            /* recurse */
            const Vector3f recursed = RayTracerRadiance( pR,
               &surfacePoint.position, &nextDirection, pRandom,
               SurfacePointHitId( &surfacePoint ) );
            recursedReflection = Vector3fMulV( &recursed, &color );
         }
      }

      /* sum components */
      radiance = Vector3fAdd( &localEmission, &emitterSample );
      radiance = Vector3fAdd( &recursedReflection, &radiance );
   }
   else
   {
      /* no hit: default/background scene emission */
      radiance = SceneDefaultEmission( pR->pScene, &rayBackDirection );
   }

   return radiance;
}
