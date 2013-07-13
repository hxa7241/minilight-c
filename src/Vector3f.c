/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>
#include <string.h>

#include "Exceptions.h"

#include "Vector3f.h"




/* initialisation ----------------------------------------------------------- */

/*Vector3f Vector3fCreate
(
   const real64 x,
   const real64 y,
   const real64 z
)
{
   Vector3f v;
   v.xyz[0] = x;
   v.xyz[1] = y;
   v.xyz[2] = z;

   return v;
}*/




/* queries ------------------------------------------------------------------ */

real64 Vector3fDot
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   return (pV0->xyz[0] * pV1->xyz[0]) +
          (pV0->xyz[1] * pV1->xyz[1]) +
          (pV0->xyz[2] * pV1->xyz[2]);
}


Vector3f Vector3fNegative
(
   const Vector3f* pV
)
{
   Vector3f v;
   v.xyz[0] = -pV->xyz[0];
   v.xyz[1] = -pV->xyz[1];
   v.xyz[2] = -pV->xyz[2];

   return v;
}


Vector3f Vector3fUnitized
(
   const Vector3f* pV
)
{
   /* Zero vectors, and vectors of near zero magnitude, produce zero length,
      and (since 1 / 0 is conditioned to 0) ultimately a zero vector result.
      Vectors of extremely large magnitude produce +infinity length, and (since
      1 / inf is 0) ultimately a zero vector result.
      (Perhaps zero vectors should produce infinite results, but pragmatically,
      zeros are probably easier to handle than infinities.) */

   const real64 length        = sqrt( Vector3fDot( pV, pV ) );
   const real64 oneOverLength = length != 0.0 ? 1.0 / length : 0.0;

   return Vector3fMulF( pV, oneOverLength );
}


Vector3f Vector3fCross
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   Vector3f v;
   v.xyz[0] = (pV0->xyz[1] * pV1->xyz[2]) - (pV0->xyz[2] * pV1->xyz[1]);
   v.xyz[1] = (pV0->xyz[2] * pV1->xyz[0]) - (pV0->xyz[0] * pV1->xyz[2]);
   v.xyz[2] = (pV0->xyz[0] * pV1->xyz[1]) - (pV0->xyz[1] * pV1->xyz[0]);

   return v;
}


Vector3f Vector3fAdd
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   Vector3f v;
   v.xyz[0] = pV0->xyz[0] + pV1->xyz[0];
   v.xyz[1] = pV0->xyz[1] + pV1->xyz[1];
   v.xyz[2] = pV0->xyz[2] + pV1->xyz[2];

   return v;
}


Vector3f Vector3fSub
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   Vector3f v;
   v.xyz[0] = pV0->xyz[0] - pV1->xyz[0];
   v.xyz[1] = pV0->xyz[1] - pV1->xyz[1];
   v.xyz[2] = pV0->xyz[2] - pV1->xyz[2];

   return v;
}


Vector3f Vector3fMulV
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   Vector3f v;
   v.xyz[0] = pV0->xyz[0] * pV1->xyz[0];
   v.xyz[1] = pV0->xyz[1] * pV1->xyz[1];
   v.xyz[2] = pV0->xyz[2] * pV1->xyz[2];

   return v;
}


/*Vector3f Vector3fDivV
(
   const Vector3f* pV0,
   const Vector3f* pV1
)
{
   Vector3f v;
   v.xyz[0] = pV0->xyz[0] / pV1->xyz[0];
   v.xyz[1] = pV0->xyz[1] / pV1->xyz[1];
   v.xyz[2] = pV0->xyz[2] / pV1->xyz[2];

   return v;
}*/


Vector3f Vector3fMulF
(
   const Vector3f* pV,
   real64          f
)
{
   Vector3f v;
   v.xyz[0] = pV->xyz[0] * f;
   v.xyz[1] = pV->xyz[1] * f;
   v.xyz[2] = pV->xyz[2] * f;

   return v;
}


/*Vector3f Vector3fDivF
(
   const Vector3f* pV,
   const real64    f
)
{
   const real64 oneOverF = 1.0 / f;

   Vector3f v;
   v.xyz[0] = pV->xyz[0] * oneOverF;
   v.xyz[1] = pV->xyz[1] * oneOverF;
   v.xyz[2] = pV->xyz[2] * oneOverF;

   return v;
}*/


bool Vector3fIsZero
(
   const Vector3f* pV
)
{
   return (pV->xyz[0] == 0.0) & (pV->xyz[1] == 0.0) & (pV->xyz[2] == 0.0);
}


Vector3f Vector3fClamped
(
   const Vector3f* pV,
   const Vector3f* pMin,
   const Vector3f* pMax
)
{
   Vector3f v;

   int i;
   for( i = 3;  i-- > 0; )
   {
      v.xyz[i] = pV->xyz[i] < pMin->xyz[i] ? pMin->xyz[i] :
                (pV->xyz[i] > pMax->xyz[i] ? pMax->xyz[i] : pV->xyz[i]);
   }

   return v;
}




/* constants ---------------------------------------------------------------- */

const Vector3f Vector3fZERO = {{ 0.0, 0.0, 0.0 }};
const Vector3f Vector3fONE  = {{ 1.0, 1.0, 1.0 }};




/* io ----------------------------------------------------------------------- */

Vector3f Vector3fRead
(
   FILE*   pIn,
   jmp_buf jmpBuf
)
{
   Vector3f v;
   int      i;

   char  s[3];
   float f[3];
   throwReadExceptions( pIn, jmpBuf, 3, fscanf( pIn, "%1s %g %g %g %1s",
      s, &f[0], &f[1], &f[2], s + 1 ) );
   throwExceptions( jmpBuf, (bool)strcmp( s, "()" ), ERROR_READ_INVAL );

   for( i = 3;  i-- > 0;  v.xyz[i] = (real64)f[i] ) {}

   return v;
}


/*void Vector3fWrite
(
   const Vector3f* pV,
   jmp_buf         jmpBuf,
   FILE*           pOut
)
{
   throwWriteExceptions( pOut, jmpBuf, fprintf( pOut,
      "(% #6.6g % #6.6g % #6.6g)", pV->xyz[0], pV->xyz[1], pV->xyz[2] ) );
}*/
