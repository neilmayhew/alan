#ifndef _VERB_H_
#define _VERB_H_
/*----------------------------------------------------------------------*\

				Verb.h

			      Verb Nodes
                                                                     
\*----------------------------------------------------------------------*/


/* USE: */
#include "Srcp.h"
#include "List.h"
#include "Syntax.h"


/* Types: */

typedef struct Verb {		/* VERB */
  Srcp srcp;			/* Source position */
  List *verbs;			/* List of verbs to perform this action */
  List *alternatives;		/* Alternative bodies */
  Aaddr altadr;			/* AN - Address to alt-table */
  Syntax *syntax;		/* AN - Which syntax defines this verb */
} Verb;



/* Data: */


/* Methods: */

#ifdef _PROTOTYPES_

/* Create a new Verb node */
extern Verb *newVerb(Srcp *srcp,
		     List *verbs,
		     List *alternatives);

/* Analyze a list of Verbs */
extern void analyseVerbs(List *verbs);

/* Generate code for a list of Verbs */
extern Aaddr generateVerbs(List *verbs);

/* Dump a Verb */
extern void dumpVerb(Verb *verb);

#else
extern Verb *newVerb();
extern void analyseVerbs();
extern Aaddr generateVerbs();
extern void dumpVerb();
#endif

#endif
