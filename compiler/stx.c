/*----------------------------------------------------------------------*\

                               STX.C
                            Syntax Nodes

\*----------------------------------------------------------------------*/

#include "alan.h"
#include "util.h"

#include "srcp.h"
#include "lmList.h"

#include "adv.h"                /* ADV-nodes */
#include "sym.h"                /* SYM-nodes */
#include "lst.h"                /* LST-nodes */
#include "res.h"                /* RES-nodes */
#include "stx.h"                /* STX-nodes */
#include "elm.h"                /* ELM-nodes */
#include "wrd.h"                /* WRD-nodes */

#include "emit.h"

#include "acode.h"

#include "dump.h"


/* PUBLIC: */


/*======================================================================

  newstx()

  Allocates and initialises a syntax node.

 */
StxNod *newstx(Srcp *srcp,      /* IN - Source Position */
               IdNod *id,     /* IN - Name of the verb it defines */
               List *elms,      /* IN - List of elements */
               List *ress)      /* IN - List of class restrictions */
{
  StxNod *new;                  /* The newly created node */

  if (verbose) { printf("%8ld\b\b\b\b\b\b\b\b", counter++); fflush(stdout); }

  new = NEW(StxNod);

  new->srcp = *srcp;
  new->id = id;
  new->elms = elms;
  new->ress = ress;

  new->generated = FALSE;

  return(new);
}



/*----------------------------------------------------------------------

  anstx()

  Analyzes one syntax node.

 */
static void anstx(StxNod *stx)  /* IN - Syntax node to analyze */
{
  SymNod *sym;

  if (verbose) { printf("%8ld\b\b\b\b\b\b\b\b", counter++); fflush(stdout); }

  /* Find which verb it defines */
  sym = lookup(stx->id->string);  /* Find earlier definition */
  if (sym == NULL) {
    if (stx->id->string[0] != '$') /* generated id? */
      lmLog(&stx->id->srcp, 207, sevWAR, stx->id->string);
  } else if (sym->kind != VERB_SYMBOL)
    lmLog(&stx->id->srcp, 208, sevWAR, stx->id->string);
  else
    stx->id->symbol = sym;

  stx->pars = anelms(stx->elms, stx->ress, stx);
  anress(stx->ress, stx->pars);

  /* Link the last syntax element to this stx to prepare for code generation */
  stx->elms->tail->element.elm->stx = stx;
}



/*======================================================================

  anstxs()

  Analyzes all syntax nodes in the adventure by calling the analyzer for
  each one.

 */
void anstxs(void)
{
  List *lst, *other;

  for (lst = adv.stxs; lst != NULL; lst = lst->next)
    anstx(lst->element.stx);

  /* Check for multiple definitions of the syntax for a verb */
  for (lst = adv.stxs; lst != NULL; lst = lst->next)
    for (other = lst->next; other != NULL; other = other->next) {
      if (other->element.stx->id->symbol->code != -1 || lst->element.stx->id->symbol->code != -1)
	if (other->element.stx->id->symbol->code == lst->element.stx->id->symbol->code) {
	  if (!lst->element.stx->muldef){
	    lmLog(&lst->element.stx->id->srcp, 206, sevWAR,
		  lst->element.stx->id->string);
	    lst->element.stx->muldef = TRUE;
	  }
	  if (!other->element.stx->muldef){
	    lmLog(&other->element.stx->id->srcp, 206, sevWAR,
		  other->element.stx->id->string);
	    other->element.stx->muldef = TRUE;
	  }
	  break;
	}
    }
}


/*======================================================================

  defaultStx()

  Returns the address a defaut syntax node which is used for verbs
  without any defined syntax:

  Syntax x = x (object).

 */
StxNod *defaultStx(char *vrbstr) /* IN - The string for the verb */
{
  StxNod *stx;
  List *elms;

  elms = concat(concat(concat(NULL,
			      newelm(&nulsrcp, ELMWRD, newid(&nulsrcp,
							     vrbstr),
				     FALSE),
			      LIST_ELM),
		       newelm(&nulsrcp, ELMPAR, newid(&nulsrcp, "object"), FALSE),
		       LIST_ELM),
		newelm(&nulsrcp, ELMEOS, NULL, FALSE), LIST_ELM);
  stx = newstx(&nulsrcp, newid(&nulsrcp, vrbstr), elms, NULL);

  adv.stxs = concat(adv.stxs, stx, LIST_STX);
  anstx(stx);                   /* Make sure the syntax is analysed */
  return stx;
}



/*======================================================================

  eqparams()

  Compare two syntax nodes and return true if their parameter lists are
  compatible (same ordering with same parameter names).

  */
Bool eqparams(StxNod *stx1,     /* IN - Syntax node to compare */
              StxNod *stx2)     /* IN - Syntax node to compare */
{
  List *elm1, *elm2;

  for (elm1 = stx1->pars, elm2 = stx2->pars;
       elm1 != NULL && elm2 != NULL;
       elm1 = elm1->next, elm2 = elm2->next) {
    if (!eqids(elm1->element.elm->id, elm2->element.elm->id))
      return FALSE;
    if (elm1->element.elm->flags != elm2->element.elm->flags)
      return FALSE;
  }
  return elm1 == elm2;          /* Both NULL => equal */
}


/*----------------------------------------------------------------------

  gestx()

  Generate one syntax node.

 */
static void gestx(StxNod *stx)  /* IN - Syntax node to generate for */
{
  WrdNod *wrd;
  List *lst = NULL;
  List *elms = NULL;            /* A list of parallell elms-lists */


  if (verbose) { printf("%8ld\b\b\b\b\b\b\b\b", counter++); fflush(stdout); }
  
  if (!stx->generated) {
    /* First word is a verb which points to all stxs starting with that word */
    wrd = findwrd(stx->elms->element.elm->id->string);
    /* Ignore words that are not verbs and prepositions */
    if (wrd->classbits&(1L<<WRD_PREP))
      lst = wrd->ref[WRD_PREP];
    if (wrd->classbits&(1L<<WRD_VRB))
      lst = wrd->ref[WRD_VRB];
    /* Create a list of all parallell elements */
    while (lst) {
      elms = concat(elms, lst->element.stx->elms, LIST_LST);
      lst->element.stx->generated = TRUE;
      lst = lst->next;
    }
    stx->elmsadr = geelms(elms, stx);
  } else
    stx->elmsadr = 0;
}



/*----------------------------------------------------------------------

  gestxent()

  Generate a table entry for one syntax node.

 */
static void gestxent(StxNod *stx) /* IN - Syntax node to generate for */
{
  if (stx->elmsadr != 0) {
    /* The code for the verb word */
    emit(stx->elms->element.elm->id->symbol->code);
    /* Address to syntax element tables */
    emit(stx->elmsadr);
  }
}



/*======================================================================

  gestxs()

  Generate the data structure for all syntax definitions in the \
  adventure.

 */
Aaddr gestxs(void)
{
  List *lst;
  Aaddr stxadr;

  /* First generate all class restriction checks */
  for (lst = adv.stxs; lst != NULL; lst = lst->next)
    lst->element.stx->resadr = geress(lst->element.stx->ress, lst->element.stx);

  /* Then the actual stxs */
  for (lst = adv.stxs; lst != NULL; lst = lst->next)
    gestx(lst->element.stx);

  /* Then a table of entries */
  stxadr = emadr();
  for (lst = adv.stxs; lst != NULL; lst = lst->next)
    gestxent(lst->element.stx);
  emit(EOF);
  return(stxadr);
}



/*======================================================================

  dustx()

  Dump an Syntax node.

 */
void dustx(StxNod *stx)
{
  if (stx == NULL) {
    put("NULL");
    return;
  }

  put("STX: "); duptr(stx); dusrcp(&stx->srcp); in();
  put("id: "); dumpId(stx->id); nl();
  put("generated: "); dumpBool(stx->generated); nl();
  put("elmsadr: "); duadr(stx->elmsadr); nl();
  put("elms: "); dulst(stx->elms, LIST_ELM); nl();
  put("resadr: "); duadr(stx->resadr); nl();
  put("ress: "); dulst(stx->ress, LIST_RES); nl();
  put("pars: "); dulst(stx->pars, LIST_ELM); out();
}
