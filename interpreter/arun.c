/*----------------------------------------------------------------------*\

  ARUN.C

  Main program for interpreter for ALAN Adventure Language
  Simple terminal style I/O only

\*----------------------------------------------------------------------*/

#include "main.h"
#include "term.h"
#ifdef HAVE_SHORT_FILENAMES
#include "av.h"
#else
#include "alan.version.h"
#endif
#include "args.h"

#ifdef GLK
#include "glkio.h"
#include "WinGlk.h"
#endif

/*======================================================================

  main()

  Main program of main unit in Alan interpreter module, ARUN

  */

#ifdef GLK
#ifdef _PROTOTYPES_
void glk_main(void)
#else
void glk_main()
#endif
#else
#ifdef _PROTOTYPES_
int main(
     int argc,
     char *argv[]
)
#else
int main(argc, argv)
     int argc;
     char *argv[];
#endif
#endif
{
#ifdef MALLOC
  malloc_debug(2);
#endif

  /* Set up page format in case we get a system error */
  lin = col = 1;
  header->pageLength = 24;
  header->pageWidth = 70;

  getPageSize();

#ifdef GLK
  /* args() is called from glkstart.c */
#else
  args(argc, argv);
#endif

  if ((debugOption && ! regressionTestOption) || verbose) {
    if (debugOption) printf("<");
    printf("Arun, Adventure Interpreter version %s (%s %s)",
	   alan.version.string, alan.date, alan.time);
    if (debugOption) printf(">");
    newline();
  }
  
  if (strcmp(advnam, "") == 0) {
#ifdef WINGLK
    char *filename;
    filename = (char*)winglk_get_initial_filename(NULL, "Arun : Select an Alan game file",
		"Alan Game Files (*.a3c)|*.a3c||");
    if (filename) {
      advnam = strdup(filename);
      advnam[strlen(advnam)-4] = 0; /* Strip of .A3C */
    } else {
      printf("You should supply a game file to play.\n");
      usage();
      terminate(0);
    }
#else
    printf("You should supply a game file to play.\n");
    usage();
    terminate(0);
#endif
  }

#ifdef WINGLK
  winglk_window_set_title(advnam);
#endif

  run();

#ifdef GLK
  return;
#else
  return(EXIT_SUCCESS);
#endif
}

