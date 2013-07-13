/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Scene_h
#define Scene_h


#include <stdio.h>
#include <setjmp.h>

#include "Primitives.h"
#include "Random.h"
#include "Vector3f.h"
#include "Triangle.h"
#include "SpatialIndex.h"




/**
 * Collection of objects in the environment.<br/><br/>
 *
 * Constant.
 *
 * @invariants
 * * trianglesLength < MAX_TRIANGLES and >= 0
 * * emittersLength  < MAX_TRIANGLES and >= 0
 * * pIndex is not 0
 * * skyEmission      >= 0
 * * groundReflection >= 0 and <= 1
 */

struct Scene
{
   /* objects */
   Triangle*     aTriangles;
   int32         trianglesLength;

   Triangle**    apEmitters;
   int32         emittersLength;

   SpatialIndex* pIndex;

   /* background */
   Vector3f      skyEmission;
   Vector3f      groundReflection;
};

typedef struct Scene Scene;




/* initialisation ----------------------------------------------------------- */

const Scene* SceneConstruct
(
   FILE*           pIn,
   jmp_buf         jmpBuf,
   const Vector3f* pEyePosition
);

void SceneDestruct
(
   Scene*
);




/* queries ------------------------------------------------------------------ */

/**
 * Find nearest intersection of ray with object.
 */
void SceneIntersection
(
   const Scene*,
   const Vector3f*  pRayOrigin,
   const Vector3f*  pRayDirection,
   const void*      lastHit,
   const Triangle** ppHitObject_o,
   Vector3f*        pHitPosition_o
);

/**
 * Monte-carlo sample point on monte-carlo selected emitting object.
 */
void SceneEmitter
(
   const Scene*,
   Random*          pRandom,
   Vector3f*        pPosition_o,
   const Triangle** pId_o
);

/**
 * Number of emitters in scene.
 */
#define SceneEmittersCount( pS ) ((pS)->emittersLength)

/**
 * Default/'background' light of scene universe.
 */
Vector3f SceneDefaultEmission
(
   const Scene*,
   const Vector3f* pBackDirection
);




/* constants ---------------------------------------------------------------- */

/**
 * Maximum number of objects in Scene.
 * (2^24 ~= 16 million)
 */
#define MAX_TRIANGLES ((int32)0x1000000)




#endif
