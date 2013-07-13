/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Triangle_h
#define Triangle_h


#include <stdio.h>
#include <setjmp.h>

#include "Primitives.h"
#include "Random.h"
#include "Vector3f.h"




/**
 * Simple, explicit/non-vertex-shared triangle.<br/><br/>
 *
 * Includes geometry and quality.<br/><br/>
 *
 * Constant.<br/><br/>
 *
 * @implementation
 * Adapts ray intersection code from:
 * <cite>'Fast, Minimum Storage Ray-Triangle Intersection';
 * Moller, Trumbore;
 * Journal of Graphics Tools, v2 n1 p21; 1997.
 * http://www.acm.org/jgt/papers/MollerTrumbore97/</cite>
 *
 * @invariants
 * * reflectivity >= 0 and <= 1
 * * emitivity    >= 0
 */

struct Triangle
{
   /* geometry */
   Vector3f aVertexs[3];

   /* quality */
   Vector3f reflectivity;
   Vector3f emitivity;
};

typedef struct Triangle Triangle;




/* initialisation ----------------------------------------------------------- */

Triangle TriangleCreate
(
   FILE*   pIn,
   jmp_buf jmpBuf
);




/* queries ------------------------------------------------------------------ */

/**
 * Axis-aligned bounding box of triangle.
 *
 * @param aBound_o Lower corner in [0-2], upper corner in [3-5].
 */
void TriangleBound
(
   const Triangle*,
   real64          aBound_o[6]
);

/**
 * Intersection point of ray with triangle.
 */
bool TriangleIntersection
(
   const Triangle*,
   const Vector3f* pRayOrigin,
   const Vector3f* pRayDirection,
   real64*         pHitDistance_o
);

/**
 * Monte-carlo sample point on triangle.
 */
Vector3f TriangleSamplePoint
(
   const Triangle*,
   Random*         pRandom
);

/**
 * Normal, unitized.
 */
Vector3f TriangleNormal
(
   const Triangle*
);

/**
 * Tangent, unitized.
 */
Vector3f TriangleTangent
(
   const Triangle*
);

real64 TriangleArea
(
   const Triangle*
);




/* constants ---------------------------------------------------------------- */

/**
 * General tolerance of 1 mm seems reasonable.
 */
#define TOLERANCE (1.0 / 1024.0)




#endif
