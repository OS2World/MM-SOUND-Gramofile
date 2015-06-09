/* Tracksplitting - Parameters

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "tracksplit_parammenu.h"
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


int
tracksplit_parammenu (int *make_use_rms, int *make_graphs, long *blocklen,
		      int *global_silence_factor,
		      int *local_silence_threshold, int *min_silence_blocks,
		      int *min_track_blocks, int *extra_blocks_start,
		      int *extra_blocks_end)
/* Returns 0: canceled, 1: PrevScreen, 2: NextScreen */
{
  button_t next_button, cancel_button, prev_button;
  button_t make_use_rms_check, make_graphs_check;
  stringinput_t blocklen_string, global_silence_factor_string, local_silence_threshold_string,
    min_silence_blocks_string, min_track_blocks_string, extra_blocks_start_string,
    extra_blocks_end_string;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 11;
  int in_ch;
  long templong;

  char *helplines[12] =
  {
    " Save results or use saved results of lengthy computation.      TAB: Next field",
    " For manual (double-)checking/adjustment. Needs a few MB's.     TAB: Next field",
    " Number of samples to compute RMS of; 4410 = 0.1 sec.           TAB: Next field",
    " The relative threshold during the initial search for tracks.   TAB: Next field",
    " Begin/end of track ...% above local (silence) power level.     TAB: Next field",
    " Shorter periods of silence not treated as track separation.    TAB: Next field",
    " Shorter tracks are ignored.                                    TAB: Next field",
    " Correction for fade-in effects that are hard to detect.        TAB: Next field",
    " Correction for fade-out effects that are hard to detect.       TAB: Next field",
    " To Track Splitting - File Name.                                TAB: Next field",
    " Back to main menu.                                             TAB: Next field",
    " Start searching for tracks.                                    TAB: Next field"};

  make_use_rms_check.text = "";	/* see below */
  make_use_rms_check.y = 4;
  make_use_rms_check.x = 2;
  make_use_rms_check.selected = FALSE;

  make_graphs_check.text = "";	/* see below */
  make_graphs_check.y = 6;
  make_graphs_check.x = 2;
  make_graphs_check.selected = FALSE;

  blocklen_string.maxlen = 500;
  blocklen_string.string = (char *) malloc (
				    blocklen_string.maxlen * sizeof (char));
  sprintf (blocklen_string.string, "%ld", *blocklen);
  blocklen_string.y = 8;
  blocklen_string.x = 54;
  blocklen_string.w = 12;
  blocklen_string.cursorpos =
    strlen (blocklen_string.string);
  blocklen_string.firstcharonscreen = 0;

  global_silence_factor_string.maxlen = 500;
  global_silence_factor_string.string = (char *) malloc (
		       global_silence_factor_string.maxlen * sizeof (char));
  sprintf (global_silence_factor_string.string, "%d",
	   *global_silence_factor);
  global_silence_factor_string.y = 10;
  global_silence_factor_string.x = 54;
  global_silence_factor_string.w = 12;
  global_silence_factor_string.cursorpos =
    strlen (global_silence_factor_string.string);
  global_silence_factor_string.firstcharonscreen = 0;

  local_silence_threshold_string.maxlen = 500;
  local_silence_threshold_string.string = (char *) malloc (
		     local_silence_threshold_string.maxlen * sizeof (char));
  sprintf (local_silence_threshold_string.string, "%d",
	   *local_silence_threshold);
  local_silence_threshold_string.y = 11;
  local_silence_threshold_string.x = 54;
  local_silence_threshold_string.w = 12;
  local_silence_threshold_string.cursorpos =
    strlen (local_silence_threshold_string.string);
  local_silence_threshold_string.firstcharonscreen = 0;

  min_silence_blocks_string.maxlen = 500;
  min_silence_blocks_string.string = (char *) malloc (
			  min_silence_blocks_string.maxlen * sizeof (char));
  sprintf (min_silence_blocks_string.string, "%d",
	   *min_silence_blocks);
  min_silence_blocks_string.y = 13;
  min_silence_blocks_string.x = 54;
  min_silence_blocks_string.w = 12;
  min_silence_blocks_string.cursorpos =
    strlen (min_silence_blocks_string.string);
  min_silence_blocks_string.firstcharonscreen = 0;

  min_track_blocks_string.maxlen = 500;
  min_track_blocks_string.string = (char *) malloc (
			    min_track_blocks_string.maxlen * sizeof (char));
  sprintf (min_track_blocks_string.string, "%d",
	   *min_track_blocks);
  min_track_blocks_string.y = 14;
  min_track_blocks_string.x = 54;
  min_track_blocks_string.w = 12;
  min_track_blocks_string.cursorpos =
    strlen (min_track_blocks_string.string);
  min_track_blocks_string.firstcharonscreen = 0;

  extra_blocks_start_string.maxlen = 500;
  extra_blocks_start_string.string = (char *) malloc (
			  extra_blocks_start_string.maxlen * sizeof (char));
  sprintf (extra_blocks_start_string.string, "%d",
	   *extra_blocks_start);
  extra_blocks_start_string.y = 16;
  extra_blocks_start_string.x = 54;
  extra_blocks_start_string.w = 12;
  extra_blocks_start_string.cursorpos =
    strlen (extra_blocks_start_string.string);
  extra_blocks_start_string.firstcharonscreen = 0;

  extra_blocks_end_string.maxlen = 500;
  extra_blocks_end_string.string = (char *) malloc (
			    extra_blocks_end_string.maxlen * sizeof (char));
  sprintf (extra_blocks_end_string.string, "%d",
	   *extra_blocks_end);
  extra_blocks_end_string.y = 17;
  extra_blocks_end_string.x = 54;
  extra_blocks_end_string.w = 12;
  extra_blocks_end_string.cursorpos =
    strlen (extra_blocks_end_string.string);
  extra_blocks_end_string.firstcharonscreen = 0;

  prev_button.text = " < Previous screen ";
  prev_button.y = 20;
  prev_button.x = 5;
  prev_button.selected = FALSE;

  next_button.text = " Start computation ";
  next_button.y = 20;
  next_button.x = 56;
  next_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 36;
  cancel_button.selected = FALSE;

  clearscreen (TRACKSPLIT_PARAMMENU_HEADERTEXT);

  do
    {
      header (TRACKSPLIT_PARAMMENU_HEADERTEXT);

      if (*make_use_rms)
	make_use_rms_check.text = "[X]";
      else
	make_use_rms_check.text = "[ ]";

      if (*make_graphs)
	make_graphs_check.text = "[X]";
      else
	make_graphs_check.text = "[ ]";

      if (focus == 0)
	make_use_rms_check.selected = TRUE;
      else
	make_use_rms_check.selected = FALSE;

      if (focus == 1)
	make_graphs_check.selected = TRUE;
      else
	make_graphs_check.selected = FALSE;

      if (focus == 9)
	prev_button.selected = TRUE;
      else
	prev_button.selected = FALSE;

      if (focus == 10)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 11)
	next_button.selected = TRUE;
      else
	next_button.selected = FALSE;

      stringinput_display (&blocklen_string);
      mvprintw (blocklen_string.y, 2,
		"Length of blocks of signal power data (samples)   :");

      stringinput_display (&global_silence_factor_string);
      mvprintw (global_silence_factor_string.y, 2,
		"Global silence factor (0.1 %)                     :");

      stringinput_display (&local_silence_threshold_string);
      mvprintw (local_silence_threshold_string.y, 2,
		"Local silence factor (%)                          :");

      stringinput_display (&min_silence_blocks_string);
      mvprintw (min_silence_blocks_string.y, 2,
		"Minimal length of inter-track silence (blocks)    :");

      stringinput_display (&min_track_blocks_string);
      mvprintw (min_track_blocks_string.y, 2,
		"Minimal length of tracks (blocks)                 :");

      stringinput_display (&extra_blocks_start_string);
      mvprintw (extra_blocks_start_string.y, 2,
		"Number of extra blocks at track start             :");

      stringinput_display (&extra_blocks_end_string);
      mvprintw (extra_blocks_end_string.y, 2,
		"Number of extra blocks at track end               :");

      button_display (&prev_button);
      mybox (prev_button.y - 1, prev_button.x - 1,
	     3, strlen (prev_button.text) + 2);
      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&next_button);
      mybox (next_button.y - 1, next_button.x - 1,
	     3, strlen (next_button.text) + 2);

      button_display (&make_use_rms_check);
      mvprintw (make_use_rms_check.y, make_use_rms_check.x + 4,
		"Save/load signal power (RMS) data to/from .rms file");

      button_display (&make_graphs_check);
      mvprintw (make_graphs_check.y, make_graphs_check.x + 4,
		"Generate graph files");

      helpline (helplines[focus]);

      switch (focus)
	{
	case 2:
	  stringinput_display (&blocklen_string);
	  break;
	case 3:
	  stringinput_display (&global_silence_factor_string);
	  break;
	case 4:
	  stringinput_display (&local_silence_threshold_string);
	  break;
	case 5:
	  stringinput_display (&min_silence_blocks_string);
	  break;
	case 6:
	  stringinput_display (&min_track_blocks_string);
	  break;
	case 7:
	  stringinput_display (&extra_blocks_start_string);
	  break;
	case 8:
	  stringinput_display (&extra_blocks_end_string);
	  break;
	default:
	  move (0, 79);
	}

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* Make/use .RMS file */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	    case ' ':
	    case 'x':
	    case 'X':
	      *make_use_rms = 1 - *make_use_rms;
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

	case 1:		/* Make graph files */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	    case ' ':
	    case 'x':
	    case 'X':
	      *make_graphs = 1 - *make_graphs;
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

	case 2:		/* Block length */
	  stringinput_stdkeys (in_ch, &blocklen_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (blocklen_string.string);
	      if (templong < 1)
		{
		  error_window ("Enter a whole number, greater than 0. \
Default: 4410.");
		  blocklen_string.cursorpos =
		    strlen (blocklen_string.string);
		}
	      else
		{
		  *blocklen = templong;
		  sprintf (blocklen_string.string,
			   "%ld", templong);
		  blocklen_string.cursorpos =
		    strlen (blocklen_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 3:		/* Global silence factor */
	  stringinput_stdkeys (in_ch,
			       &global_silence_factor_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				global_silence_factor_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number, preferably \
less than 1000. Default: 150.");
		  global_silence_factor_string.cursorpos =
		    strlen (
			     global_silence_factor_string.string);
		}
	      else
		{
		  *global_silence_factor = templong;
		  sprintf (global_silence_factor_string.string,
			   "%ld", templong);
		  global_silence_factor_string.cursorpos =
		    strlen (global_silence_factor_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 4:		/* Local silence threshold */
	  stringinput_stdkeys (in_ch,
			       &local_silence_threshold_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				local_silence_threshold_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number. \
Default: 5.");
		  local_silence_threshold_string.cursorpos =
		    strlen (
			     local_silence_threshold_string.string);
		}
	      else
		{
		  *local_silence_threshold = templong;
		  sprintf (local_silence_threshold_string.string,
			   "%ld", templong);
		  local_silence_threshold_string.cursorpos =
		    strlen (
			     local_silence_threshold_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 5:		/* Min. silence length (blocks) */
	  stringinput_stdkeys (in_ch,
			       &min_silence_blocks_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				min_silence_blocks_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number. \
Default: 20.");
		  min_silence_blocks_string.cursorpos =
		    strlen (
			     min_silence_blocks_string.string);
		}
	      else
		{
		  *min_silence_blocks = templong;
		  sprintf (min_silence_blocks_string.string,
			   "%ld", templong);
		  min_silence_blocks_string.cursorpos =
		    strlen (
			     min_silence_blocks_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 6:		/* Min. track length (blocks) */
	  stringinput_stdkeys (in_ch,
			       &min_track_blocks_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				min_track_blocks_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number. \
Default: 50.");
		  min_track_blocks_string.cursorpos =
		    strlen (
			     min_track_blocks_string.string);
		}
	      else
		{
		  *min_track_blocks = templong;
		  sprintf (min_track_blocks_string.string,
			   "%ld", templong);
		  min_track_blocks_string.cursorpos =
		    strlen (
			     min_track_blocks_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 7:		/* Extra blocks at track start */
	  stringinput_stdkeys (in_ch,
			       &extra_blocks_start_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				extra_blocks_start_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number. \
Default: 3.");
		  extra_blocks_start_string.cursorpos =
		    strlen (
			     extra_blocks_start_string.string);
		}
	      else
		{
		  *extra_blocks_start = templong;
		  sprintf (extra_blocks_start_string.string,
			   "%ld", templong);
		  extra_blocks_start_string.cursorpos =
		    strlen (
			     extra_blocks_start_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 8:		/* Extra blocks at track end */
	  stringinput_stdkeys (in_ch,
			       &extra_blocks_end_string);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      templong = atol (
				extra_blocks_end_string.string);
	      if (templong < 0 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number. \
Default: 6.");
		  extra_blocks_end_string.cursorpos =
		    strlen (
			     extra_blocks_end_string.string);
		}
	      else
		{
		  *extra_blocks_end = templong;
		  sprintf (extra_blocks_end_string.string,
			   "%ld", templong);
		  extra_blocks_end_string.cursorpos =
		    strlen (
			     extra_blocks_end_string.string);
		  focus++;
		}
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }
	  break;

	case 9:		/* < Previous */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      returnval = 1;
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

	case 10:		/* Cancel */
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

	case 11:		/* Next > */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      templong = atol (blocklen_string.string);
	      if (templong < 1)
		{
		  error_window ("Enter a whole number, greater than 0, as \
block length. Default: 4410.");
		  blocklen_string.cursorpos =
		    strlen (blocklen_string.string);
		  focus = 2;
		  break;
		}
	      *blocklen = templong;

	      templong = atol (global_silence_factor_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
global silence factor. Default: 150.");
		  global_silence_factor_string.cursorpos =
		    strlen (global_silence_factor_string.string);
		  focus = 3;
		  break;
		}
	      *global_silence_factor = templong;

	      templong = atol (local_silence_threshold_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
local silence factor. Default: 5.");
		  local_silence_threshold_string.cursorpos =
		    strlen (local_silence_threshold_string.string);
		  focus = 4;
		  break;
		}
	      *local_silence_threshold = templong;

	      templong = atol (min_silence_blocks_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
minimal silence duration. Default: 20.");
		  min_silence_blocks_string.cursorpos =
		    strlen (min_silence_blocks_string.string);
		  focus = 5;
		  break;
		}
	      *min_silence_blocks = templong;

	      templong = atol (min_track_blocks_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
minimal track length. Default: 50.");
		  min_track_blocks_string.cursorpos =
		    strlen (min_track_blocks_string.string);
		  focus = 6;
		  break;
		}
	      *min_track_blocks = templong;

	      templong = atol (extra_blocks_start_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
block addition. Default: 3.");
		  extra_blocks_start_string.cursorpos =
		    strlen (extra_blocks_start_string.string);
		  focus = 7;
		  break;
		}
	      *extra_blocks_start = templong;

	      templong = atol (extra_blocks_end_string.string);
	      if (templong < 1 || templong > 10000)
		{
		  error_window ("Enter a whole, positive number as \
block addition. Default: 6.");
		  extra_blocks_end_string.cursorpos =
		    strlen (extra_blocks_end_string.string);
		  focus = 8;
		  break;
		}
	      *extra_blocks_end = templong;

	      returnval = 2;
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

	}			/* switch(focus) */

      if (in_ch == 9)		/* TAB */
	focus++;

      if (in_ch == 27)
	dont_stop = FALSE;

      if (focus > 11)
	focus = 0;
      if (focus < 0)
	focus = 11;
    }
  while (dont_stop);

  return returnval;
}
