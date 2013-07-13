/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef SpatialIndex_h
#define SpatialIndex_h


#include <setjmp.h>

#include "Primitives.h"
#include "Vector3f.h"
#include "Triangle.h"




/**
 * A minimal spatial index for ray tracing.<br/><br/>
 *
 * Suitable for a scale of 1 metre == 1 numerical unit, and with a resolution
 * of 1 millimetre. (Implementation uses fixed tolerances)
 *
 * Constant.<br/><br/>
 *
 * @implementation
 * A crude State pattern: typed by isBranch field to be either a branch
 * or leaf cell.<br/><br/>
 *
 * Octree: axis-aligned, cubical. Subcells are numbered thusly:
 * <pre>      110---111
 *            /|    /|
 *         010---011 |
 *    y z   | 100-|-101
 *    |/    |/    | /
 *    .-x  000---001      </pre><br/><br/>
 *
 * Each cell stores its bound (fatter data, but simpler code).<br/><br/>
 *
 * Calculations for building and tracing are absolute rather than incremental --
 * so quite numerically solid. Uses tolerances in: bounding triangles (in
 * TriangleBound), and checking intersection is inside cell (both effective
 * for axis-aligned items). Also, depth is constrained to an absolute subcell
 * size (easy way to handle overlapping items).
 *
 * @invariants
 * * aBound[0-2] <= aBound[3-5]
 * * bound encompasses the cell's contents
 * if isBranch
 * * apArray elements are SpatialIndex pointers or zeros
 * * length (of apArray) is 8
 * else
 * * apArray elements are non-zero Triangle pointers
 */

struct SpatialIndex
{
   bool         isBranch;
   real64       aBound[6];
   const void** apArray;
   int32        length;
};

typedef struct SpatialIndex SpatialIndex;




/* initialisation ----------------------------------------------------------- */

const SpatialIndex* SpatialIndexConstruct
(
   const Vector3f* pEyePosition,
   const Triangle* aItems,
   int32           itemsLength,
   jmp_buf         jmpBuf
);

void SpatialIndexDestruct
(
   SpatialIndex*
);




/* queries ------------------------------------------------------------------ */

/**
 * Find nearest intersection of ray with item.
 */
void SpatialIndexIntersection
(
   const SpatialIndex*,
   const Vector3f*     pRayOrigin,
   const Vector3f*     pRayDirection,
   const void*         lastHit,
   const Vector3f*     null,
   const Triangle**    ppHitObject_o,
   Vector3f*           pHitPosition_o
);




#endif
