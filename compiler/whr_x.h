#ifndef _WHR_X_H_
#define _WHR_X_H_
/*----------------------------------------------------------------------*\

				WHR.H
			  Where Nodes Export
                                                                     
\*----------------------------------------------------------------------*/

#include "whr.h"

/* USE: */
#include "context.h"


/* DATA: */


/* FUNCTIONS: */

extern Where *newwhr(Srcp *srcp,
		      WhrKind kind,
		      What *wht);
extern void symbolizeWhere(Where *whr);
extern void verifyInitialLocation(Where *whr);
extern Symbol *classOfContent(Where *where);
extern void analyzeWhere(Where *whr, Context *context);
extern Aword generateInitialLocation(Where *whr);
extern void generateWhere(Where *whr);
extern void duwhr(Where *whr);

#endif
