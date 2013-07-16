/*------------------------------------------------------------------------------

   MiniLightTone C : Tone-mapper tool for MiniLight RGBE renders
   Harrison Ainsworth / HXA7241 : 2013

   http://www.hxa.name/minilight

------------------------------------------------------------------------------*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Primitives.h"
#include "Exception.h"
#include "Image.h"




/* user messages ------------------------------------------------------------ */

/* elements */
static const char TITLE[]  = "MiniLightTone 1.0 C";
static const char AUTHOR[] = "Harrison Ainsworth / HXA7241 : 2013";
static const char URL[]    = "http://www.hxa.name/minilight";

static const char DESCRIPTION[] =
"MiniLightTone tone-maps a MiniLight RGBE render into a PPM image.\n";
static const char USAGE[] =
"Usage:\n"
"  minilighttone [-d displayLuminanceMax] imageFilePathName.rgbe\n"
"-d : display luminance max [int pos] (optional)\n";

static const char DETAILS[] =
"The input RGBE file must be non-RLE. The output PPM file is a P6 24-bit.\n"
"The tone-mapping is just the simple Ward linear one from:\n"
"  'A Contrast Based Scalefactor For Luminance Display'; Ward; 1994.\n";


/* templates */
static const char BANNER_MESSAGE[] = "\n  %s - %s\n\n";
static const char HELP_MESSAGE[]   =
   "\n  %s\n  %s\n  %s\n\n%s\n%s\n%s\n";




/* ctrl-c handling ---------------------------------------------------------- */

void sigintHandler
(
   const int arg
)
{
   /* exit if called (asynchronously) by an 'interactive-attention' */
   if( SIGINT == arg )
   {
      printf( "\ninterrupted\n" );
      exit( EXIT_SUCCESS );
   }
}




/* the actual application --------------------------------------------------- */

void readMapWrite
(
   const char*  sInFilePathName,
   const int32u displayLuminanceMax
)
{
   /* read RGBE file into float image */
   Image* pImage = 0;
   {
      Exception e = ImageConstruct( sInFilePathName, &pImage );
      error( e, sInFilePathName );
   }

   /* tone-map image */
   ImageTonemap( pImage, displayLuminanceMax );

   /* write image to PPM file */
   {
      Exception e;

      /* make output filepathname */
      char* sOutFilePathName = (char*)calloc( strlen(sInFilePathName) + 5,
         sizeof(char) );
      if( !sOutFilePathName ) error( ERROR_ALLOC, 0 );
      strcpy( sOutFilePathName, sInFilePathName );
      strcat( sOutFilePathName, ".ppm" );

      /* write */
      e = ImageWrite( pImage, sOutFilePathName );
      error( e, sOutFilePathName );

      printf( "output: %s\n", sOutFilePathName );
   }

   ImageDestruct( pImage );
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
      const char* sInFilePathName     = "";
      int32u      displayLuminanceMax = 0;

      if( !strcmp( argv[1], "-d" ) )
      {
         if( argc >= 4 )
         {
            const int i = atoi( argv[2] );
            displayLuminanceMax = i > 0 ? (int32u)i : 0;
            sInFilePathName     = argv[3];
         }
      }
      else
      {
         sInFilePathName = argv[1];
      }

      readMapWrite( sInFilePathName, displayLuminanceMax );
   }

   return EXIT_SUCCESS;
}
