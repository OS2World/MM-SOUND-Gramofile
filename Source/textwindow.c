/* Text in a 'window'

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "textwindow.h"
#include <stdlib.h>
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
display_textwin (char *text, int y, int x, int h, int w)
			/* (y,x): upper left position of 'text block'
			   h = heigth, w = width of block */
{

  int current_y;
  char mytext[DISPLAYMENU_MAXTEXTLEN];
  char mytext2[DISPLAYMENU_MAXTEXTLEN];
  char helptext[DISPLAYMENU_MAXTEXTLEN];
  char *lastspace;
  int i;

  current_y = y;
  strncpy (mytext, text, DISPLAYMENU_MAXTEXTLEN);

  do
    {
      strncpy (mytext2, mytext, w + 1);
      mytext2[w + 1] = '\0';

      if (strlen (mytext2) > w)
	{
	  lastspace = strrchr (mytext2, ' ');
	  if (lastspace != NULL)
	    {
	      mytext2[lastspace - mytext2] = '\0';

	      strcpy (helptext, mytext + (lastspace - mytext2 + 1));

	      strcpy (mytext, helptext);
	    }
	  else
	    /* no space... */
	    {
	      mytext2[w] = '\0';

	      strcpy (helptext, mytext + w);
	      strcpy (mytext, helptext);
	    }
	}
      else			/* strlen(mytext2) <= w */
	mytext[0] = '\0';

      mvaddstr (current_y, x, mytext2);

      for (i = strlen (mytext2) + 1; i <= w; i++)
	addch (' ');

      current_y++;
    }
  while (current_y < y + h /*&& strlen(mytext) > 0 */ );
}
