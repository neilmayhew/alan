#ifndef _TYPE_X_H_
#define _TYPE_X_H_
/*----------------------------------------------------------------------*\

				TYPE_X.H
			     Type handling

\*----------------------------------------------------------------------*/

#include "type.h"
#include "types.h"

extern Bool isComplexType(TypeKind type);
extern char *typeToString(TypeKind type);
extern void dumpType(TypeKind type);

#endif
