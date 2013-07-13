/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef RayTracer_h
#define RayTracer_h


#include "Random.h"
#include "Vector3f.h"
#include "Scene.h"




/**
 * Ray tracer for general light transport.<br/><br/>
 *
 * Traces a path with emitter sampling: A single chain of ray-steps advances
 * from the eye into the scene with one sampling of emitters at each
 * node.<br/><br/>
 *
 * Constant.
 *
 * @invariants
 * * pScene is not 0
 */

struct RayTracer
{
   const Scene* pScene;
};

typedef struct RayTracer RayTracer;




/* initialisation ----------------------------------------------------------- */

RayTracer RayTracerCreate
(
   const Scene*
);




/* queries ------------------------------------------------------------------ */

/**
 * Radiance returned from a trace.
 */
Vector3f RayTracerRadiance
(
   const RayTracer*,
   const Vector3f*  pRayOrigin,
   const Vector3f*  pRayDirection,
   Random*          pRandom,
   const void*      null
);




#endif
