/* Yes/No window

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "yesnowindow.h"
#include "buttons.h"
#include "boxes.h"
#include "textwindow.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


int
yesno_window (char *text, char *yestext, char *notext, int preselected)
/* returns 1 if yes-button selected,
   0 if no-button selected or Escape pressed */
{
  button_t yes_button, no_button;
  int i;
  int focus;

  yes_button.text = yestext;
  yes_button.y = YESNO_WINDOW_Y + YESNO_WINDOW_H - 1;
  yes_button.x = YESNO_WINDOW_X + YESNO_WINDOW_W
    - 1 - strlen (yes_button.text);
  yes_button.selected = FALSE;

  no_button.text = notext;
  no_button.y = YESNO_WINDOW_Y + YESNO_WINDOW_H - 1;
  no_button.x = YESNO_WINDOW_X + 1;
  no_button.selected = FALSE;

  mybox (YESNO_WINDOW_Y - 1, YESNO_WINDOW_X - 1,
	 YESNO_WINDOW_H + 2, YESNO_WINDOW_W + 2);
  display_textwin ("", YESNO_WINDOW_Y, YESNO_WINDOW_X,
		   YESNO_WINDOW_H, YESNO_WINDOW_W);
  display_textwin (text, YESNO_WINDOW_Y, YESNO_WINDOW_X + 1,
		   YESNO_WINDOW_H, YESNO_WINDOW_W - 2);

  focus = preselected;

  do
    {
      yes_button.selected = FALSE;
      no_button.selected = FALSE;
      if (focus == 0)
	no_button.selected = TRUE;
      if (focus == 1)
	yes_button.selected = TRUE;

      button_display (&yes_button);
      button_display (&no_button);

      move (0, 79);
      refresh ();

      do
	i = getch ();
      while (i != 13 && i != KEY_ENTER && i != 27 && i != KEY_LEFT
	     && i != KEY_RIGHT && i != 9);

      switch (i)
	{
	case KEY_LEFT:
	  focus = 0;
	  break;

	case KEY_RIGHT:
	  focus = 1;
	  break;

	case 9:
	  focus = 1 - focus;
	  break;
	}

    }
  while (i != 13 && i != KEY_ENTER && i != 27);

  clear ();
  refresh ();

  if (i == 27)
    return 0;
  else
    return focus;
}
