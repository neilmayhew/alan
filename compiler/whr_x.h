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

extern WhrNod *newwhr(Srcp *srcp,
		      WhrKind kind,
		      WhtNod *wht);
extern void symbolizeWhr(WhrNod *whr);
extern void verifyAtLocation(WhrNod *whr);
extern void anwhr(WhrNod *whr, Context *context);
extern Aword generateInitialLocation(WhrNod *whr);
extern void gewhr(WhrNod *whr);
extern void duwhr(WhrNod *whr);

#endif
