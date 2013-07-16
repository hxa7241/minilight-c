/*------------------------------------------------------------------------------

   MiniLight C : minimal global illumination renderer
   Harrison Ainsworth / HXA7241 : 2009, 2011, 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Primitives.h"
#include "Exceptions.h"
#include "Random.h"
#include "Image.h"
#include "Scene.h"
#include "Camera.h"




/**
 * Control-module and entry point.<br/><br/>
 *
 * Handles command-line UI, and runs the main progressive-refinement render
 * loop.<br/><br/>
 *
 * Supply a model file pathname as the command-line argument. Or -? for help.
 */


/* user messages ------------------------------------------------------------ */

/* elements */
static const char TITLE[]  = "MiniLight 1.7 C";
static const char AUTHOR[] = "Harrison Ainsworth / HXA7241 : 2009, 2011, 2013";
static const char URL[]    = "http://www.hxa.name/minilight";
static const char DATE[]   = "2013-05-04";
static const char LINE[]   =
"----------------------------------------------------------------------\n";
static const char DESCRIPTION[] =
"MiniLight is a minimal global illumination renderer.";
static const char USAGE[] =
"usage:\n"
"  minilight modelFilePathName\n"
"\n"
"The model text file format is:\n"
"  #MiniLight\n"
"\n"
"  iterations\n"
"\n"
"  imagewidth imageheight\n"
"  viewposition viewdirection viewangle\n"
"\n"
"  skyemission groundreflection\n"
"\n"
"  vertex0 vertex1 vertex2 reflectivity emitivity\n"
"  vertex0 vertex1 vertex2 reflectivity emitivity\n"
"  ...\n"
"\n"
"- where iterations and image values are integers, viewangle is a real,\n"
"and all other values are three parenthised reals. The file must end\n"
"with a newline. For example:\n";
static const char EXAMPLE[] =
"  #MiniLight\n"
"\n"
"  100\n"
"\n"
"  200 150\n"
"  (0 0.75 -2) (0 0 1) 45\n"
"\n"
"  (3626 5572 5802) (0.1 0.09 0.07)\n"
"\n"
"  (0 0 0) (0 1 0) (1 1 0)  (0.7 0.7 0.7) (0 0 0)\n"
"\n";

/* templates */
static const char BANNER_MESSAGE[] = "\n  %s - %s\n\n";
static const char HELP_MESSAGE[]   =
   "\n%s  %s\n\n  %s\n  %s\n\n  %s\n%s\n%s\n\n%s%s";




/* constants ---------------------------------------------------------------- */

static const char MODEL_FORMAT_ID[] = "#MiniLight";

#define ERROR_FORMAT_UNREC 1
#define ERROR_FILE         128




/* ctrl-c handling ---------------------------------------------------------- */

void sigintHandler
(
   int arg
)
{
   /* exit if called (asynchronously) by an interrupt */
   if( SIGINT == arg )
   {
      printf( "\ninterrupted\n" );
      exit( EXIT_SUCCESS );
   }
}




/* implementation ----------------------------------------------------------- */

static void makeRenderingObjects
(
   jmp_buf       jmpBuf,
   char*         argv[],
   Random*       pRandom_o,
   char**        psImageFilePathname_o,
   int32*        pIterations_o,
   Image**       ppImage_o,
   Camera*       pCamera_o,
   const Scene** ppScene_o
)
{
   FILE* pModelFile;
   const char* sModelFilePathname = 0;

   /* make random generator */
   *pRandom_o = RandomCreate();

   /* get/make file names */
   sModelFilePathname = argv[ 1 ];
   *psImageFilePathname_o = (char*)throwAllocExceptions( jmpBuf,
      calloc( strlen(sModelFilePathname) + 15, sizeof(char) ) );
   strcpy( *psImageFilePathname_o, sModelFilePathname );
   strcat( strcat( *psImageFilePathname_o, "." ), RandomGetId( pRandom_o ) );
   strcat( *psImageFilePathname_o, ".rgbe" );

   /* open model file */
   pModelFile = fopen( sModelFilePathname, "r" );
   throwExceptions( jmpBuf, !pModelFile, ERROR_FILE );

   /* check model file format identifier at start of first line */
   {
      long p;
      /* read chars until a mismatch */
      throwReadExceptions( pModelFile, jmpBuf, 0,
         fscanf( pModelFile, MODEL_FORMAT_ID ) );
      p = ftell( pModelFile );
      throwExceptions( jmpBuf, (-1L == p), ERROR_FILE );
      /* check if all chars were read */
      throwExceptions( jmpBuf,
         ((long)strlen( MODEL_FORMAT_ID ) != p), ERROR_FORMAT_UNREC );
   }

   /* read and condition frame iterations */
   *pIterations_o = 0;
   throwReadExceptions( pModelFile, jmpBuf, 1,
      fscanf( pModelFile, "%i", pIterations_o ) );
   *pIterations_o = *pIterations_o < 0 ? 0 : *pIterations_o;

   /* create main rendering objects, from model file */
   *ppImage_o = ImageConstruct( pModelFile, jmpBuf );
   *pCamera_o = CameraCreate( pModelFile, jmpBuf );
   *ppScene_o = SceneConstruct( pModelFile, jmpBuf,
      &CameraEyePoint( pCamera_o ) );

   /* close model file */
   throwExceptions( jmpBuf, (EOF == fclose( pModelFile )), ERROR_FILE );
}


static void renderProgressively
(
   jmp_buf       jmpBuf,
   const int32   iterations,
   const Camera* pCamera,
   const Scene*  pScene,
   Random*       pRandom,
   const char*   sImageFilePathname,
   Image*        pImage_o
)
{
   /* do progressive refinement render loop */
   int32 frameNo;
   for( frameNo = 1;  frameNo <= iterations;  ++frameNo )
   {
      /* display current iteration number */
      printf( "\riteration: %i", frameNo );
      fflush( stdout );

      /* render a frame */
      CameraFrame( pCamera, pScene, pRandom, pImage_o );

      /* save image at twice error-halving rate, and at start and end */
      if( ((frameNo & (frameNo - 1)) == 0) | (iterations == frameNo) )
      {
         /* open image file */
         FILE* pImageFile = fopen( sImageFilePathname, "wb" );
         throwExceptions( jmpBuf, !pImageFile, ERROR_FILE );

         /* write image frame to file */
         ImageFormatted( pImage_o, frameNo, jmpBuf, pImageFile );

         throwExceptions( jmpBuf, (EOF == fclose( pImageFile )), ERROR_FILE );
      }
   }
}




/* entry point ************************************************************** */

int main
(
   int   argc,
   char* argv[]
)
{
   int returnValue = EXIT_FAILURE;

   jmp_buf     jmpBuf;
   const char* sException = 0;

   /* extract thrown value */
   switch( setjmp( jmpBuf ) )
   {
      case 0                  : sException = 0;                           break;
      case ERROR_READ_INVAL   : sException = "invalid model syntax";      break;
      case ERROR_READ_TRUNC   : sException = "truncated model file";      break;
      case ERROR_READ_IO      : sException = "I/O read error";            break;
      case ERROR_WRITE_IO     : sException = "I/O write error";           break;
      case ERROR_FORMAT_UNREC : sException = "unrecognised model format"; break;
      case ERROR_FILE         : sException = "file error";                break;
      case ERROR_ALLOC        : sException = "storage allocation error";  break;
      default                 : sException = "(unspecified error)";       break;
   }
   /* try */
   if( !sException )
   {
      /* check for help request */
      if( (argc <= 1) || !strcmp(argv[1], "-?") || !strcmp(argv[1], "--help") )
      {
         printf( HELP_MESSAGE, LINE, TITLE, AUTHOR, URL, DATE, LINE,
            DESCRIPTION, USAGE, EXAMPLE );
      }
      /* execute */
      else
      {
         Random       random;
         char*        sImageFilePathname;
         int32        iterations;
         Image*       pImage;
         Camera       camera;
         const Scene* pScene;

         printf( BANNER_MESSAGE, TITLE, URL );

         /* setup ctrl-c/interruption handler */
         signal( SIGINT, sigintHandler );
         /*throwExceptions( jmpBuf_g,
            (signal( SIGINT, sigintHandler ) == SIG_ERR), ERROR_UNSPECIFIED );*/

         makeRenderingObjects( jmpBuf, argv, &random, &sImageFilePathname,
            &iterations, &pImage, &camera, &pScene );

         printf( "output: %s\n", sImageFilePathname );

         renderProgressively( jmpBuf, iterations, &camera, pScene, &random,
            sImageFilePathname, pImage );

         printf( "\nfinished\n" );

         SceneDestruct( (Scene*)pScene );
         ImageDestruct( pImage );
         free( sImageFilePathname );
      }

      returnValue = EXIT_SUCCESS;
   }
   /* catch everything */
   else
   {
      /* print exception message */
      printf( "\n*** execution failed:  %s\n", sException );

      returnValue = EXIT_FAILURE;
   }

   return returnValue;
}
