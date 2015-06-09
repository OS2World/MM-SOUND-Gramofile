/* Double Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_doubmed.h"
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
double_median_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 2;
  parampointer->prelength1 = 2;
  parampointer->postlength2 = 2;
  parampointer->prelength2 = 2;
}

void
double_median_param_screen (parampointer_t parampointer)
{
  stringinput_t medlength1str, medlength2str;
  button_t ok_button, cancel_button;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 0;
  int in_ch;
  int i;
  long helplong;

  char *helplines[4] =
  {
    " ^: more distortion.                    v: less effective against ticks.       ",
    " ^: less correction of distortion.      v: more (faulty) correction.           ",
    " Discard changes.                                                              ",
    " Accept changes.                                                               "};

  medlength1str.maxlen = 500;
  medlength1str.string = (char *) malloc (medlength1str.maxlen *
					  sizeof (char));
  sprintf (medlength1str.string, "%ld", parampointer->prelength1 +
	   parampointer->postlength1 + 1);
  medlength1str.y = 6;
  medlength1str.x = 57;
  medlength1str.w = 19;
  medlength1str.cursorpos = strlen (medlength1str.string);
  medlength1str.firstcharonscreen = 0;

  medlength2str.maxlen = 500;
  medlength2str.string = (char *) malloc (medlength2str.maxlen *
					  sizeof (char));
  sprintf (medlength2str.string, "%ld", parampointer->prelength2 +
	   parampointer->postlength2 + 1);
  medlength2str.y = 8;
  medlength2str.x = 57;
  medlength2str.w = 19;
  medlength2str.cursorpos = strlen (medlength2str.string);
  medlength2str.firstcharonscreen = 0;

  ok_button.text = " OK ";
  ok_button.y = 20;
  ok_button.x = 71;
  ok_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 5;
  cancel_button.selected = FALSE;

  clearscreen (SIGNPR_DOUBMED_PARAMSCR_HEADERTEXT);

  do
    {
      header (SIGNPR_DOUBMED_PARAMSCR_HEADERTEXT);

      if (focus == 2)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 3)
	ok_button.selected = TRUE;
      else
	ok_button.selected = FALSE;

      mvprintw (3, 2,
	    "See the Signproc.txt file for the meaning of the parameters.");

      stringinput_display (&medlength1str);
      mvprintw (medlength1str.y, 2,
		"Number of samples for the first median:");

      stringinput_display (&medlength2str);
      mvprintw (medlength2str.y, 2,
		"Number of samples for the second (correction) median:");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&ok_button);
      mybox (ok_button.y - 1, ok_button.x - 1,
	     3, strlen (ok_button.text) + 2);

      helpline (helplines[focus]);

      switch (focus)
	{
	case 0:
	  stringinput_display (&medlength1str);
	  break;
	case 1:
	  stringinput_display (&medlength2str);
	  break;
	default:
	  move (0, 79);
	}

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* medlength1str */
	  stringinput_stdkeys (in_ch, &medlength1str);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (medlength1str.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus = 1;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 1:		/* medlength2str */
	  stringinput_stdkeys (in_ch, &medlength2str);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (medlength2str.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus = 3;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 2:		/* Cancel */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      returnval = 0;
	      dont_stop = FALSE;
	      break;

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

	case 3:		/* OK */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (medlength1str.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as first median length.");
		  medlength1str.cursorpos =
		    strlen (medlength1str.string);
		  focus = 0;
		  break;
		}

	      parampointer->prelength1 = (helplong - 1) / 2;
	      parampointer->postlength1 = (helplong - 1) / 2;

	      i = sscanf (medlength2str.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as second median length.");
		  medlength2str.cursorpos =
		    strlen (medlength2str.string);
		  focus = 1;
		  break;
		}

	      parampointer->prelength2 = (helplong - 1) / 2;
	      parampointer->postlength2 = (helplong - 1) / 2;

	      dont_stop = FALSE;
	      break;

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

      if (focus > 3)
	focus = 0;
      if (focus < 0)
	focus = 3;
    }
  while (dont_stop);

  free (medlength1str.string);
  free (medlength2str.string);
}

void
init_double_median_filter (int filterno, parampointer_t parampointer)
{
  long total_post;

  total_post = parampointer->postlength2;
  if (parampointer->postlength1 > total_post)
    total_post = parampointer->postlength1;

  parampointer->buffer = init_buffer (total_post,
				      parampointer->prelength1 +
				      parampointer->prelength2);
  parampointer->buffer2 = init_buffer (parampointer->postlength2,
				       parampointer->prelength2);

  parampointer->filterno = filterno;
}

void
delete_double_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
}

sample_t
double_median_1 (long offset, long offset_zero,
		 parampointer_t parampointer)
{
  sample_t sample;
  signed short list1[parampointer->postlength1 + parampointer->prelength1 + 1];
  signed short list2[parampointer->postlength1 + parampointer->prelength1 + 1];
  long i;

  for (i = 0; i <= parampointer->postlength1 +
       parampointer->prelength1; i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1 +
				offset + offset_zero);
      list1[i] = sample.left;
      list2[i] = sample.right;
    }

  sample.left = median (list1, parampointer->postlength1 +
			parampointer->prelength1 + 1);
  sample.right = median (list2, parampointer->postlength1 +
			 parampointer->prelength1 + 1);
  return sample;
}

fillfuncpointer_t double_median_1_pointer = double_median_1;

sample_t
double_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  sample_t sample2;
  sample_t returnval;
  signed short list1[parampointer->postlength2 + parampointer->prelength2 + 1];
  signed short list2[parampointer->postlength2 + parampointer->prelength2 + 1];
  long i, j;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer2,
			      double_median_1_pointer,
			      0,
			      parampointer);


  for (i = 0; i <= parampointer->postlength2 +
       parampointer->prelength2; i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength2);
      sample2 = get_from_buffer (&parampointer->buffer2,
				 i - parampointer->postlength2);

      j = sample.left - sample2.left;
      j /= 2;
      list1[i] = j;

      j = sample.right - sample2.right;
      j /= 2;
      list2[i] = j;
    }

  sample2 = get_from_buffer (&parampointer->buffer2, 0);

  returnval.left = median (list1, parampointer->postlength2 +
			   parampointer->prelength2 + 1) * 2
    + sample2.left;

  returnval.right = median (list2, parampointer->postlength2 +
			    parampointer->prelength2 + 1) * 2
    + sample2.right;

  return returnval;
}
