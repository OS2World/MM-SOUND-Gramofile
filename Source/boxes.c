/* Boxes

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "boxes.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
mybox (int y, int x, int h, int w)
/* Draws a box, top left = (y,x), heigth = h, width = w. */
{
  mvaddch (y, x, ACS_ULCORNER);
  mvaddch (y, x + w - 1, ACS_URCORNER);
  mvaddch (y + h - 1, x, ACS_LLCORNER);
  mvaddch (y + h - 1, x + w - 1, ACS_LRCORNER);

  move (y, x + 1);
  hline (ACS_HLINE, w - 2);
  move (y + 1, x + w - 1);
  vline (ACS_VLINE, h - 2);
  move (y + h - 1, x + 1);
  hline (ACS_HLINE, w - 2);
  move (y + 1, x);
  vline (ACS_VLINE, h - 2);
}
