/*------------------------------------------------------------------------------

   MiniLightMerge C : Accumulation tool for MiniLight renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Primitives.h"
#include "Exception.h"
#include "Image.h"




/* user messages ------------------------------------------------------------ */

/* elements */
static const char TITLE[]  = "MiniLightMerge 1.0 C";
static const char AUTHOR[] = "Harrison Ainsworth / HXA7241 : 2013";
static const char URL[]    = "http://www.hxa.name/minilight";

static const char DESCRIPTION[] =
"MiniLightMerge accumulates separate MiniLight RGBE renders into a\n"
"single image.\n";
static const char USAGE[] =
"Usage:\n"
"  minilightmerge imageFilePathName imageFilePathName ...\n";

static const char DETAILS[] =
"All input images should be the same size. (Any not matching the first\n"
"are ignored.)\n"
"All input images should be unique. (Duplicates merely waste effort.)\n"
"No more than 256 input images can be given at once.\n";


/* templates */
static const char BANNER_MESSAGE[] = "\n  %s - %s\n\n";
static const char HELP_MESSAGE[]   =
   "\n  %s\n  %s\n  %s\n\n%s\n%s\n%s\n";




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

static void average
(
   char**             asFilePathName,
   const int          argc,
   const ImageHeader* pFirstHeader,
   Image*             pSumImage_o
)
{
   Exception e;
   int iName = 0;

   /* sum scaled images */
   for( ;  (iName < (argc - 1)) && (iName < 256);  ++iName )
   {
      Image* pImage;

      e = ImageConstructRead( asFilePathName[iName], &pImage );
      if( !e && ImageCheckHeader( pImage, pFirstHeader ) )
      {
         const int32u iterations = ImageGetIterations( pImage );
         ImageMultiplyByScalar( pImage, (real64)iterations );
         ImageAddToImage( pSumImage_o, pImage );

         printf( "input: (%u) %s\n", iterations, asFilePathName[iName] );
         /*printf( "\riterations: %u\n", pSumImage_o->iterations );*/
      }
      else
      {
         warning( e, asFilePathName[iName] );
      }
      ImageDestruct( pImage );
   }

   /* divide by total scaling */
   ImageMultiplyByScalar( pSumImage_o, 1.0 /
      (real64)ImageGetIterations( pSumImage_o ) );
}


static void write
(
   const Image* pSumImage,
   char**       asFilePathName
)
{
   Exception e;

   /* make output name -- "first-part.642AC151.mlm.rgbe" */
   char* sOutFilePathName = 0;
   {
      const char* pDot      = strchr( asFilePathName[0], '.' );
      const int   prefixLen = pDot ?
         (size_t)(pDot - asFilePathName[0]) :
         strlen( asFilePathName[0] );

      sOutFilePathName = (char*)calloc( prefixLen + 9 + 4 + 5 + 1,
         sizeof(char) );
      if( !sOutFilePathName ) error( ERROR_ALLOC, 0 );

      strncpy( sOutFilePathName, asFilePathName[0], prefixLen );
      strcat( sOutFilePathName, "." );
      sprintf( sOutFilePathName + prefixLen + 1, "%08X",
         (int32u)time(0) & 0xFFFFFFFFu );
      strcat( sOutFilePathName, ".mlm.rgbe" );
   }

   e = ImageWrite( pSumImage, sOutFilePathName );
   error( e, sOutFilePathName );

   printf( "output: (%u) %s\n", ImageGetIterations( pSumImage ),
      sOutFilePathName );

   free( sOutFilePathName );
}




/* entry point ************************************************************** */

int main
(
   int   argc,
   char* argv[]
)
{
   /* setup ctrl-c/interruption handler */
   signal( SIGINT, sigintHandler );

   /* check for help request */
   if( (argc <= 1) || !strcmp(argv[1], "-?") || !strcmp(argv[1], "--help") )
   {
      printf( HELP_MESSAGE, TITLE, AUTHOR, URL, DESCRIPTION, USAGE, DETAILS );
   }
   /* execute */
   else
   {
      char**    asFilePathName = argv + 1;
      Image*    pSumImage;
      Exception e;

      /* first file sets the dimensions for the rest */
      ImageHeader firstHeader;
      {
         e = ImageReadHeader( asFilePathName[0], &firstHeader );
         error( e, asFilePathName[0] );
         printf( "size:  %u %u\n", firstHeader.width, firstHeader.height );
      }

      e = ImageConstruct( firstHeader.width, firstHeader.height, &pSumImage );
      error( e, 0 );

      average( asFilePathName, argc, &firstHeader, pSumImage );
      write( pSumImage, asFilePathName );

      ImageDestruct( pSumImage );
   }

   return EXIT_SUCCESS;
}
