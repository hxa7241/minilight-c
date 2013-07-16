/*------------------------------------------------------------------------------

   MiniLight tools C : Supplementary tools for MiniLight
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


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
/*const Vector3f Vector3fONE  = {{ 1.0, 1.0, 1.0 }};*/
