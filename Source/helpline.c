/* Helpline (footer)

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "helpline.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
helpline (char *helptext)
{
  int i;

  move (22, 0);
  for (i = 0; i < 80; i++)
    addch (ACS_S9);
  mvaddstr (23, 0, helptext);

}
