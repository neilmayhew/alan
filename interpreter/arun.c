/*----------------------------------------------------------------------*\

  ARUN.C

  Main program for interpreter for ALAN Adventure Language
  Simple terminal style I/O only

\*----------------------------------------------------------------------*/

#include "main.h"
#include "term.h"
#include "version.h"
#include "args.h"


/*======================================================================

  main()

  Main program of main unit in Alan interpreter module, ARUN

  */
#ifdef _PROTOTYPES_
int main(
     unsigned argc,
     char *argv[]
)
#else
int main(argc, argv)
     unsigned argc;
     char *argv[];
#endif
{
#ifdef MALLOC
  malloc_debug(2);
#endif

  /* Set up page format in case we get a system error */
  lin = col = 1;
  header->paglen = 24;
  header->pagwidth = 70;
  getPageSize();

  args(argc, argv);

  if (dbgflg||verbose) {
    if (dbgflg) printf("<");
    printf("Arun, Adventure Interpreter version %s (%s %s) %s",
	   product.version.string, product.date, product.time, COMPILER);
    if (dbgflg) printf(">");
    newline();
  }
  
  if (strcmp(advnam, "") == 0) {
    printf("No Adventure name given.\n");
    terminate(0);
  }

  run();
}

