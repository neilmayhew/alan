#ifndef MEMORY_H_
#define MEMORY_H_
/*----------------------------------------------------------------------*\

    The Amachine memory

\*----------------------------------------------------------------------*/

/* IMPORTS */
#include "sysdep.h"
#include "acode.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/* CONSTANTS */


/* TYPES */


/* DATA */
extern Aword *memory;
extern ACodeHeader *header;
extern int memTop;


/* FUNCTIONS */
extern void *allocate(unsigned long lengthInBytes);
extern void *duplicate(void *original, unsigned long len);

#endif /* MEMORY_H_ */
