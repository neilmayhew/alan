#ifndef _WRD_X_H_
#define _WRD_X_H_
/*----------------------------------------------------------------------*\

				WRD_X.H
		     Dictionary Word Nodes Export

\*----------------------------------------------------------------------*/

#include "wrd.h"


/* USE: */
#include "../interpreter/acode.h"
#include "ins.h"


/* DATA: */

extern int words[];



/* FUNCTIONS: */

/* Find a Word in the dictonary */
extern WordNode *findwrd(char str[]);

/* Insert a Word into the dictionary */
extern int newWord(char wrd[],
		  WrdKind class,
		  int code,
		  Instance *ref);

/* Prepare Words in dictionary */
extern void prepwrds(void);

/* Analyse the Words in dictionary */
extern void anwrds(void);

/* Generate all Words in the dictionary */
extern Aaddr gewrds(void);


#endif
