/*----------------------------------------------------------------------*\

				SLT.C
			      Slot Nodes

\*----------------------------------------------------------------------*/

#include "slt_x.h"


/* IMPORT */
#include <stdio.h>
#include "alan.h"
#include "util.h"
#include "dump.h"
#include "emit.h"
#include "lmList.h"

#include "stm.h"

#include "nam_x.h"
#include "whr_x.h"
#include "cla_x.h"
#include "sym_x.h"
#include "cnt_x.h"
#include "id_x.h"


/*======================================================================

  newEmptySlots()

  Allocates and initialises a empty slots node.

  */
SlotsNode *newEmptySlots(void)
{
  return NEW(SlotsNode);
}



/*======================================================================

  newSlots()

  Allocates and initialises a slots node.

  */
SlotsNode *newSlots(List *names,
		    WhrNod *whr,
		    List *atrs,
		    CntNod *cnt,
		    List *description,
		    List *mentioned,
		    List *art,
		    List *does,
		    List *exits,
		    List *vrbs,
		    List *scrs)
{
  SlotsNode *new;                  /* The newly allocated area */

  if (verbose) { printf("%8ld\b\b\b\b\b\b\b\b", counter++); fflush(stdout); }

  new = NEW(SlotsNode);

  new->names = names;
  new->whr = whr;
  new->atrs = atrs;
  new->cnt = cnt;
  new->description = description;
  new->mentioned = mentioned;
  new->art = art;
  new->vrbs = vrbs;
  new->exits = exits;

  return(new);
}



/*----------------------------------------------------------------------

  symbolizeParent()

  Symbolize parent of a Slots node.

 */
static void symbolizeParent(SlotsNode *slots)
{
  SymNod *parent;

  if (slots->parent != NULL) {
    parent = lookup(slots->parent->string);
    if (parent == NULL)
      lmLog(&slots->parent->srcp, 310, sevERR, slots->parent->string);
    else if (parent->kind != CLASS_SYMBOL)
      lmLog(&slots->parent->srcp, 350, sevERR, "");
    else {
      slots->parent->symbol = parent;
      setParent(slots->symbol, slots->parent->symbol);
    }
  }
}


/*======================================================================

  symbolizeSlots()

  Symbolize data for one Slots node.

 */
void symbolizeSlots(SlotsNode *slots)
{
  symbolizeParent(slots);
  symbolizeWhr(slots->whr);
  symbolizeExits(slots->exits);
}


/*----------------------------------------------------------------------

  analyzeMentioned()

*/

void analyzeMentioned(SlotsNode *slots)
{
  long fpos;
  int len = 0;
  StmNod *stm;

  if (slots->mentioned == NULL) {
    /* Generate a mentioned from the first of the names */
    /* First output the formated name to the text file */
    fpos = ftell(txtfil);
    len = annams(slots->names, slots->id,
		 inheritsFrom(slots->symbol, location->symbol));

    /* Then create a PRINT statement */
    stm = newstm(&nulsrcp, STM_PRINT);
    stm->fields.print.fpos = fpos;
    stm->fields.print.len = len;
    slots->mentioned = concat(NULL, stm, STM_LIST);
  } else
    anstms(slots->mentioned, NULL, NULL, NULL);
}

/*======================================================================

  analyzeSlots()

  Analyze one Slots node.

 */
void analyzeSlots(SlotsNode *slots)
{
  if (slots->whr != NULL) verifyAtLocation(slots->whr);
  analyzeMentioned(slots);
  analyzeExits(slots->exits);
}


/*======================================================================

  generateSlotsData()

  Generate data for one Slots node.

 */
void generateSlotsData(SlotsNode *slots)
{
  slots->idAddress = emadr();
  emitstr(slots->id->string);

  if (slots->description != NULL) {
    slots->descriptionAddress = emadr();
    gestms(slots->description, NULL);
    emit0(C_STMOP, I_RETURN);
  }

  slots->mentionedAddress = emadr();
  gestms(slots->mentioned, NULL);
  emit0(C_STMOP, I_RETURN);

}


/*======================================================================

  generateSlotsEntry()

  Generate entries for one Slots node.

 */
void generateSlotsEntry(InstanceEntry *entry, SlotsNode *slots)
{
  entry->code = slots->symbol->code; /* First own code */
  entry->idAddress = slots->idAddress; /* Address to the id string */

  if (slots->parent == NULL)	/* Then parents */
    entry->parent = 0;
  else
    entry->parent = slots->parent->symbol->code;

  entry->location = generateInitialLocation(slots->whr);
  entry->attributes = slots->atradr;
  entry->description = slots->descriptionAddress;
  entry->mentioned = slots->mentionedAddress;
  entry->article = slots->artadr;
  entry->exits = slots->exitsAddress;
  entry->verbs = slots->vrbadr;
}


/*======================================================================

  dumpSlots()

  Dump a Slots node.

 */
void dumpSlots(SlotsNode *slots)
{
  put("SLOTS: "); in();
  put("id: "); dumpId(slots->id); nl();
  put("names: "); dulst(slots->names, LIST_NAM); nl();
  put("whr: "); duwhr(slots->whr); nl();
#ifdef FIXME
  put("cnt: "); ducnt(slots->cnt); nl();
#endif
  put("atrs: "); dulst(slots->atrs, LIST_ATR); nl();
  put("atradr: "); duadr(slots->atradr); nl();
  put("description: "); dulst(slots->description, LIST_STM); nl();
  put("descriptionAddress: "); duadr(slots->descriptionAddress); nl();
  put("art: "); dulst(slots->art, LIST_STM); nl();
  put("artadr: "); duadr(slots->artadr); nl();
  put("mentioned: "); dulst(slots->mentioned, LIST_STM); nl();
  put("mentionedAddress: "); duadr(slots->mentionedAddress); nl();
  put("vrbs: "); dulst(slots->vrbs, LIST_VRB); nl();
  put("vrbadr: "); duadr(slots->vrbadr); nl();
  put("exits: "); dulst(slots->exits, LIST_EXT); nl();
  put("exitsAddress: "); duadr(slots->exitsAddress); out();
}
