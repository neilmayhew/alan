/*----------------------------------------------------------------------*\

  ARUN.C

  Main module of interpreter for ALAN Adventure Language

\*----------------------------------------------------------------------*/

#include "sysdep.h"

#include "types.h"
#include "arun.h"

#include <time.h>
#ifdef USE_READLINE
#include "readline.h"
#endif

#include "version.h"

#include "args.h"
#include "parse.h"
#include "inter.h"
#include "rules.h"
#ifdef REVERSED
#include "reverse.h"
#endif
#include "debug.h"
#include "stack.h"
#include "exe.h"
#include "term.h"


/* PUBLIC DATA */

/* The Amachine memory */
Aword *memory;
static AcdHdr dummyHeader;	/* Dummy to use until memory allocated */
AcdHdr *header = &dummyHeader;

int memTop = 0;			/* Top of load memory */


/* Amachine variables */
CurVars cur;

/* Amachine structures */
WrdElem *dict;			/* Dictionary pointer */
ActElem *acts;			/* Actor table pointer */
LocElem *locs;			/* Location table pointer */
VrbElem *vrbs;			/* Verb table pointer */
StxElem *stxs;			/* Syntax table pointer */
ObjElem *objs;			/* Object table pointer */
CntElem *cnts;			/* Container table pointer */
RulElem *ruls;			/* Rule table pointer */
EvtElem *evts;			/* Event table pointer */
MsgElem *msgs;			/* Message table pointer */
Aword *scores;			/* Score table pointer */
Aword *freq;			/* Cumulative character frequencies */

int dictsize;

Boolean errflg = TRUE;
Boolean trcflg = FALSE;
Boolean dbgflg = FALSE;
Boolean stpflg = FALSE;
Boolean logflg = FALSE;
Boolean fail = FALSE;
Boolean anyOutput = FALSE;


/* The files and filenames */
char *advnam;
FILE *txtfil;
FILE *logfil;


/* Screen formatting info */
int col, lin;
int paglen, pagwidth;

Boolean needsp = FALSE;
Boolean skipsp = FALSE;

/* Restart jump buffer */
jmp_buf restart;


/* PRIVATE DATA */
static jmp_buf jmpbuf;		/* Error return long jump buffer */



/*======================================================================

  terminate()

  Terminate the execution of the adventure, e.g. close windows,
  return buffers...

 */
#ifdef _PROTOTYPES_
void terminate(int code)
#else
void terminate(code)
     int code;
#endif
{
#ifdef __amiga__
#include <fcntl.h>
  extern struct _dev *_devtab;
  char buf[85];
  
  if (con) { /* Running from WB, created a console so kill it */
    /* Running from WB, so we created a console and
       hacked the Aztec C device table to use it for all I/O
       so now we need to make it close it (once!) */
    _devtab[1].fd = _devtab[2].fd = 0;
  }
#endif
  free(memory);
  exit(code);
}


/*======================================================================

  syserr()

  Print a little text blaming the user for the system error.

 */
#ifdef _PROTOTYPES_
void syserr(char *str)
#else
void syserr(str)
     char *str;
#endif
{
  output("$n$nAs you enter the twilight zone of Adventures, you stumble \
and fall to your knees. In front of you, you can vaguely see the outlines \
of an Adventure that never was.$n$nSYSTEM ERROR: ");
  output(str);
  output("$n$n");

  if (logflg)
    fclose(logfil);
  newline();

#ifdef __amiga__
  {
    char buf[80];

    if (con) { /* Running from WB, wait for user ack. */
      printf("press RETURN to quit");
      gets(buf);
    }
  }
#endif
  terminate(0);
}


/*======================================================================

  error()

  Print an error message, force new player input and abort.

  */
#ifdef _PROTOTYPES_
void error(MsgKind msgno)	/* IN - The error message number */
#else
void error(msgno)
     MsgKind msgno;		/* IN - The error message number */
#endif
{
  if (msgno != MSGMAX)
    prmsg(msgno);
  wrds[wrdidx] = EOF;		/* Force new player input */
  dscrstkp = 0;			/* Reset describe stack */
  longjmp(jmpbuf,TRUE);
}



/*======================================================================

  newline()

  Make a newline, but check for screen full.

 */
#ifdef _PROTOTYPES_
void newline(void)
#else
void newline()
#endif
{
  char buf[256];
  
  col = 1;
  if (lin >= paglen - 1) {
    printf("\n");
    needsp = FALSE;
    prmsg(M_MORE);
#ifdef USE_READLINE
    (void) readline(buf);
#else
    fgets(buf, 256, stdin);
#endif
    getPageSize();
    lin = 0;
  } else
    printf("\n");
  
  lin++;
  needsp = FALSE;
}


/*======================================================================

  para()

  Make a new paragraph, i.e one empty line (one or two newlines).

 */
#ifdef _PROTOTYPES_
void para(void)
#else
void para()
#endif
{
  if (col != 1)
    newline();
  newline();
}


/*======================================================================

  allocate()

  Safely allocate new memory.

*/
#ifdef _PROTOTYPES_
void *allocate(int len)		/* IN - Length to allocate */
#else
void *allocate(len)
     int len;			/* IN - Length to allocate */
#endif
{
  void *p = (void *)malloc((size_t)len);

  if (p == NULL)
    syserr("Out of memory.");

  return p;
}


/*----------------------------------------------------------------------

  just()

  Justify a string so that it wraps at end of screen.

 */
#ifdef _PROTOTYPES_
static void just(char str[])
#else
static void just(str)
     char str[];
#endif
{
  int i;
  char ch;
  
  if (col >= pagwidth && !skipsp)
    newline();

  while (strlen(str) > pagwidth - col) {
    i = pagwidth - col - 1;
    while (!isSpace(str[i]) && i > 0) /* First find wrap point */
      i--;
    if (i == 0 && col == 1)	/* If it doesn't fit at all */
      /* Wrap immediately after this word */
      while (!isSpace(str[i]) && str[i] != '\0')
	i++;
    if (i > 0) {		/* If it fits ... */
      ch = str[i];		/* Save space or NULL */
      str[i] = '\0';		/* Terminate string */
      printf(str);		/* and print it */
      skipsp = FALSE;		/* If skipping, now we're done */
      str[i] = ch;		/* Restore character */
      /* Skip white after printed portion */
      for (str = &str[i]; isSpace(str[0]) && str[0] != '\0'; str++);
    }
    newline();			/* Then start a new line */
  }
  printf(str);			/* Print tail */
  col = col + strlen(str);	/* Update column */
}


/*----------------------------------------------------------------------

  space()

  Output a space if needed.

 */
#ifdef _PROTOTYPES_
static void space(void)
#else
static void space()
#endif
{
  if (skipsp)
    skipsp = FALSE;
  else {
    if (needsp) {
      printf(" ");
      col++;
    }
  }
  needsp = FALSE;
}



/*----------------------------------------------------------------------

  sayparam()

  A parameter needs to be said, check for words the player used and use
  them if possible.

*/
#ifdef _PROTOTYPES_
static void sayparam(int p)
#else
static void sayparam(p)
     int p;
#endif
{
  int i;

  for (i = 0; i <= p; i++)
    if (params[i].code == EOF)
      syserr("Nonexistent parameter referenced.");

  if (params[p].firstWord == EOF) /* Any words he used? */
    say(params[p].code);
  else				/* Yes, so use them... */
    for (i = params[p].firstWord; i <= params[p].lastWord; i++) {
      just((char *)addrTo(dict[wrds[i]].wrd));
      if (i < params[p].lastWord)
	just(" ");
    }
}


/*----------------------------------------------------------------------

  prsym()

  Print an expanded symbolic reference.

  N = newline
  I = indent on a new line
  P = new paragraph
  L = current location name
  O = current object -> first parameter!
  V = current verb
  A = current actor
  T = tabulation
  $ = no space needed after this
 */
#ifdef _PROTOTYPES_
static void prsym(
		  char *str	/* IN - The string starting with '$' */
)
#else
static void prsym(str)
     char *str;			/* IN - The string starting with '$' */
#endif
{
  switch (toLower(str[1])) {
  case 'n':
    newline();
    break;
  case 'i':
    newline();
    printf("    ");
    col = 5;
    break;
  case 'o':
    sayparam(0);
    break;
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    sayparam(str[1]-'1');
    break;
  case 'l':
    say(cur.loc);
    break;
  case 'a':
    say(cur.act);
    break;
  case 'v':
    just((char *)addrTo(dict[vrbwrd].wrd));
    break;
  case 'p':
    para();
    break;
  case 't': {
    int i;
    int spaces = 4-(col-1)%4;
    
    for (i = 0; i<spaces; i++) printf(" ");
    col = col + spaces;
    needsp = FALSE;
    break;
  }
  case '$':
    skipsp = TRUE;
    break;
  default:
    printf("$");
    break;
  }
}



/*======================================================================

  output()

  Output a string to suit the screen. Any symbolic inserts ('$') are
  recogniced and performed.

 */
#ifdef _PROTOTYPES_
void output(char original[])
#else
void output(original)
     char original[];
#endif
{
  char ch;
  char *str, *copy;
  char *symptr;

  copy = strdup(original);
  str = copy;

  if (str[0] != '$' || str[1] != '$')
    space();			/* Output space if needed (& not inhibited) */

  while ((symptr = strchr(str, '$')) != (char *) NULL) {
    ch = *symptr;		/* Terminate before symbol */
    *symptr = '\0';
    if (strlen(str) > 0) {
      just(str);		/* Output part before '$' */
      if (str[strlen(str)-1] == ' ')
	needsp = FALSE;
    }
    *symptr = ch;		/* restore '$' */
    prsym(symptr);		/* Print the symbolic reference */
    str = &symptr[2];		/* Advance to after symbol and continue */
  }
  if (str[0] != 0) {
    just(str);			/* Output trailing part */
    skipsp = FALSE;
    if (str[strlen(str)-1] != ' ')
      needsp = TRUE;
  }
  anyOutput = TRUE;
  free(copy);
}


/*======================================================================

  prmsg()

  Print a message from the message table.
  
  */
#ifdef _PROTOTYPES_
void prmsg(MsgKind msg)	/* IN - message number */
#else
void prmsg(msg)
     MsgKind msg;		/* IN - message number */
#endif
{
  print(msgs[msg].fpos, msgs[msg].len);
}


/*----------------------------------------------------------------------*\

  Various check functions

  endOfTable()
  isObj, isLoc, isAct, IsCnt & isNum

\*----------------------------------------------------------------------*/

/* How to know we are at end of a table */
#ifdef _PROTOTYPES_
Boolean eot(Aword *adr)
#else
Boolean eot(adr)
     Aword *adr;
#endif
{
  return *adr == EOF;
}


#ifdef _PROTOTYPES_
Boolean isObj(Aword x)
#else
Boolean isObj(x)
     Aword x;
#endif
{
  return x >= OBJMIN && x <= OBJMAX;
}

#ifdef _PROTOTYPES_
Boolean isCnt(Aword x)
#else
Boolean isCnt(x)
     Aword x;
#endif
{
  return (x >= CNTMIN && x <= CNTMAX) ||
    (isObj(x) && objs[x-OBJMIN].cont != 0) ||
    (isAct(x) && acts[x-ACTMIN].cont != 0);
}

#ifdef _PROTOTYPES_
Boolean isAct(Aword x)
#else
Boolean isAct(x)
     Aword x;
#endif
{
  return x >= ACTMIN && x <= ACTMAX;
}

#ifdef _PROTOTYPES_
Boolean isLoc(Aword x)
#else
Boolean isLoc(x)
     Aword x;
#endif
{
  return x >= LOCMIN && x <= LOCMAX;
}

#ifdef _PROTOTYPES_
Boolean isNum(Aword x)
#else
Boolean isNum(x)
     Aword x;
#endif
{
  return x >= LITMIN && x <= LITMAX && litValues[x-LITMIN].type == TYPNUM;
}

#ifdef _PROTOTYPES_
Boolean isStr(Aword x)
#else
Boolean isStr(x)
     Aword x;
#endif
{
  return x >= LITMIN && x <= LITMAX && litValues[x-LITMIN].type == TYPSTR;
}

#ifdef _PROTOTYPES_
Boolean isLit(Aword x)
#else
Boolean isLit(x)
     Aword x;
#endif
{
  return x >= LITMIN && x <= LITMAX;
}



/*======================================================================

  exitto()

  Is there an exit from one location to another?

  */
#ifdef _PROTOTYPES_
Boolean exitto(int to, int from)
#else
Boolean exitto(to, from)
     int to, from;
#endif
{
  ExtElem *ext;

  if (locs[from-LOCMIN].exts == 0)
    return(FALSE); /* No exits */

  for (ext = (ExtElem *) addrTo(locs[from-LOCMIN].exts); !endOfTable(ext); ext++)
    if (ext->next == to)
      return(TRUE);

  return(FALSE);
}
    
      

#ifdef CHECKOBJ
/*======================================================================

  checkobj()

  Check that the object given is valid, else print an error message
  or find out what he wanted.

  This routine is not used any longer, kept for sentimental reasons ;-)

  */
void checkobj(obj)
     Aword *obj;
{
  Aword oldobj;
  
  if (*obj != EOF)
    return;
  
  oldobj = EOF;
  for (cur.obj = OBJMIN; cur.obj <= OBJMAX; cur.obj++) {
    /* If an object is present and it is possible to perform his action */
    if (isHere(cur.obj) && possible())
      if (oldobj == EOF)
	oldobj = cur.obj;
      else
	error(WANT);          /* And we didn't find multiple objects */
    }
  
  if (oldobj == EOF)
    error(WANT);              /* But we found ONE */
  
  *obj = cur.obj = oldobj;    
  output("($o)");             /* Then he surely meant this object */
}
#endif




/*----------------------------------------------------------------------
  count()

  Count the number of items in a container.

  */
#ifdef _PROTOTYPES_
static int count(int cnt)	/* IN - the container to count */
#else
static int count(cnt)
     int cnt;			/* IN - the container to count */
#endif
{
  int i, j = 0;
  
  for (i = OBJMIN; i <= OBJMAX; i++)
    if (in(i, cnt))
      /* Then it's in this container also */
      j++;
  return(j);
}



/*----------------------------------------------------------------------
  sumatr()

  Sum the values of one attribute in a container. Recursively.

  */
#ifdef _PROTOTYPES_
static int sumatr(
     Aword atr,			/* IN - the attribute to sum over */
     Aword cnt			/* IN - the container to sum */
)
#else
static int sumatr(atr, cnt)
     Aword atr;			/* IN - the attribute to sum over */
     Aword cnt;			/* IN - the container to sum */
#endif
{
  int i;
  int sum = 0;

  for (i = OBJMIN; i <= OBJMAX; i++)
    if (objs[i-OBJMIN].loc == cnt) {	/* Then it's in this container */
      if (objs[i-OBJMIN].cont != 0)	/* This is also a container! */
	sum = sum + sumatr(atr, i);
      sum = sum + attribute(i, atr);
    }
  return(sum);
}




/*======================================================================
  checklim()

  Checks if a limit for a container is exceeded.

  */
#ifdef _PROTOTYPES_
Boolean checklim(
     Aword cnt,			/* IN - Container code */
     Aword obj			/* IN - The object to add */
)
#else
Boolean checklim(cnt, obj)
     Aword cnt;			/* IN - Container code */
     Aword obj;			/* IN - The object to add */
#endif
{
  LimElem *lim;
  Aword props;

  fail = TRUE;
  if (!isCnt(cnt))
    syserr("Checking limits for a non-container.");

  /* Find the container properties */
  if (isObj(cnt))
      props = objs[cnt-OBJMIN].cont;
  else if (isAct(cnt))
      props = acts[cnt-ACTMIN].cont;
  else
    props = cnt;

  if (cnts[props-CNTMIN].lims != 0) /* Any limits at all? */
    for (lim = (LimElem *) addrTo(cnts[props-CNTMIN].lims); !endOfTable(lim); lim++)
      if (lim->atr == 0) {
	if (count(cnt) >= lim->val) {
	  interpret(lim->stms);
	  return(TRUE);		/* Limit check failed */
	}
      } else {
	if (sumatr(lim->atr, cnt) + attribute(obj, lim->atr) > lim->val) {
	  interpret(lim->stms);
	  return(TRUE);
	}
      }
  fail = FALSE;
  return(FALSE);
}





/*----------------------------------------------------------------------*\

  Action routines

\*----------------------------------------------------------------------*/



/*----------------------------------------------------------------------
  trycheck()

  Tries a check, returns TRUE if it passed, FALSE else.

  */
#ifdef _PROTOTYPES_
static Boolean trycheck(
     Aaddr adr,			/* IN - ACODE address to check table */
     Boolean act		/* IN - Act if it fails ? */
)
#else
static Boolean trycheck(adr, act)
     Aaddr adr;			/* IN - ACODE address to check table */
     Boolean act;		/* IN - Act if it fails ? */
#endif
{
  ChkElem *chk;

  chk = (ChkElem *) addrTo(adr);
  if (chk->exp == 0) {
    interpret(chk->stms);
    return(FALSE);
  } else {
    while (!endOfTable(chk)) {
      interpret(chk->exp);
      if (!(Abool)pop()) {
	if (act)
	  interpret(chk->stms);
	return(FALSE);
      }
      chk++;
    }
    return(TRUE);
  }
}


/*======================================================================
  go()

  Move hero in a direction.

  */
#ifdef _PROTOTYPES_
void go(int dir)
#else
void go(dir)
     int dir;
#endif
{
  ExtElem *ext;
  Boolean ok;
  Aword oldloc;

  ext = (ExtElem *) addrTo(locs[cur.loc-LOCMIN].exts);
  if (locs[cur.loc-LOCMIN].exts != 0)
    while (!endOfTable(ext)) {
      if (ext->code == dir) {
	ok = TRUE;
	if (ext->checks != 0) {
	  if (trcflg) {
	    printf("\n<EXIT %d (%s) from %d (", dir,
		   (char *)addrTo(dict[wrds[wrdidx-1]].wrd), cur.loc);
	    debugsay(cur.loc);
	    printf("), Checking:>\n");
	  }
	  ok = trycheck(ext->checks, TRUE);
	}
	if (ok) {
	  oldloc = cur.loc;
	  if (ext->action != 0) {
	    if (trcflg) {
	      printf("\n<EXIT %d (%s) from %d (", dir, 
		     (char *)addrTo(dict[wrds[wrdidx-1]].wrd), cur.loc);
	      debugsay(cur.loc);
	      printf("), Executing:>\n");
	    }	    
	    interpret(ext->action);
	  }
	  /* Still at the same place? */
	  if (where(HERO) == oldloc) {
	    if (trcflg) {
	      printf("\n<EXIT %d (%s) from %d (", dir, 
		     (char *)addrTo(dict[wrds[wrdidx-1]].wrd), cur.loc);
	      debugsay(cur.loc);
	      printf("), Moving:>\n");
	    }
	    locate(HERO, ext->next);
	  }
	}
	return;
      }
      ext++;
    }
  error(M_NO_WAY);
}



/*----------------------------------------------------------------------

  findalt()

  Find the verb alternative wanted in a verb list and return
  the address to it.

 */
#ifdef _PROTOTYPES_
static AltElem *findalt(
     Aword vrbsadr,		/* IN - Address to start of list */
     Aword param		/* IN - Which parameter to match */
)
#else
static AltElem *findalt(vrbsadr, param)
     Aword vrbsadr;		/* IN - Address to start of list */
     Aword param;		/* IN - Which parameter to match */
#endif
{
  VrbElem *vrb;
  AltElem *alt;

  if (vrbsadr == 0)
    return(NULL);

  for (vrb = (VrbElem *) addrTo(vrbsadr); !endOfTable(vrb); vrb++)
    if (vrb->code == cur.vrb) {
      for (alt = (AltElem *) addrTo(vrb->alts); !endOfTable(alt); alt++)
	if (alt->param == param || alt->param == 0)
	  return alt;
      return NULL;
    }
  return NULL;
}




/*======================================================================

  possible()

  Check if current action is possible according to the CHECKs.

  */
#ifdef _PROTOTYPES_
Boolean possible(void)
#else
Boolean possible()
#endif
{
  AltElem *alt[MAXPARAMS+2];	/* List of alt-pointers, one for each param */
  int i;			/* Parameter index */
  
  fail = FALSE;
  alt[0] = findalt(header->vrbs, 0);
  /* Perform global checks */
  if (alt[0] != 0 && alt[0]->checks != 0) {
    if (!trycheck(alt[0]->checks, FALSE)) return FALSE;
    if (fail) return FALSE;
  }
  
  /* Now CHECKs in this location */
  alt[1] = findalt(locs[cur.loc-LOCMIN].vrbs, 0);
  if (alt[1] != 0 && alt[1]->checks != 0)
    if (!trycheck(alt[1]->checks, FALSE))
      return FALSE;
  
  for (i = 0; params[i].code != EOF; i++) {
    alt[i+2] = findalt(objs[params[i].code-OBJMIN].vrbs, i+1);
    /* CHECKs in a possible parameter */
    if (alt[i+2] != 0 && alt[i+2]->checks != 0)
      if (!trycheck(alt[i+2]->checks, FALSE))
	return FALSE;
  }

  for (i = 0; i < 2 || params[i-2].code != EOF; i++)
    if (alt[i] != 0 && alt[i]->action != 0)
      break;
  if (i >= 2 && params[i-2].code == EOF)
    /* Didn't find any code for this verb/object combination */
    return FALSE;
  else
    return TRUE;
}



/*----------------------------------------------------------------------

  do_it()

  Execute the action commanded by hero.

  */
#ifdef _PROTOTYPES_
static void do_it(void)
#else
static void do_it()
#endif
{
  AltElem *alt[MAXPARAMS+2];	/* List of alt-pointers, one for each param */
  Boolean done[MAXPARAMS+2];	/* Is it done */
  int i;			/* Parameter index */
  char trace[80];		/* Trace string buffer */
  
  fail = FALSE;
  alt[0] = findalt(header->vrbs, 0);
  /* Perform global checks */
  if (alt[0] != 0 && alt[0]->checks != 0) {
    if (!trycheck(alt[0]->checks, TRUE)) return;
    if (fail) return;
  }
  
  /* Now CHECKs in this location */
  alt[1] = findalt(locs[cur.loc-LOCMIN].vrbs, 0);
  if (alt[1] != 0 && alt[1]->checks != 0) {
    if (!trycheck(alt[1]->checks, TRUE)) return;
    if (fail) return;
  }
  
  for (i = 0; params[i].code != EOF; i++) {
    if (isLit(params[i].code))
      alt[i+2] = 0;
    else {
      if (isObj(params[i].code))
	alt[i+2] = findalt(objs[params[i].code-OBJMIN].vrbs, i+1);
      else if (isAct(params[i].code))
	alt[i+2] = findalt(acts[params[i].code-ACTMIN].vrbs, i+1);
      else
	syserr("Illegal parameter type.");
      /* CHECKs in the parameters */
      if (alt[i+2] != 0 && alt[i+2]->checks != 0) {
	if (!trycheck(alt[i+2]->checks, TRUE)) return;
	if (fail) return;
      }
    }
  }

  /* Check for anything to execute... */
  for (i = 0; i < 2 || params[i-2].code != EOF; i++)
    if (alt[i] != 0 && alt[i]->action != 0)
      break;
  if (i >= 2 && params[i-2].code == EOF)
    /* Didn't find any code for this verb/object combination */
    error(M_CANT0);
  
  /* Perform actions! */
  
  /* First try any BEFORE or ONLY from outside in */
  done[0] = FALSE;
  done[1] = FALSE;
  for (i = 2; params[i-2].code != EOF; i++)
    done[i] = FALSE;
  i--;
  while (i >= 0) {
    if (alt[i] != 0)
      if (alt[i]->qual == (Aword)Q_BEFORE || alt[i]->qual == (Aword)Q_ONLY) {
	if (alt[i]->action != 0) {
	  if (trcflg) {
	    if (i == 0)
	      strcpy(trace, "GLOBAL");
	    else if (i == 1)
	      strcpy(trace, "in LOCATION");
	    else
	      sprintf(trace, "in PARAMETER %d", i-1);
	    if (alt[i]->qual == (Aword)Q_BEFORE)
	      printf("\n<VERB %s (BEFORE), Body:>\n", trace);
	    else
	      printf("\n<VERB %s (ONLY), Body:>\n", trace);
	  }
	  interpret(alt[i]->action);
	  if (fail) return;
	  if (alt[i]->qual == (Aword)Q_ONLY) return;
	}
	done[i] = TRUE;
      }
    i--;
  }
  
  /* Then execute any not declared as AFTER, i.e. the default */
  for (i = 0; i < 2 || params[i-2].code != EOF; i++) {
    if (alt[i] != 0)
      if (alt[i]->qual != (Aword)Q_AFTER) {
	if (!done[i] && alt[i]->action != 0) {
	  if (trcflg) {
	    if (i == 0)
	      strcpy(trace, "GLOBAL");
	    else if (i == 1)
	      strcpy(trace, "in LOCATION");
	    else
	      sprintf(trace, "in PARAMETER %d", i-1);
	    printf("\n<VERB %s, Body:>\n", trace);
	  }
	  interpret(alt[i]->action);
	  if (fail) return;
	}
	done[i] = TRUE;
      }
  }

  /* Finally, the ones declared as after */
  i--;
  while (i >= 0) {
    if (alt[i] != 0)
      if (!done[i] && alt[i]->action != 0) {
	if (trcflg) {
	  if (i == 0)
	    strcpy(trace, "GLOBAL");
	  else if (i == 1)
	    strcpy(trace, "in LOCATION");
	  else
	    sprintf(trace, "in PARAMETER %d", i-1);
	  printf("\n<VERB %s (AFTER), Body:>\n", trace);
	}
	interpret(alt[i]->action);
	if (fail) return;
      }
    i--;
  }
}



/*======================================================================

  action()

  Execute all activities commanded. Handles possible multiple actions
  such as THEM or lists of objects.

  */
#ifdef _PROTOTYPES_
void action(
     ParamElem plst[]		/* IN - Plural parameters */
)
#else
void action(plst)
     ParamElem plst[];
#endif
{
  int i, pno;
  char marker[10];

  if (plural) {
    for (pno = 0; params[pno].code != 0; pno++);
    sprintf(marker, "($%d)", pno+1);
    for (i = 0; plst[i].code != EOF; i++) {
      params[pno] = plst[i];
      output(marker);
      do_it();
      if (plst[i+1].code != EOF)
        para();
    }
    params[pno].code = 0;
  } else
    do_it();
}



/*----------------------------------------------------------------------*\

  Event Handling

  eventchk()

\*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------
  eventchk()

  Check if any events are pending. If so execute them.
  */
#ifdef _PROTOTYPES_
static void eventchk(void)
#else
static void eventchk()
#endif
{
  while (etop != 0 && eventq[etop-1].time == cur.tick) {
    etop--;
    if (isLoc(eventq[etop].where))
      cur.loc = eventq[etop].where;
    else
      cur.loc = where(eventq[etop].where);
    if (trcflg) {
      printf("\n<EVENT %d (at ", eventq[etop].event);
      debugsay(cur.loc);
      printf("):>\n");
    }
    interpret(evts[eventq[etop].event-EVTMIN].code);
  }
}





/*----------------------------------------------------------------------*\

  Main program and initialisation

  codfil
  filenames
  checkvers()
  load()
  checkdebug()
  initheader()
  initstrings()
  start()
  init()
  main()

\*----------------------------------------------------------------------*/


static FILE *codfil;
static char codfnm[256] = "";
static char txtfnm[256] = "";
static char logfnm[256] = "";


/*----------------------------------------------------------------------

  checkvers()

 */
#ifdef _PROTOTYPES_
static void checkvers(AcdHdr *header)
#else
static void checkvers(header)
     AcdHdr *header;
#endif
{
  char *v = (char *)header;
  char *c = v+1;
  Aword vers;
  char state[2];

  /* Check version of .ACD file */
  vers = product.version.version<<8;
  vers+= product.version.revision;

  if (dbgflg) {
    state[0] = header->vers&0x0000000ff;
    state[1] = '\0';
    printf("<Version of '%s' is %d.%d(%d)%s>",
	   advnam,
	   (int)(header->vers>>24)&0xff,
	   (int)(header->vers>>16)&0xff,
	   (int)(header->vers>>8)&0xff,
	   (header->vers&0xff)==0? "": state);
    newline();
  }

  if (((*v)<<8)+(*c) != vers) {
    if (errflg)
      syserr("Incompatible version of ACODE program.");
    else
      printf("<WARNING! Incompatible version of ACODE program.>\n");
  }
}


/*----------------------------------------------------------------------

  load()

 */
#ifdef _PROTOTYPES_
static void load(void)
#else
static void load()
#endif
{
  AcdHdr hdr;
  Aword crc = 0;
  int i;
  char err[100];

  rewind(codfil);
  fread(&hdr, sizeof(hdr), 1, codfil);
  rewind(codfil);
  checkvers(&hdr);

  /* Allocate and load memory */

#ifdef REVERSED
  reverseHdr(&hdr);
#endif
  if (memory == NULL)
    memory = allocate(hdr.size*sizeof(Aword));
  header = (AcdHdr *) addrTo(0);

  memTop = fread(addrTo(0), sizeof(Aword), hdr.size, codfil);
  if (memTop != hdr.size)
    syserr("Could not read all ACD code.");

  /* Calculate checksum */
  for (i = sizeof(hdr)/sizeof(Aword); i < memTop; i++) {
    crc += memory[i]&0xff;
    crc += (memory[i]>>8)&0xff;
    crc += (memory[i]>>16)&0xff;
    crc += (memory[i]>>24)&0xff;
  }
  if (crc != hdr.acdcrc) {
    sprintf(err, "Checksum error in .ACD file (0x%lx instead of 0x%lx).",
	    crc, hdr.acdcrc);
    if (errflg)
      syserr(err);
    else
      printf("%s\n", err);
  }

#ifdef REVERSED
  if (dbgflg|trcflg|stpflg)
    printf("Hmm, please wait a moment while I set things up....\n");
  reverseACD();		/* Reverse all words in the ACD file */
#endif
}


/*----------------------------------------------------------------------

  checkdebug()

 */
#ifdef _PROTOTYPES_
static void checkdebug(void)
#else
static void checkdebug()
#endif
{
  /* Make sure he can't debug if not allowed! */
  if (!header->debug) {
    if (dbgflg|trcflg|stpflg)
      printf("<Sorry, '%s' is not compiled for debug!>\n", advnam);
    para();
    dbgflg = FALSE;
    trcflg = FALSE;
    stpflg = FALSE;
  }

  if (dbgflg)			/* If debugging */
    srand(0);			/* use no randomization */
  else
    srand(time(0));		/* seed random generator */
}


/*----------------------------------------------------------------------

  initheader()

 */
#ifdef _PROTOTYPES_
static void initheader(void)
#else
static void initheader()
#endif
{
  dict = (WrdElem *) addrTo(header->dict);
  /* Find out number of entries in dictionary */
  for (dictsize = 0; !endOfTable(&dict[dictsize]); dictsize++);
  vrbs = (VrbElem *) addrTo(header->vrbs);
  stxs = (StxElem *) addrTo(header->stxs);
  locs = (LocElem *) addrTo(header->locs);
  acts = (ActElem *) addrTo(header->acts);
  objs = (ObjElem *) addrTo(header->objs);
  evts = (EvtElem *) addrTo(header->evts);
  cnts = (CntElem *) addrTo(header->cnts);
  ruls = (RulElem *) addrTo(header->ruls);
  msgs = (MsgElem *) addrTo(header->msgs);
  scores = (Aword *) addrTo(header->scores);

  if (header->pack)
    freq = (Aword *) addrTo(header->freq);
}


/*----------------------------------------------------------------------

  initstrings()

  */
#ifdef _PROTOTYPES_
static void initstrings(void)
#else
static void initstrings()
#endif
{
  IniElem *init;

  for (init = (IniElem *) addrTo(header->init); !endOfTable(init); init++) {
    getstr(init->fpos, init->len);
    memory[init->adr] = pop();
  }
}


/*----------------------------------------------------------------------

  start()

 */
#ifdef _PROTOTYPES_
static void start(void)
#else
static void start()
#endif
{
  int startloc;

  cur.tick = 0;
  cur.loc = startloc = where(HERO);
  cur.act = HERO;
  cur.score = 0;
  if (trcflg)
    printf("\n<START:>\n");
  interpret(header->start);
  para();

  acts[HERO-ACTMIN].loc = 0;
  locate(HERO, startloc);
}



/*----------------------------------------------------------------------
  init()

  Initialization, program load etc.

  */
#ifdef _PROTOTYPES_
static void init(void)
#else
static void init()
#endif
{
  /* Initialise some status */
  etop = 0;			/* No pending events */
  looking = FALSE;		/* Not looking now */
  dscrstkp = 0;			/* No describe in progress */

  load();

  initheader();
  checkdebug();

  /* Initialise string attributes */
  initstrings();

  getPageSize();
  /* Start the adventure */
  start();
}



/*----------------------------------------------------------------------
  movactor()

  Let the current actor move. If player, ask him.

 */
#ifdef _PROTOTYPES_
static void movactor(void)
#else
static void movactor()
#endif
{
  ScrElem *scr;
  StepElem *step;
  ActElem *act = (ActElem *) &acts[cur.act-ACTMIN];

  cur.loc = where(cur.act);
  if (cur.act == HERO) {
    parse();
    fail = FALSE;			/* fail only aborts one actor */
    rules();
  } else if (act->script != 0) {
    for (scr = (ScrElem *) addrTo(act->scradr); !endOfTable(scr); scr++)
      if (scr->code == act->script) {
	/* Find correct step in the list by indexing */
	step = (StepElem *) addrTo(scr->steps);
	step = (StepElem *) &step[act->step];
	/* Now execute it, maybe. First check wait count */
	if (step->after > act->count) {
	  /* Wait some more */
	  if (trcflg) {
	    printf("\n<ACTOR %d, ", cur.act);
	    debugsay(cur.act);
	    printf(" (at ");
	    debugsay(cur.loc);
	    printf("), SCRIPT %ld, STEP %ld, Waiting %ld more>\n",
		   act->script, act->step+1, step->after-act->count);
	  }
	  act->count++;
	  rules();
	  return;
	} else
	  act->count = 0;
	/* Then check possible expression */
	if (step->exp != 0) {
	  if (trcflg) {
	    printf("\n<ACTOR %d, ", cur.act);
	    debugsay(cur.act);
	    printf(" (at ");
	    debugsay(cur.loc);
	    printf("), SCRIPT %ld, STEP %ld, Evaluating:>\n",
		   act->script, act->step+1);
	  }
	  interpret(step->exp);
	  if (!(Abool)pop()) {
	    rules();
	    return; /* Hadn't happened yet */
	  }
	}
	/* OK, so finally let him do his thing */
	act->step++;		/* Increment step number before executing... */
	if (trcflg) {
	  printf("\n<ACTOR %d, ", cur.act);
	  debugsay(cur.act);
	  printf(" (at ");
	  debugsay(cur.loc);
	  printf("), SCRIPT %ld, STEP %ld, Executing:>\n",
		 act->script, act->step);
	}
	interpret(step->stm);
	step++;
	/* ... so that we can see if he is USEing another script now */
	if (act->step != 0 && endOfTable(step))
	  /* No more steps in this script, so stop him */
	  act->script = 0;
	fail = FALSE;			/* fail only aborts one actor */
	rules();
	return;
      }
    syserr("Unknown actor script.");
  } else if (trcflg) {
    printf("\n<ACTOR %d, ", cur.act);
    debugsay(cur.act);
    printf(" (at ");
    debugsay(cur.loc);
    printf("), Idle>\n");
    rules();
    return;
  }
}

/*----------------------------------------------------------------------

  openFiles()

  Open the necessary files.

  */
#ifdef _PROTOTYPES_
static void openFiles(void)
#else
static void openFiles()
#endif
{
  char str[256];
  char *usr;
  time_t tick;

  /* Open Acode file */
  strcpy(codfnm, advnam);
  strcat(codfnm, ".acd");
  if ((codfil = fopen(codfnm, READ_MODE)) == NULL) {
    strcpy(str, "Can't open adventure code file '");
    strcat(str, codfnm);
    strcat(str, "'.");
    syserr(str);
  }

  /* Open Text file */
  strcpy(txtfnm, advnam);
  strcat(txtfnm, ".dat");
  if ((txtfil = fopen(txtfnm, READ_MODE)) == NULL) {
    strcpy(str, "Can't open adventure text data file '");
    strcat(str, txtfnm);
    strcat(str, "'.");
    syserr(str);
  }

  /* If logging open log file */
  if (logflg) {
#ifdef __amiga__
    strcpy(str, "AlanDir:log/");
    usr = "";
#else
#ifdef __vax__
    strcpy(str, "ALANDIR:");
    usr = "";
#else
#ifdef __unix__
    if (getenv("ALANDIR") == NULL)
      strcpy(str, "");
    else {
      strcpy(str, getenv("ALANDIR"));
      strcat(str, "/log/");
    }
    if ((usr = getenv("USER")) == NULL)
      usr = "";
#endif
#endif
#endif

    time(&tick);
    sprintf(logfnm, "%s%s%d%s.log", str, advnam, (int)tick, usr);
    if ((logfil = fopen(logfnm, "w")) == NULL)
      logflg = FALSE;
  }
}
    

/*======================================================================

  main()

  Main program of main unit in Alan interpreter module, ARUN

  */
#ifdef _PROTOTYPES_
int main(
     unsigned argc,
     char *argv[]
)
#else
int main(argc, argv)
     unsigned argc;
     char *argv[];
#endif
{
#ifdef MALLOC
  malloc_debug(2);
#endif

  args(argc, argv);

  /* Set up page format in case we get a system error */
  lin = col = 1;
  header->paglen = 24;
  header->pagwidth = 70;
  getPageSize();

  if (dbgflg) {
    printf("<Arun, Adventure Interpreter version %s>",
	   product.version.string);
    newline();
  }
  
  if (strcmp(advnam, "") == 0) {
    printf("No Adventure name given.\n");
    terminate(0);
  }

  openFiles();

  setjmp(restart);		/* Return here if he wanted to restart */

  init();			/* Load, initialise and start the adventure */

  while (TRUE) {
#ifdef MALLOC
    if (malloc_verify() == 0) syserr("Error in heap.");
#endif
    if (dbgflg)
      debug();

    eventchk();
    cur.tick++;
    (void) setjmp(jmpbuf);

    /* Move all characters */
    for (cur.act = ACTMIN; cur.act <= ACTMAX; cur.act++)
      movactor();
  }
}

