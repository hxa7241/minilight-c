/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <stdlib.h>

#include "Exceptions.h"

#include "SpatialIndex.h"




/* constants ---------------------------------------------------------------- */

/* accommodates scene including sun and earth, down to cm cells
   (use 47 for mm) */
static const int32 MAX_LEVELS = 44;

/* 8 seemed reasonably optimal in casual testing */
static const int32 MAX_ITEMS  =  8;




/* implementation ----------------------------------------------------------- */

static void construct
(
   const Triangle** apItems,
   const int32      itemsLength,
   const int32      level,
   jmp_buf          jmpBuf,
   SpatialIndex*    pS_o
)
{
   /* is branch if items overflow leaf and tree not too deep */
   pS_o->isBranch = (itemsLength > MAX_ITEMS) & (level < (MAX_LEVELS - 1));

   /* make branch: make sub-cells, and recurse construction */
   if( pS_o->isBranch )
   {
      int32 s, q;

      /* make subcells */
      pS_o->length  = 8;
      pS_o->apArray = (const void**)throwAllocExceptions( jmpBuf,
         calloc( pS_o->length, sizeof(const void*) ) );
      for( s = pS_o->length, q = 0;  s-- > 0; )
      {
         real64 aSubBound[6];
         int32  subItemsLength = 0;
         const Triangle** apSubItems = (const Triangle**)throwAllocExceptions(
            jmpBuf, calloc( subItemsLength, sizeof(Triangle*) ) );

         int32 j, d, m, i;

         /* make subcell bound */
         for( j = 0, d = 0, m = 0;  j < 6;  ++j, d = j / 3, m = j % 3 )
         {
            aSubBound[j] = ((s >> m) & 1) ^ d ? (pS_o->aBound[m] +
               pS_o->aBound[m + 3]) * 0.5 : pS_o->aBound[j];
         }

         /* collect items that overlap subcell */
         for( i = itemsLength;  i-- > 0; )
         {
            int32 isOverlap = 1;

            /* must overlap in all dimensions */
            real64 aItemBound[6];
            TriangleBound( apItems[i], aItemBound );
            for( j = 0, d = 0, m = 0;  j < 6;  ++j, d = j / 3, m = j % 3 )
            {
               isOverlap &= (aItemBound[(d ^ 1) * 3 + m] >= aSubBound[j]) ^ d;
            }

            /* maybe append to subitems store */
            if( isOverlap )
            {
               apSubItems = (const Triangle**)throwAllocExceptions( jmpBuf,
                  realloc( (Triangle**)apSubItems, ++subItemsLength *
                  sizeof(Triangle*) ) );
               apSubItems[subItemsLength - 1] = apItems[i];
            }
         }

         q += subItemsLength == itemsLength ? 1 : 0;

         /* maybe make subcell, if any overlapping subitems */
         if( subItemsLength > 0 )
         {
            SpatialIndex* pS = (SpatialIndex*)throwAllocExceptions( jmpBuf,
               calloc( 1, sizeof(SpatialIndex) ) );
            /* curtail degenerate subdivision by adjusting next level
               (degenerate if two or more subcells copy entire contents of
               parent, or if subdivision reaches below mm size)
               (having a model including the sun requires one subcell copying
               entire contents of parent to be allowed) */
            const int32 nextLevel = (q > 1) | ((aSubBound[3] - aSubBound[0]) <
               (TOLERANCE * 4.0)) ? MAX_LEVELS : level + 1;

            pS_o->apArray[s] = pS;
            for( i = 6;  i-- > 0;  pS->aBound[i] = aSubBound[i] ) {}

            /* recurse */
            construct( apSubItems, subItemsLength, nextLevel, jmpBuf, pS );
         }

         free( (Triangle**)apSubItems );
      }
   }
   /* make leaf: store items, and end recursion */
   else
   {
      int32 i;

      /* alloc */
      pS_o->length  = itemsLength;
      pS_o->apArray = (const void**)throwAllocExceptions( jmpBuf,
         calloc( pS_o->length, sizeof(const void*) ) );

      /* copy */
      for( i = pS_o->length;  i-- > 0;  pS_o->apArray[i] = apItems[i] ) {}
   }
}




/* initialisation ----------------------------------------------------------- */

const SpatialIndex* SpatialIndexConstruct
(
   const Vector3f* pEyePosition,
   const Triangle* aItems,
   int32           itemsLength,
   jmp_buf         jmpBuf
)
{
   SpatialIndex* pS = (SpatialIndex*)throwAllocExceptions( jmpBuf,
      calloc( 1, sizeof(SpatialIndex) ) );

   /* set overall bound (and convert to collection of pointers) */
   const Triangle** apItems = (const Triangle**)throwAllocExceptions( jmpBuf,
      calloc( itemsLength, sizeof(Triangle*) ) );
   {
      int32 i, j;

      /* accommodate eye position (makes tracing algorithm simpler) */
      for( i = 6;  i-- > 0;  pS->aBound[i] = pEyePosition->xyz[i % 3] ) {}

      /* accommodate all items */
      for( i = itemsLength;  i-- > 0;  apItems[i] = &aItems[i] )
      {
         real64 aItemBound[6];
         TriangleBound( &aItems[i], aItemBound );

         /* accommodate item */
         for( j = 0;  j < 6;  ++j )
         {
            if( (pS->aBound[j] > aItemBound[j]) ^ (j > 2) )
            {
               pS->aBound[j] = aItemBound[j];
            }
         }
      }

      /* make cubical */
      {
         real64 maxSize = 0.0, *b = 0;
         /* find max dimension */
         for( b = pS->aBound + 3;  b-- > pS->aBound; )
         {
            if( maxSize < (b[3] - b[0]) ) maxSize = b[3] - b[0];
         }
         /* set all dimensions to max */
         for( b = pS->aBound + 3;  b-- > pS->aBound; )
         {
            if( b[3] < (b[0] + maxSize) ) b[3] = b[0] + maxSize;
         }
      }
   }

   /* make subcell tree */
   construct( apItems, itemsLength, 0, jmpBuf, pS );

   free( (Triangle**)apItems );

   return pS;
}


void SpatialIndexDestruct
(
   SpatialIndex* pS
)
{
   /* recurse through branch subcells */
   int32 i;
   for( i = pS->length;  pS->isBranch & (i-- > 0); )
   {
      if( pS->apArray[i] )
      {
         SpatialIndexDestruct( (SpatialIndex*)pS->apArray[i] );
      }
   }

   free( (void**)pS->apArray );

   free( pS );
}




/* queries ------------------------------------------------------------------ */

void SpatialIndexIntersection
(
   const SpatialIndex* pS,
   const Vector3f*     pRayOrigin,
   const Vector3f*     pRayDirection,
   const void*         lastHit,
   const Vector3f*     pStart,
   const Triangle**    ppHitObject_o,
   Vector3f*           pHitPosition_o
)
{
   /* is branch: step through subcells and recurse */
   if( pS->isBranch )
   {
      int32    subCell, i;
      Vector3f cellPosition;

      pStart = pStart ? pStart : pRayOrigin;

      /* find which subcell holds ray origin (ray origin is inside cell) */
      for( subCell = 0, i = 3;  i-- > 0; )
      {
         /* compare dimension with center */
         subCell |= (pStart->xyz[i] >=
            ((pS->aBound[i] + pS->aBound[i+3]) * 0.5)) << i;
      }

      /* step through intersected subcells */
      for( cellPosition = *pStart;  ; )
      {
         int32  axis = 2, i;
         real64 step[3];

         if( pS->apArray[subCell] )
         {
            /* intersect subcell (by recursing) */
            SpatialIndexIntersection( (const SpatialIndex*)
               pS->apArray[subCell], pRayOrigin, pRayDirection, lastHit,
               &cellPosition, ppHitObject_o, pHitPosition_o );

            /* exit branch (this function) if item hit */
            if( *ppHitObject_o )
            {
               break;
            }
         }

         /* find next subcell ray moves to
            (by finding which face of the corner ahead is crossed first) */
         for( i = 3;  i-- > 0;  axis = step[i] < step[axis] ? i : axis )
         {
            /* find which face (inter-/outer-) the ray is heading for (in this
               dimension) */
            const bool   high = (subCell >> i) & 1;
            const real64 face = (pRayDirection->xyz[i] < 0.0) ^ high ?
               pS->aBound[i + (high * 3)] :
               (pS->aBound[i] + pS->aBound[i + 3]) * 0.5;
            /* calculate distance to face
               (div by zero produces infinity, which is later discarded) */
            step[i] = (face - pRayOrigin->xyz[i]) / pRayDirection->xyz[i];
            /* last clause of for-statement notes nearest so far */
         }

         /* leaving branch if: direction is negative and subcell is low,
            or direction is positive and subcell is high */
         if( ((subCell >> axis) & 1) ^ (pRayDirection->xyz[axis] < 0.0) )
         {
            break;
         }

         /* move to (outer face of) next subcell */
         {
            const Vector3f rs = Vector3fMulF( pRayDirection, step[axis] );
            cellPosition = Vector3fAdd( pRayOrigin, &rs );
            subCell      = subCell ^ (1 << axis);
         }
      }
   }
   /* is leaf: exhaustively intersect contained items */
   else
   {
      real64 nearestDistance = REAL64_MAX;
      int32 i;

      *ppHitObject_o = 0;

      /* step through items */
      for( i = pS->length;  i-- > 0; )
      {
         const Triangle* pItem = (const Triangle*)(pS->apArray[i]);

         /* avoid spurious intersection with surface just come from */
         if( pItem != lastHit )
         {
            /* intersect ray with item, and inspect if nearest so far */
            real64 distance = REAL64_MAX;
            if( TriangleIntersection( pItem, pRayOrigin, pRayDirection,
               &distance ) && (distance < nearestDistance) )
            {
               /* check intersection is inside cell bound (with tolerance) */
               const Vector3f ray = Vector3fMulF( pRayDirection, distance );
               const Vector3f hit = Vector3fAdd( pRayOrigin, &ray );
               if( (pS->aBound[0] - hit.xyz[0] <= TOLERANCE) &
                   (hit.xyz[0] - pS->aBound[3] <= TOLERANCE) &
                   (pS->aBound[1] - hit.xyz[1] <= TOLERANCE) &
                   (hit.xyz[1] - pS->aBound[4] <= TOLERANCE) &
                   (pS->aBound[2] - hit.xyz[2] <= TOLERANCE) &
                   (hit.xyz[2] - pS->aBound[5] <= TOLERANCE) )
               {
                  /* note nearest so far */
                  *ppHitObject_o  = pItem;
                  nearestDistance = distance;
                  *pHitPosition_o = hit;
               }
            }
         }
      }
   }
}
