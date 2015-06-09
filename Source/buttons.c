/* Buttons

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "buttons.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
button_display (button_t * button)
{
  int y, x;
  if (button->selected)
    attron (A_STANDOUT);

  mvaddstr (button->y, button->x, button->text);

  getyx (stdscr, y, x);
  move (y, x - 1);

  if (button->selected)
    attroff (A_STANDOUT);
}
