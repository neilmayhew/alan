#ifndef _DOES_H_
#define _DOES_H_
/*----------------------------------------------------------------------*\

				Does.h
			      Does Nodes
                                                                     
\*----------------------------------------------------------------------*/


/* USE: */
#include "List.h"

#include "Srcp.h"
#include "Id.h"


/* Types: */

typedef enum DoesKind {
  DOES_BEFORE,
  DOES_AFTER,
  DOES_ONLY,
  DOES_DEFAULT
} DoesKind;

typedef struct Does {		/* DOES nodes */
  Srcp srcp;			/* Source position */
  DoesKind kind;		/* Does kind */
  List *statements;		/* The statements */
} Does;


/* Data: */


/* Functions: */
#ifdef _PROTOTYPES_

/* Create a new Does node */
extern Does *newDoes(Srcp *srcp,
		     DoesKind kind,
		     List *statements);

/* Analyse a Does node */
extern void analyseDoes(Does *does, List *parameters);

/* Generate a reference to a Does */
extern void generateDoes(Does *does, List *parameters);

/* Dump a Does node */
extern void dumpDoes(Does *does);

#else
extern Does *newDoes();
extern void generateDoes();
extern void dumpDoes();
#endif

#endif
