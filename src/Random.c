/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


/*#include <time.h>
#include <stdio.h>*/

#include "Random.h"




/* constants ---------------------------------------------------------------- */

/* default seed */
static const int32u SEED = 987654321;

/* minimum seeds */
/*static const int32u SEED_MINS[4] = { 2, 8, 16, 128 };*/




/* implementation ----------------------------------------------------------- */

/*static void getSeed
(
   int32u seed[4]
)
{
   // try to get 'non-determinate' seed //

   // UUID //
   bool isUuid = false;
   {
      int32u a[8], sc, i;
      FILE* pPf = popen( "uuidgen", "r" );

      // read in chunks of 16-bits (dodging '-'s) //
      sc = fscanf( pPf, "%4x%4x%*c%4x%*c%4x%*c%4x%*c%4x%4x%4x",
         &a[7], &a[6], &a[5], &a[4], &a[3], &a[2], &a[1], &a[0] );

      // check it worked //
      isUuid = !pclose( pPf ) && (sc == 8);
      if( isUuid )
      {
         // merge into chunks of 32-bits //
         for( i = 4;  i--;  seed[i] = (a[i * 2 + 1] << 16) | a[i * 2] ) {}
      }
   }

   // else time //
   if( !isUuid )
   {
      // probably Unix time -- signed 32-bit, seconds since 1970 //
      const time_t t = time(0);

      // check time sufficient (probably) and available //
      if( (sizeof(time_t) >= 4) && (t != -1) )
      {
         // rotate to make frequently changing bits more significant //
         const int32u tu = ((int32u)t << 8) | ((int32u)t >> 24);
         seed[0] = seed[1] = seed[2] = seed[3] = tu;
      }
   }
}*/




/* initialisation ----------------------------------------------------------- */

/*Random RandomCreate()
{
   Random r;

   // get seed //
   int32u seed[4] = { 0, 0, 0, 0 };
   getSeed( seed );

   // init state from seed //
   {
      int i;
      // *** VERY IMPORTANT ***
         The initial seeds z1, z2, z3, z4  MUST be larger
         than 1, 7, 15, and 127 respectively. //
      for( i = 4;  i--; )
      {
         r.state[i] = (seed[i] >= SEED_MINS[i]) ? seed[i] : SEED;
      }
   }

   // store seed/id as 8 digit hex number string //
   sprintf( r.sId, "%08X", r.state[3] & 0xFFFFFFFFu );

   return r;
}*/


Random RandomCreate()
{
   Random r;

   /* *** VERY IMPORTANT ***
      The initial seeds z1, z2, z3, z4  MUST be larger
      than 1, 7, 15, and 127 respectively. */

   r.state[0] = r.state[1] = r.state[2] = r.state[3] = SEED;

   return r;
}




/* queries ------------------------------------------------------------------ */

/*
 * LFSR113-LEcuyer, seed: 987654321, first few int32u rands.
 *
 * EB975594  3952563604
 * 471B9434  1192989748
 * 9078435E  2423800670
 * 49540227  1230242343
 * 2EF9F25D   788132445
 * 23C908D6   600377558
 * AE5E533A  2925417274
 * 69054221  1761952289
 */

int32u RandomInt32u
(
   Random* pR
)
{
   pR->state[0] = ((pR->state[0] & 0xFFFFFFFEu) << 18) ^
                  (((pR->state[0] <<  6) ^ pR->state[0]) >> 13);
   pR->state[1] = ((pR->state[1] & 0xFFFFFFF8u) <<  2) ^
                  (((pR->state[1] <<  2) ^ pR->state[1]) >> 27);
   pR->state[2] = ((pR->state[2] & 0xFFFFFFF0u) <<  7) ^
                  (((pR->state[2] << 13) ^ pR->state[2]) >> 21);
   pR->state[3] = ((pR->state[3] & 0xFFFFFF80u) << 13) ^
                  (((pR->state[3] <<  3) ^ pR->state[3]) >> 12);

   return pR->state[0] ^ pR->state[1] ^ pR->state[2] ^ pR->state[3];
}


/*real32 RandomReal32
(
   Random* pR
)
{
   return (real32)(int32)(RandomInt32u( pR ) & 0xFFFFFF00u) *
      (1.0f / 4294967296.0f) + 0.5f;
}*/


/*
 * LFSR113-LEcuyer, seed: 987654321, first few real64 rands.
 * (with left-to-right evaluation of int32u calls)
 *
 * 4.202779282028417107142104214290156960487365722656250000000000e-01
 * 6.433507023036078020794548137928359210491180419921875000000000e-02
 * 6.835013845195827553169465318205766379833221435546875000000000e-01
 * 1.811267868998279739756185335863847285509109497070312500000000e-01
 * 8.499654106161308453337710488995071500539779663085937500000000e-01
 * 8.761154864510395379184615194390062242746353149414062500000000e-01
 * 2.663372339453352610760816787660587579011917114257812500000000e-01
 * 1.962157346722648298964486457407474517822265625000000000000000e-01
 */

real64 RandomReal64
(
   Random* pR
)
{
   return (real64)(int32)RandomInt32u( pR ) * (1.0 / 4294967296.0) + 0.5 +
      (real64)(int32)(RandomInt32u( pR ) & 0x001FFFFFu) *
      (1.0 / 9007199254740992.0);
}


/*real64 RandomReal64_
(
   Random* pR
)
{
   return (real64)(int32)RandomInt32u( pR ) * (1.0 / 4294967296.0) +
      (0.5 + (1.0  / 4503599627370496.0) * 0.5) +
      (real64)(int32)(RandomInt32u( pR ) & 0x000FFFFFu) *
      (1.0  / 4503599627370496.0);
}*/


/*const char* RandomGetId
(
   Random* pR
)
{
   return pR->sId;
}*/
