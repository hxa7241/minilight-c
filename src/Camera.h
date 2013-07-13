/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#ifndef Camera_h
#define Camera_h


#include <stdio.h>
#include <setjmp.h>

#include "Random.h"
#include "Vector3f.h"
#include "Image.h"
#include "Scene.h"




/**
 * View definition and rasterizer.<br/><br/>
 *
 * CameraFrame() accumulates a frame to the image.<br/><br/>
 *
 * Constant.
 *
 * @invariants
 * * viewAngle is >= 10 and <= 160 degrees, in radians
 * * viewDirection is unitized
 * * right is unitized
 * * up is unitized
 * * viewDirection, right, and up form a coordinate frame
 */

struct Camera
{
   /* eye definition */
   Vector3f viewPosition;
   real64   viewAngle;

   /* view frame */
   Vector3f viewDirection;
   Vector3f right;
   Vector3f up;
};

typedef struct Camera Camera;




/* initialisation ----------------------------------------------------------- */

Camera CameraCreate
(
   FILE*   pIn,
   jmp_buf jmpBuf
);




/* queries ------------------------------------------------------------------ */

/**
 * Position of the eye.
 */
#define CameraEyePoint( pC ) ((pC)->viewPosition)

/**
 * Accumulate a frame of samples to the image.
 */
void CameraFrame
(
   const Camera*,
   const Scene*  pScene,
   Random*       pRandom,
   Image*        pImage_o
);




/* constants ---------------------------------------------------------------- */

/**
 * View angle max and min.
 */
#define VIEW_ANGLE_MIN  10.0
#define VIEW_ANGLE_MAX 160.0




#endif
