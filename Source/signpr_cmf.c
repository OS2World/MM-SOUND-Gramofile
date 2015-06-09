/* Conditional Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/* Remove the `dont' to get b[t].left on left channel and g[t].left on
   right channel - useful for verifying properties.

   Note that
   b[t] is z[t] if RMSlength=RMFlength=1                                    
   and 
   b[t] is w[t] if RMFlength=1                                              
   (See also Signproc.txt)
 */
#define dontVIEW_INTERNALS


#include "signpr_cmf.h"
#include "signpr_general.h"
#include "errorwindow.h"
#include "stringinput.h"
#include "buttons.h"
#include "clrscr.h"
#include "boxes.h"
#include "helpline.h"
#include "yesnowindow.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


/* Macros I used first:

   COND_MEDIAN_MF_POSTLENGTH   ==  parampointer->postlength1
   COND_MEDIAN_MF_PRELENGTH    ==  parampointer->prelength1

   COND_MEDIAN_RMS_POSTLENGTH  ==  parampointer->postlength2
   COND_MEDIAN_RMS_PRELENGTH   ==  parampointer->prelength2

   COND_MEDIAN_RMF_POSTLENGTH  ==  parampointer->postlength3
   COND_MEDIAN_RMF_PRELENGTH   ==  parampointer->prelength3

   COND_MEDIAN_RMF_DECIMATE    ==  parampointer->int1 

   COND_MEDIAN_THRESHOLD       ==  parampointer->long1 
 */


void
cond_median_param_defaults (parampointer_t parampointer)
{
  /* Best tick-reduction: 21 - 9 - 11 - 5 - 2500 */

  parampointer->postlength1 = 10;
  parampointer->prelength1 = 10;
  parampointer->postlength2 = 4;
  parampointer->prelength2 = 4;
  parampointer->postlength3 = 5;
  parampointer->prelength3 = 5;
  parampointer->int1 = 5;	/* actually, this should really be 12 */
  parampointer->long1 = 2500;

  /* If you experience badly affected sound, try 15 - 11 - 9 - 4 - 2500 */
}

void
cond_median_param_screen (parampointer_t parampointer)
{
  stringinput_t medlengthstr, rmslengthstr, rmflengthstr, decimatestr,
    thresholdstr;
  button_t ok_button, cancel_button, defaults_button;
  int dont_stop = TRUE;
  int focus = 0;
  int in_ch;
  int i;
  long helplong;

  char *helplines[8] =
  {
    " ^: no neat interpolation.              v: broad ticks not filtered out.       ",
    " ^: less ticks detected.                v: not all of tick interpolated.       ",
    " ^: bad following of dynamics.          v: less ticks detected.                ",
    " ^: bad following of dynamics.          v: less ticks detected.                ",
    " ^: only strong ticks detected.         v: music-ticks also filtered out.      ",
    " Discard changes.                                                              ",
    " Reset default values.                                                         ",
    " Accept changes.                                                               "};

  medlengthstr.maxlen = 500;
  medlengthstr.string = (char *) malloc (medlengthstr.maxlen *
					 sizeof (char));
  sprintf (medlengthstr.string, "%ld", parampointer->prelength1 +
	   parampointer->postlength1 + 1);
  medlengthstr.y = 6;
  medlengthstr.x = 57;
  medlengthstr.w = 19;
  medlengthstr.cursorpos = strlen (medlengthstr.string);
  medlengthstr.firstcharonscreen = 0;

  rmslengthstr.maxlen = 500;
  rmslengthstr.string = (char *) malloc (rmslengthstr.maxlen *
					 sizeof (char));
  sprintf (rmslengthstr.string, "%ld", parampointer->prelength2 +
	   parampointer->postlength2 + 1);
  rmslengthstr.y = 8;
  rmslengthstr.x = 57;
  rmslengthstr.w = 19;
  rmslengthstr.cursorpos = strlen (rmslengthstr.string);
  rmslengthstr.firstcharonscreen = 0;

  rmflengthstr.maxlen = 500;
  rmflengthstr.string = (char *) malloc (rmflengthstr.maxlen *
					 sizeof (char));
  sprintf (rmflengthstr.string, "%ld", parampointer->prelength3 +
	   parampointer->postlength3 + 1);
  rmflengthstr.y = 10;
  rmflengthstr.x = 57;
  rmflengthstr.w = 19;
  rmflengthstr.cursorpos = strlen (rmflengthstr.string);
  rmflengthstr.firstcharonscreen = 0;

  decimatestr.maxlen = 500;
  decimatestr.string = (char *) malloc (decimatestr.maxlen *
					sizeof (char));
  sprintf (decimatestr.string, "%d", parampointer->int1);
  decimatestr.y = 12;
  decimatestr.x = 57;
  decimatestr.w = 19;
  decimatestr.cursorpos = strlen (decimatestr.string);
  decimatestr.firstcharonscreen = 0;

  thresholdstr.maxlen = 500;
  thresholdstr.string = (char *) malloc (thresholdstr.maxlen *
					 sizeof (char));
  sprintf (thresholdstr.string, "%ld", parampointer->long1);
  thresholdstr.y = 14;
  thresholdstr.x = 57;
  thresholdstr.w = 19;
  thresholdstr.cursorpos = strlen (thresholdstr.string);
  thresholdstr.firstcharonscreen = 0;

  ok_button.text = " OK ";
  ok_button.y = 20;
  ok_button.x = 71;
  ok_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 5;
  cancel_button.selected = FALSE;

  defaults_button.text = " Defaults ";
  defaults_button.y = 20;
  defaults_button.x = 36;
  defaults_button.selected = FALSE;

  clearscreen (SIGNPR_CMF_PARAMSCR_HEADERTEXT);

  do
    {
      header (SIGNPR_CMF_PARAMSCR_HEADERTEXT);

      if (focus == 5)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 6)
	defaults_button.selected = TRUE;
      else
	defaults_button.selected = FALSE;

      if (focus == 7)
	ok_button.selected = TRUE;
      else
	ok_button.selected = FALSE;

      mvprintw (3, 2,
	    "See the Signproc.txt file for the meaning of the parameters.");

      stringinput_display (&medlengthstr);
      mvprintw (medlengthstr.y, 2,
		"Number of samples for median to interpolate ticks:");

      stringinput_display (&rmslengthstr);
      mvprintw (rmslengthstr.y, 2,
		"Length of the RMS operation (samples):");

      stringinput_display (&rmflengthstr);
      mvprintw (rmflengthstr.y, 2,
		"Length of the recursive median operation (samples):");

      stringinput_display (&decimatestr);
      mvprintw (decimatestr.y, 2,
		"Decimation factor for the recursive median:");

      stringinput_display (&thresholdstr);
      mvprintw (thresholdstr.y, 2,
		"Threshold for tick detection (thousandths):");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&defaults_button);
      mybox (defaults_button.y - 1, defaults_button.x - 1,
	     3, strlen (defaults_button.text) + 2);
      button_display (&ok_button);
      mybox (ok_button.y - 1, ok_button.x - 1,
	     3, strlen (ok_button.text) + 2);

      helpline (helplines[focus]);

      switch (focus)
	{
	case 0:
	  stringinput_display (&medlengthstr);
	  break;
	case 1:
	  stringinput_display (&rmslengthstr);
	  break;
	case 2:
	  stringinput_display (&rmflengthstr);
	  break;
	case 3:
	  stringinput_display (&decimatestr);
	  break;
	case 4:
	  stringinput_display (&thresholdstr);
	  break;
	default:
	  move (0, 79);
	}

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* medlengthstr */
	  stringinput_stdkeys (in_ch, &medlengthstr);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (medlengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus++;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 1:		/* rmslengthstr */
	  stringinput_stdkeys (in_ch, &rmslengthstr);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (rmslengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus++;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 2:		/* rmflengthstr */
	  stringinput_stdkeys (in_ch, &rmflengthstr);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (rmflengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		error_window ("A whole, odd number, greater than 0, must \
be specified.");
	      else
		focus++;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 3:		/* decimatestr */
	  stringinput_stdkeys (in_ch, &decimatestr);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (decimatestr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1)
		error_window ("A whole number, greater than 0, must \
be specified.");
	      else
		focus++;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 4:		/* thresholdstr */
	  stringinput_stdkeys (in_ch, &thresholdstr);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      i = sscanf (thresholdstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1000)
		error_window ("A whole number, greater than 1000, must \
be specified.");
	      else
		focus = 7;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 5:		/* Cancel */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
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

	case 6:		/* Defaults */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      if (yesno_window ("Restore default parameters?", " Yes ",
				" No ", 0))
		{
		  cond_median_param_defaults (parampointer);
		  dont_stop = FALSE;
		}
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

	case 7:		/* OK */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:

	      i = sscanf (medlengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as median length.");
		  medlengthstr.cursorpos =
		    strlen (medlengthstr.string);
		  focus = 0;
		  break;
		}

	      parampointer->prelength1 = (helplong - 1) / 2;
	      parampointer->postlength1 = (helplong - 1) / 2;

	      i = sscanf (rmslengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as RMS length.");
		  rmslengthstr.cursorpos =
		    strlen (rmslengthstr.string);
		  focus = 1;
		  break;
		}

	      parampointer->prelength2 = (helplong - 1) / 2;
	      parampointer->postlength2 = (helplong - 1) / 2;

	      i = sscanf (rmflengthstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1 || helplong % 2 == 0)
		{
		  error_window ("A whole, odd number, greater than 0, must \
be specified as length of the recursive median.");
		  rmflengthstr.cursorpos =
		    strlen (rmflengthstr.string);
		  focus = 2;
		  break;
		}

	      parampointer->prelength3 = (helplong - 1) / 2;
	      parampointer->postlength3 = (helplong - 1) / 2;

	      i = sscanf (decimatestr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1)
		{
		  error_window ("A whole number, greater than 0, must \
be specified as decimation factor.");
		  decimatestr.cursorpos =
		    strlen (decimatestr.string);
		  focus = 3;
		  break;
		}

	      parampointer->int1 = helplong;

	      i = sscanf (thresholdstr.string, "%li", &helplong);
	      if (i < 1 || helplong < 1000)
		{
		  error_window ("A whole number, greater than 1000, must \
be specified as threshold.");
		  thresholdstr.cursorpos =
		    strlen (thresholdstr.string);
		  focus = 4;
		  break;
		}

	      parampointer->long1 = helplong;

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

      if (focus > 7)
	focus = 0;
      if (focus < 0)
	focus = 7;
    }
  while (dont_stop);

  free (medlengthstr.string);
  free (rmslengthstr.string);
  free (rmflengthstr.string);
  free (decimatestr.string);
  free (thresholdstr.string);
}

void
init_cond_median_filter (int filterno, parampointer_t parampointer)
{
  long total_post;
  long total_pre;

  total_post = parampointer->postlength1;
  if (parampointer->postlength2 > total_post)
    total_post = parampointer->postlength2;

  total_pre = parampointer->prelength1;
  if (parampointer->prelength2 +
      parampointer->prelength3 * parampointer->int1 + 1 > total_pre)
    total_pre = parampointer->prelength2 +
      parampointer->prelength3 * parampointer->int1 + 1;

  parampointer->buffer = init_buffer (total_post, total_pre);
  parampointer->buffer2 = init_buffer (parampointer->postlength2,
				       parampointer->prelength2);
  parampointer->buffer3 = init_buffer (parampointer->postlength3,
			     parampointer->prelength3 * parampointer->int1);

  parampointer->filterno = filterno;
}


void
delete_cond_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
  delete_buffer (&parampointer->buffer3);
}


sample_t
cond_median_highpass (long offset, long offset_zero,
		      parampointer_t parampointer)
{
  sample_t sample;
  longsample_t sum;

  offset += offset_zero;	/* middle for highpass filter in
				   'big buffer' */
  sum.left = 0;
  sum.right = 0;

#define notTEST_DAVE_PLATT
#ifndef TEST_DAVE_PLATT
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left += sample.left;
  sum.right += sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left -= 2 * (long) sample.left;
  sum.right -= 2 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left += sample.left;
  sum.right += sample.right;

  sum.left /= 4;
  sum.right /= 4;
#else
  /* Testing, suggested by Dave Platt. Invert phase of one channel, then
     do tick detection using the sum signal. This is because most ticks
     are out-of-phase signals. I've not really tested this - it might
     require other settings for thresholds etc. */
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left += sample.left;
  sum.left -= sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left -= 2 * (long) sample.left;
  sum.left += 2 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left += sample.left;
  sum.left -= sample.right;

  /* just in case L/R: 32000/-32000 -32000/32000 32000/-32000 : */
  sum.left /= 8;
  sum.right = sum.left;
#endif /* TEST_DAVE_PLATT */

  sample.left = sum.left;
  sample.right = sum.right;

  return sample;
}

fillfuncpointer_t cond_median_highpass_pointer = cond_median_highpass;

sample_t
cond_median_rms (long offset, long offset_zero,
		 parampointer_t parampointer)
{
  sample_t sample;
  doublesample_t doublesample;
  doublesample_t sum;
  long i;

  advance_current_pos_custom (&parampointer->buffer2,
			      cond_median_highpass_pointer,
			      offset + offset_zero,
			      parampointer);

  sum.left = 0;
  sum.right = 0;

  for (i = -parampointer->postlength2; i <= parampointer->prelength2;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer2, i);
      doublesample.left = sample.left;
      doublesample.right = sample.right;
      sum.left += doublesample.left * doublesample.left;
      sum.right += doublesample.right * doublesample.right;
    }

  sum.left /= (parampointer->postlength2 +
	       parampointer->prelength2 + 1);
  sum.right /= (parampointer->postlength2 +
		parampointer->prelength2 + 1);

  sample.left = sqrt (sum.left + 1);
  sample.right = sqrt (sum.right + 1);

  return sample;
}

fillfuncpointer_t cond_median_rms_pointer = cond_median_rms;

sample_t
cond_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  sample_t w_t;
  sample_t b_t;
  sample_t returnval;
  signed short list1[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  signed short list2[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  signed short list3[parampointer->postlength1 + parampointer->prelength1 + 1];
  long i, j;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer3,
			      cond_median_rms_pointer,
			      0,
			      parampointer);

  w_t = get_from_buffer (&parampointer->buffer3, 0);

  /* The RMF Filter */

  for (i = 0; i < parampointer->postlength3; i++)
    {
      sample = get_from_buffer (&parampointer->buffer3,
				i - parampointer->postlength3);
      list1[i] = sample.left;
      list2[i] = sample.right;
    }

  j = i;

  for (; i <= parampointer->postlength3 +
       parampointer->prelength3 * parampointer->int1;
       i += parampointer->int1)
    {
      sample = get_from_buffer (&parampointer->buffer3,
				i - parampointer->postlength3);
      list1[j] = sample.left;
      list2[j] = sample.right;
      j++;
    }

  b_t.left = median (list1, j);
  b_t.right = median (list2, j);

  put_in_buffer (&parampointer->buffer3, 0, b_t);

#ifdef VIEW_INTERNALS

  returnval.left = b_t.left * 10;

  if (
       (labs (w_t.left - b_t.left) * 1000)
       /
       b_t.left > parampointer->long1)
    returnval.right = 2000;
  else
    returnval.right = 0;

#else /* not VIEW_INTERNALS */

  returnval = get_from_buffer (&parampointer->buffer, 0);

  /* Median Filters - if necessary */

  if (
       (labs (w_t.left - b_t.left) * 1000)
       /
       b_t.left > parampointer->long1)
    {
      for (i = 0; i <= parampointer->postlength1 +
	   parampointer->prelength1; i++)
	list3[i] = get_from_buffer (&parampointer->buffer,
				    i - parampointer->postlength1).left;

      returnval.left = median (list3, parampointer->postlength1 +
			       parampointer->prelength1 + 1);
    }

  if (
       (labs (w_t.right - b_t.right) * 1000)
       /
       b_t.right > parampointer->long1)
    {
      for (i = 0; i <= parampointer->postlength1 +
	   parampointer->prelength1; i++)
	list3[i] = get_from_buffer (&parampointer->buffer,
				    i - parampointer->postlength1).right;

      returnval.right = median (list3, parampointer->postlength1 +
				parampointer->prelength1 + 1);
    }

#endif /* VIEW_INTERNALS */

  return returnval;
}
