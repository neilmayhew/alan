/*----------------------------------------------------------------------*\

  rules.c

  Rule handling unit of Alan interpreter module, ARUN.

\*----------------------------------------------------------------------*/

#include <stdio.h>

#include "types.h"
#include "main.h"
#include "inter.h"
#include "debug.h"
#include "exe.h"
#include "stack.h"

#include "rules.h"

#ifdef GLK
#include "glkio.h"
#endif

#ifdef _PROTOTYPES_
void rules(void)
#else
void rules()
#endif
{
  Boolean change = TRUE;
  int i;
  
  for (i = 1; !endOfTable(&ruls[i-1]); i++)
    ruls[i-1].run = FALSE;
  
  while (change) {
    change = FALSE;
    for (i = 1; !endOfTable(&ruls[i-1]); i++) 
      if (!ruls[i-1].run) {
	if (traceOption) {
	  printf("\n<RULE %d (at ", i);
	  debugsay(current.location);
	  if (!singleStepOption)
	    printf("), Evaluating");
	  else
	    printf("), Evaluating:>\n");
	}
	interpret(ruls[i-1].exp);
	if (pop()) {
	  change = TRUE;
	  ruls[i-1].run = TRUE;
	  if (traceOption) {
	    if (!singleStepOption)
	      printf(", Executing:>\n");
	    else {
	      printf("\nRULE %d (at ", i);
	      debugsay(current.location);
	      printf("), Executing:>\n");
	    }
	  }
	  interpret(ruls[i-1].stms);
	} else if (traceOption && !singleStepOption)
	  printf(":>\n");
      }
  }
}
