/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <stdlib.h>
#include <math.h>

#include "Exceptions.h"

#include "Scene.h"




/* initialisation ----------------------------------------------------------- */

const Scene* SceneConstruct
(
   FILE*           pIn,
   jmp_buf         jmpBuf,
   const Vector3f* pEyePosition
)
{
   Scene* pS = (Scene*)throwAllocExceptions( jmpBuf,
      calloc( 1, sizeof(Scene) ) );

   /* read and condition background sky and ground values */
   {
      pS->skyEmission      = Vector3fRead( pIn, jmpBuf );
      pS->groundReflection = Vector3fRead( pIn, jmpBuf );

      pS->skyEmission = Vector3fClamped( &pS->skyEmission,
         &Vector3fZERO, &pS->skyEmission );
      pS->groundReflection = Vector3fClamped( &pS->groundReflection,
         &Vector3fZERO, &Vector3fONE );
   }

   /* read objects, until end of file or until maximum reached */
   {
      int32 i;

      pS->aTriangles = (Triangle*)throwAllocExceptions( jmpBuf,
         calloc( 0, sizeof(Triangle) ) );
      pS->trianglesLength = 0;

      for( i = 0;  i < MAX_TRIANGLES;  ++i )
      {
         /* stop reading if no more objects */
         {
            /* read next non blank char */
            char s[2];
            const int r = fscanf( pIn, "%1s", s );

            /* throw non-EOF failure */
            const int code = ferror( pIn ) ? ERROR_READ_IO : 0;
            clearerr( pIn );
            throwExceptions( jmpBuf, (bool)code, code );

            /* if char was found, put back, else end reading */
            if( 1 == r )
            {
               throwExceptions( jmpBuf, (EOF == ungetc( s[0], pIn )),
                  ERROR_READ_IO );
            }
            else
            {
               break;
            }
         }

         /* read an object */
         {
            const Triangle t = TriangleCreate( pIn, jmpBuf );

            /* append to objects storage */
            pS->aTriangles = (Triangle*)throwAllocExceptions( jmpBuf,
               realloc( pS->aTriangles, ++pS->trianglesLength *
               sizeof(Triangle) ) );
            pS->aTriangles[pS->trianglesLength - 1] = t;
         }
      }
   }

   /* find emitting objects */
   {
      int32 i;

      pS->apEmitters = (Triangle**)throwAllocExceptions( jmpBuf,
         calloc( 0, sizeof(Triangle*) ) );
      pS->emittersLength = 0;

      for( i = 0;  i < pS->trianglesLength;  ++i )
      {
         /* has non-zero emission and area */
         if( !Vector3fIsZero( &pS->aTriangles[i].emitivity ) &&
            (TriangleArea( &pS->aTriangles[i] ) > 0.0) )
         {
            /* append to emitters storage */
            pS->apEmitters = (Triangle**)throwAllocExceptions( jmpBuf,
               realloc( pS->apEmitters, ++pS->emittersLength *
               sizeof(Triangle*) ) );
            pS->apEmitters[pS->emittersLength - 1] = &(pS->aTriangles[i]);
         }
      }
   }

   /* make index of objects */
   pS->pIndex = (SpatialIndex*)SpatialIndexConstruct( pEyePosition,
      pS->aTriangles, pS->trianglesLength, jmpBuf );

   return pS;
}


void SceneDestruct
(
   Scene* pS
)
{
   SpatialIndexDestruct( pS->pIndex );
   free( pS->apEmitters );
   free( pS->aTriangles );

   free( pS );
}




/* queries ------------------------------------------------------------------ */

void SceneIntersection
(
   const Scene*     pS,
   const Vector3f*  pRayOrigin,
   const Vector3f*  pRayDirection,
   const void*      lastHit,
   const Triangle** ppHitObject_o,
   Vector3f*        pHitPosition_o
)
{
   SpatialIndexIntersection( pS->pIndex, pRayOrigin, pRayDirection, lastHit,
      0, ppHitObject_o, pHitPosition_o );
}


void SceneEmitter
(
   const Scene*     pS,
   Random*          pRandom,
   Vector3f*        pPosition_o,
   const Triangle** pId_o
)
{
   if( pS->emittersLength > 0 )
   {
      /* select emitter */
      int32 index = (int32)floor( RandomReal64( pRandom ) *
         (real64)pS->emittersLength );
      index = index < pS->emittersLength ? index : pS->emittersLength - 1;

      /* choose position on emitter */
      *pPosition_o = TriangleSamplePoint( pS->apEmitters[index], pRandom );
      *pId_o       = pS->apEmitters[index];
   }
   else
   {
      *pPosition_o = Vector3fZERO;
      *pId_o       = 0;
   }
}


Vector3f SceneDefaultEmission
(
   const Scene*    pS,
   const Vector3f* pBackDirection
)
{
   /* sky for downward ray, ground for upward ray */
   return (pBackDirection->xyz[1] < 0.0) ?
      pS->skyEmission : Vector3fMulV( &pS->skyEmission, &pS->groundReflection );
}
