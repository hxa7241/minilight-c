/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Vector3f_h
#define Vector3f_h


#include "Primitives.h"




/**
 * Minimal specialised 3D vector type.<br/><br/>
 *
 * Constant.<br/><br/>
 */

struct Vector3f
{
   real64 xyz[3];
};

typedef struct Vector3f Vector3f;




/* initialisation ----------------------------------------------------------- */

/* use structure initialisers instead:
      Vector3f Y = {{ 0.0, 1.0, 0.0 }} */


/*Vector3f Vector3fCreate
(
   real64 x,
   real64 y,
   real64 z
);*/




/* queries ------------------------------------------------------------------ */

Vector3f Vector3fAdd
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fMulF
(
   const Vector3f*,
   real64
);


real64 Vector3fDot
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fClamped
(
   const Vector3f*,
   const Vector3f* min,
   const Vector3f* max
);




/* constants ---------------------------------------------------------------- */

extern const Vector3f Vector3fZERO;
/*extern const Vector3f Vector3fONE;*/




#endif
