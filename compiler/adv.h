#ifndef _ADV_H_
#define _ADV_H_

/* USE other definitions */
#include "lst.h"
#include "whr.h"

/* Types: */

typedef struct Adventure {
  List *syns;			/* List of synonyms */
  List *stxs;			/* List of syntax definitions */
  List *vrbs;			/* List of global verbs */
  List *clas;			/* List of defined classes */
  List *adds;			/* List of additions to the classes */
  List *inss;			/* List of defined instances */
  List *evts;			/* List of events */
  List *cnts;			/* List of containers */
  List *ruls;			/* List of rules */
  List *stratrs;		/* List of string attributes to initialize */
  Where *whr;			/* Where to start */
  List *stms;			/* List of start statements */
  int *scores;			/* Pointer to array of scores */
  List *msgs;			/* List of error messages */
} Adventure;

#endif
