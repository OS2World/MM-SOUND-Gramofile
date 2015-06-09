/* Simple Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_median.h"
#include "signpr_general.h"
#include "errorwindow.h"
#include "stringinput.h"
#include "buttons.h"
#include "clrscr.h"
#include "boxes.h"
#include "helpline.h"
#include <stdlib.h>
#include <stdio.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
simple_median_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 1;
  parampointer->prelength1 = 1;
}

void
simple_median_param_screen (parampointer_t parampointer)
{
  stringinput_t medlengthstr;
  button_t ok_button, cancel_button;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 0;
  int in_ch;
  int i;
  long helplong;

  char *helplines[3] =
  {
    " ^: broader ticks removed.              v: less distortion.                    ",
    " Discard changes.                                                              ",
    " Accept changes.                                                               "};

  medlengthstr.maxlen = 500;
  medlengthstr.string = (char *) malloc (medlengthstr.maxlen *
					 sizeof (char));
  sprintf (medlengthstr.string, "%ld", parampointer->prelength1 +
	   parampointer->postlength1 + 1);
  medlengthstr.y = 6;
  medlengthstr.x = 44;
  medlengthstr.w = 19;
  medlengthstr.cursorpos = strlen (medlengthstr.string);
  medlengthstr.firstcharonscreen = strlen (medlengthstr.string) -
    medlengthstr.w + 2;
  if (medlengthstr.firstcharonscreen < 0)
    medlengthstr.firstcharonscreen = 0;

  ok_button.text = " OK ";
  ok_button.y = 20;
  ok_button.x = 71;
  ok_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 5;
  cancel_button.selected = FALSE;

  clearscreen (SIGNPR_MEDIAN_PARAMSCR_HEADERTEXT);

  do
    {
      header (SIGNPR_MEDIAN_PARAMSCR_HEADERTEXT);

      if (focus == 1)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 2)
	ok_button.selected = TRUE;
      else
	ok_button.selected = FALSE;

      mvprintw (3, 2,
	    "See the Signproc.txt file for the meaning of the parameters.");

      stringinput_display (&medlengthstr);
      mvprintw (medlengthstr.y, 2,
		"Number of samples to take the median of:");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&ok_button);
      mybox (ok_button.y - 1, ok_button.x - 1,
	     3, strlen (ok_button.text) + 2);

      helpline (helplines[focus]);

      if (focus == 0)
	stringinput_display (&medlengthstr);
      else
	move (0, 79);

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* medlengthstr */
	  stringinput_stdkeys (in_ch, &medlengthstr);
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      i = sscanf (medlengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus = 2;
	    }
	  else
	    switch (in_ch)
	      {
	      case KEY_UP:
		focus--;
		break;
	      case KEY_DOWN:
		focus++;
		break;
	      }
	  break;

	case 1:		/* Cancel */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      returnval = 0;
	      dont_stop = FALSE;
	    }
	  else
	    switch (in_ch)
	      {
	      case KEY_LEFT:
	      case KEY_UP:
		focus--;
		break;
	      case KEY_RIGHT:
	      case KEY_DOWN:
		focus++;
		break;
	      }
	  break;

	case 2:		/* OK */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      i = sscanf (medlengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as median length.");
		  medlengthstr.cursorpos =
		    strlen (medlengthstr.string);
		  focus = 0;
		}
	      else
		{
		  parampointer->prelength1 = (helplong - 1) / 2;
		  parampointer->postlength1 = (helplong - 1) / 2;
		  dont_stop = FALSE;
		}
	    }
	  else
	    switch (in_ch)
	      {
	      case KEY_LEFT:
	      case KEY_UP:
		focus--;
		break;
	      case KEY_RIGHT:
	      case KEY_DOWN:
		focus++;
		break;
	      }
	  break;
	}

      if (in_ch == 9)		/* TAB */
	focus++;

      if (in_ch == 27)
	dont_stop = FALSE;

      if (focus > 2)
	focus -= 3;
      if (focus < 0)
	focus += 3;
    }
  while (dont_stop);

  free (medlengthstr.string);
}

void
init_simple_median_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (parampointer->postlength1,
				      parampointer->prelength1);

  parampointer->filterno = filterno;

  parampointer->sslist1 = (signed short *) malloc
    ((parampointer->postlength1 +
      parampointer->prelength1 + 1)
     * sizeof (signed short));

  parampointer->sslist2 = (signed short *) malloc
    ((parampointer->postlength1 +
      parampointer->prelength1 + 1)
     * sizeof (signed short));
}

void
delete_simple_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  free (parampointer->sslist1);
  free (parampointer->sslist2);
}


sample_t
simple_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  long i;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  for (i = 0; i <= parampointer->postlength1 + parampointer->prelength1;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1);
      parampointer->sslist1[i] = sample.left;
      parampointer->sslist2[i] = sample.right;
    }

  sample.left = median (parampointer->sslist1,
			parampointer->postlength1 +
			parampointer->prelength1 + 1);
  sample.right = median (parampointer->sslist2,
			 parampointer->postlength1 +
			 parampointer->prelength1 + 1);

  return sample;
}
