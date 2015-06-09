/* RMS Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_rms.h"
#include "signpr_general.h"
#include "errorwindow.h"
#include "stringinput.h"
#include "buttons.h"
#include "clrscr.h"
#include "boxes.h"
#include "helpline.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
rms_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 1;
  parampointer->prelength1 = 1;
}

void
rms_param_screen (parampointer_t parampointer)
{
  stringinput_t rmslengthstr;
  button_t ok_button, cancel_button;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 0;
  int in_ch;
  int i;
  long helplong;

  char *helplines[3] =
  {
    "                                                                               ",
    " Discard changes.                                                              ",
    " Accept changes.                                                               "};

  rmslengthstr.maxlen = 500;
  rmslengthstr.string = (char *) malloc (rmslengthstr.maxlen *
					 sizeof (char));
  sprintf (rmslengthstr.string, "%ld", parampointer->prelength1 +
	   parampointer->postlength1 + 1);
  rmslengthstr.y = 6;
  rmslengthstr.x = 40;
  rmslengthstr.w = 15;
  rmslengthstr.cursorpos = strlen (rmslengthstr.string);
  rmslengthstr.firstcharonscreen = 0;

  ok_button.text = " OK ";
  ok_button.y = 20;
  ok_button.x = 71;
  ok_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 5;
  cancel_button.selected = FALSE;

  clearscreen (SIGNPR_RMS_PARAMSCR_HEADERTEXT);

  do
    {
      header (SIGNPR_RMS_PARAMSCR_HEADERTEXT);

      if (focus == 1)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 2)
	ok_button.selected = TRUE;
      else
	ok_button.selected = FALSE;

/*      mvprintw (3, 2,
   "See the Signproc.txt file for the meaning of the parameters.");
 */
      stringinput_display (&rmslengthstr);
      mvprintw (rmslengthstr.y, 2,
		"Number of samples to compute RMS of:");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&ok_button);
      mybox (ok_button.y - 1, ok_button.x - 1,
	     3, strlen (ok_button.text) + 2);

      helpline (helplines[focus]);

      if (focus == 0)
	stringinput_display (&rmslengthstr);
      else
	move (0, 79);

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* rmslengthstr */
	  stringinput_stdkeys (in_ch, &rmslengthstr);
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      i = sscanf (rmslengthstr.string, "%li", &helplong);
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
	      i = sscanf (rmslengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as RMS length.");
		  rmslengthstr.cursorpos =
		    strlen (rmslengthstr.string);
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

  free (rmslengthstr.string);
}

void
init_rms_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (parampointer->postlength1,
				      parampointer->prelength1);

  parampointer->filterno = filterno;
}

void
delete_rms_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
rms_filter (parampointer_t parampointer)
{
  doublesample_t sum;
  sample_t sample;
  long i;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  sum.left = 0;
  sum.right = 0;

  for (i = 0; i <= parampointer->postlength1 + parampointer->prelength1;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1);
      sum.left += sample.left * sample.left;
      sum.right += sample.right * sample.right;
    }

  sample.left = sqrt (sum.left / (parampointer->postlength1 +
				  parampointer->prelength1 + 1));
  sample.right = sqrt (sum.right / (parampointer->postlength1 +
				    parampointer->prelength1 + 1));

  return sample;
}
