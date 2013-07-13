/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Vector3f_h
#define Vector3f_h


#include <stdio.h>
#include <setjmp.h>

#include "Primitives.h"




/**
 * Yes, it is the 3D vector type!.<br/><br/>
 *
 * The usual arithmetic operators, two constants, and IO -- nothing really
 * unexpected.<br/><br/>
 *
 * Constant.<br/><br/>
 */

struct Vector3f
{
   real64 xyz[3];
};

typedef struct Vector3f Vector3f;




/* initialisation ----------------------------------------------------------- */

/* just use structure initialisers instead:
      Vector3f Y = {{ 0.0, 1.0, 0.0 }} */
/*Vector3f Vector3fCreate
(
   real64 x,
   real64 y,
   real64 z
);*/




/* queries ------------------------------------------------------------------ */

real64 Vector3fDot
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fNegative
(
   const Vector3f*
);


/**
 * Unitized vector.
 *
 * * Zero vectors, and vectors of near zero magnitude, return zero vectors.
 * * Vectors of extremely large magnitude return zero vectors.
 */
Vector3f Vector3fUnitized
(
   const Vector3f*
);


Vector3f Vector3fCross
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fAdd
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fSub
(
   const Vector3f*,
   const Vector3f*
);


Vector3f Vector3fMulV
(
   const Vector3f*,
   const Vector3f*
);


/*Vector3f Vector3fDivV
(
   const Vector3f*,
   const Vector3f*
);*/


Vector3f Vector3fMulF
(
   const Vector3f*,
   real64
);


/*Vector3f Vector3fDivF
(
   const Vector3f*,
   real64
);*/


bool Vector3fIsZero
(
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
extern const Vector3f Vector3fONE;




/* io ----------------------------------------------------------------------- */

Vector3f Vector3fRead
(
   FILE*   pIn,
   jmp_buf jmpBuf
);


/*void Vector3fWrite
(
   const Vector3f* pV,
   jmp_buf         jmpBuf,
   FILE*           pOut
);*/




#endif
