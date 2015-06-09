/* String input

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "stringinput.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
stringinput_display (stringinput_t * data)
{
  int i;

  if (data->cursorpos < 0)
    data->cursorpos = 0;
  if (data->cursorpos > strlen (data->string))
    data->cursorpos = strlen (data->string);

  if (data->cursorpos < data->firstcharonscreen + 2)
    data->firstcharonscreen = data->cursorpos - 2;
  if (data->cursorpos >= data->firstcharonscreen + data->w)
    data->firstcharonscreen = data->cursorpos - data->w + 1;

  if (data->firstcharonscreen < 0)
    data->firstcharonscreen = 0;

  move (data->y, data->x);
  i = data->firstcharonscreen;
  while (i < strlen (data->string) && i < data->firstcharonscreen + data->w)
    {
      addch (data->string[i]);
      i++;
    }

  while (i < data->firstcharonscreen + data->w)
    {
      addch (' ');
      i++;
    }

  move (data->y, data->x + data->cursorpos - data->firstcharonscreen);
}

void
stringinput_stdkeys (int key, stringinput_t * data)
{
  char helpstring[500];

  switch (key)
    {
    case KEY_LEFT:
      data->cursorpos--;
      break;

    case KEY_RIGHT:
      data->cursorpos++;
      break;

    case KEY_HOME:
      data->cursorpos = 0;
      break;

    case KEY_END:
      data->cursorpos = strlen (data->string);
      break;

    case KEY_BACKSPACE:
#ifdef TREAT_DEL_AS_BACKSPACE
    case 127:
#endif
      if (data->cursorpos > 0)
	{
	  strcpy (helpstring, data->string);
	  strcpy (helpstring + data->cursorpos - 1,
		  data->string + data->cursorpos);
	  strcpy (data->string, helpstring);
	  data->cursorpos--;
	}
      break;

#ifndef TREAT_DEL_AS_BACKSPACE
    case 127:			/* DEL */
      if (data->cursorpos < strlen (data->string))
	{
	  strcpy (helpstring, data->string);
	  strcpy (helpstring + data->cursorpos,
		  data->string + data->cursorpos + 1);
	  strcpy (data->string, helpstring);
	}
      break;
#endif
    }

  if (key >= 32 && key <= 126)	/* insert */
    {
      strcpy (helpstring, data->string);
      strcpy (helpstring + data->cursorpos + 1,
	      data->string + data->cursorpos);
      helpstring[data->cursorpos] = (char) key;
      strcpy (data->string, helpstring);
      data->cursorpos++;
    }

  if (data->cursorpos < 0)
    data->cursorpos = 0;
  if (data->cursorpos > strlen (data->string))
    data->cursorpos = strlen (data->string);
}
