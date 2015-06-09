/* Error window

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "errorwindow.h"
#include "buttons.h"
#include "boxes.h"
#include "textwindow.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
error_window_display (char *text, char *buttontext)
{
  button_t ok_button;

  ok_button.text = buttontext;
  ok_button.y = ERROR_WINDOW_Y + ERROR_WINDOW_H - 1;
  ok_button.x = ERROR_WINDOW_X + ERROR_WINDOW_W
    - 1 - strlen (ok_button.text);
  ok_button.selected = TRUE;

  mybox (ERROR_WINDOW_Y - 1, ERROR_WINDOW_X - 1,
	 ERROR_WINDOW_H + 2, ERROR_WINDOW_W + 2);
  display_textwin ("", ERROR_WINDOW_Y, ERROR_WINDOW_X,
		   ERROR_WINDOW_H, ERROR_WINDOW_W);
  display_textwin (text, ERROR_WINDOW_Y, ERROR_WINDOW_X + 1,
		   ERROR_WINDOW_H, ERROR_WINDOW_W - 2);
  button_display (&ok_button);
  move (0, 79);
  refresh ();
}


void
error_window (char *text)
{
  int i;

  error_window_display (text, " OK ");

  do
    i = getch ();
  while (i != 13 && i != KEY_ENTER && i != 27);

  clear ();
  refresh ();
}
