/* Clear Screen + Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "clrscr.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
header (char *headertext)
{
  int i;
  mvaddstr (0, 1, PROGRAM_NAME);

  mvaddstr (0, (80 - strlen (headertext) + 1) / 2,
	    headertext);

  move (1, 0);
  for (i = 0; i < 80; i++)
    addch (ACS_S1);
}

void
clearscreen (char *headertext)
{
  clear ();
  refresh ();

  header (headertext);
}
