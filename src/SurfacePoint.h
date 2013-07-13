/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef SurfacePoint_h
#define SurfacePoint_h


#include "Primitives.h"
#include "Random.h"
#include "Vector3f.h"
#include "Triangle.h"




/**
 * Surface point at a ray-object intersection.<br/><br/>
 *
 * All direction parameters are away from surface.<br/><br/>
 *
 * Constant.<br/><br/>
  *
 * @invariants
 * * pTriangle is not 0
*/

struct SurfacePoint
{
   const Triangle* pTriangle;
   Vector3f        position;
};

typedef struct SurfacePoint SurfacePoint;




/* initialisation ----------------------------------------------------------- */

SurfacePoint SurfacePointCreate
(
   const Triangle* pTriangle,
   const Vector3f* pPosition
);




/* queries ------------------------------------------------------------------ */

/**
 * Emission from surface element to point.
 */
Vector3f SurfacePointEmission
(
   const SurfacePoint*,
   const Vector3f*     pToPosition,
   const Vector3f*     pOutDirection,
   bool                isSolidAngle
);

/**
 * Light reflection from ray to ray by surface.
 */
Vector3f SurfacePointReflection
(
   const SurfacePoint*,
   const Vector3f*     pInDirection,
   const Vector3f*     pInRadiance,
   const Vector3f*     pOutDirection
);

/**
 * Monte-carlo direction of reflection from surface.
 */
bool SurfacePointNextDirection
(
   const SurfacePoint*,
   Random*             pRandom,
   const Vector3f*     pInDirection,
   Vector3f*           pOutDirection_o,
   Vector3f*           pColor_o
);

#define SurfacePointHitId( pS ) ((const void*)(pS)->pTriangle)




#endif
