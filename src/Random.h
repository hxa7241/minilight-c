/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Random_h
#define Random_h


#include "Primitives.h"




/**
 * Simple, fast, good random number generator.<br/><br/>
 *
 * @implementation
 *
 * 'Maximally Equidistributed Combined Tausworthe Generators'; L'Ecuyer; 1996.
 * http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme2.ps
 * http://www.iro.umontreal.ca/~simardr/rng/lfsr113.c
 *
 * 'Conversion of High-Period Random Numbers to Floating Point'; Doornik; 2006.
 * http://www.doornik.com/research/randomdouble.pdf
 */

struct Random
{
   int32u state[4];
   /*char   sId[9];*/
};

typedef struct Random Random;




/* initialisation ----------------------------------------------------------- */

/**
 * Create Random object.
 */
Random RandomCreate();




/* queries ------------------------------------------------------------------ */

int32u RandomInt32u
(
   Random*
);

/**
 * Random real, single precision, [0,1) interval (never returns 1).
 */
/*real32 RandomReal32
(
   Random*
);*/

/**
 * Random real, double precision, [0,1) interval (never returns 1).
 */
real64 RandomReal64
(
   Random*
);

/**
 * Random real, double precision, (0,1) interval (never returns 0 or 1).
 */
/*real64 RandomReal64_
(
   Random*
);*/

/**
 * Get original seed / id, as 8 digit/char hex number string.
 */
/*const char* RandomGetId
(
   Random*
);*/




#endif
