#ifndef _PROP_X_H_
#define _PROP_X_H_
/*----------------------------------------------------------------------*\

			       PROP_X.H
			Property Nodes Export
                                                                     
\*----------------------------------------------------------------------*/

#include "prop.h"

/* USE: */
#include "../interpreter/acode.h"
#include "ins.h"
#include "context.h"


/* DATA: */

/* METHODS: */

extern Properties *newEmptyProps(void);

extern Properties *newProps(List *nams,
			    Where *whr,
			    List *atrs,
			    Container *cnt,
			    Srcp descriptionCheckSrcp,
			    List *descriptionCheck,
			    Srcp descriptionSrcp,
			    List *description,
			    Srcp enteredSrcp, List *entered,
			    Srcp mentionedSrcp, List *mentioned, Srcp definiteSrcp,
			    List *definite,
			    Bool definiteIsForm, Srcp indefiniteSrcp,
			    List *indefinite,
			    Bool indefiniteIsForm,
			    List *exts,
			    List *vrbs,
			    List *scrs);

extern void symbolizeProps(Properties *props);
extern void analyzeProps(Properties *props, Context *context);
extern void generateCommonPropertiesData(Properties *props);
extern void generateInstancePropertiesData(Properties *props);
extern void generatePropertiesEntry(InstanceEntry *entry, Properties *props);
extern void dumpProps(Properties *props);


#endif
