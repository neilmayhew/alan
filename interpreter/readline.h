#ifndef _READLINE_H_
#define _READLINE_H_
/*----------------------------------------------------------------------*\

  readline.h

  Header file for user input, history and editing support

\*----------------------------------------------------------------------*/

#include "types.h"

#define LINELENGTH 80
#define HISTORYLENGTH 20

extern Bool readline(char usrbuf[]);

#endif
