/* Signal Processing - Filter Menu

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_filtmenu.h"
#include "signpr_general.h"
#include "buttons.h"
#include "boxes.h"
#include "helpline.h"
#include "clrscr.h"
#include "errorwindow.h"
#include "stringinput.h"
#include "secshms.h"
#include <stdlib.h>
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


int
signproc_select_filters (scrollmenu_t * filtlist,
			 int *filtnumbers,
			 char **helptexts,
			 scrollmenu_t * selectedfilts,
			 int *usetracktimes, int *usebeginendtime,
			 double *begintime, double *endtime)
/* Returns 0: canceled, 1: PrevScreen, 2: NextScreen */
{
  button_t next_button, cancel_button, prev_button;
  button_t beginend_check, tracktimes_check;
  stringinput_t begintimestring, endtimestring;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 0;
  int in_ch;
  int i;
  double tempdouble = 0;
  char tempstring[250];

  char *emptyhelpline =
  "                                                                TAB: Next field";

  int maxhelplength = 62;
  char helphelpline[100];

  char *helplines[9] =
  {
    " Enter: Add selected filter.                                    TAB: Next field",
    " Enter: Parameters   U/D: Move   R/Del: Delete                  TAB: Next field",
    " Use the .tracks file to split tracks.                          TAB: Next field",
    " Process only the specified part of the sound.                  TAB: Next field",
    " Enter begin time of part to be processed.                      TAB: Next field",
    " Enter end time of part to be processed.                        TAB: Next field",
    " To Signal Processing - Source File.                            TAB: Next field",
    " Back to main menu.                                             TAB: Next field",
    " To Signal Processing - Destination File.                       TAB: Next field"};

  char *helpcharptr;
  int helpint;
  parampointer_t helpparampointer;


  filtlist->y = 3;
  filtlist->x = 5;
  filtlist->h = 12;
  filtlist->w = 32;
  filtlist->firstonscreen = 0;
  filtlist->selected = 0;

  selectedfilts->y = 3;
  selectedfilts->x = 43;
  selectedfilts->h = 12;
  selectedfilts->w = 32;
  selectedfilts->firstonscreen = 0;
  selectedfilts->selected = 0;


  begintimestring.maxlen = 500;
  begintimestring.string = (char *) malloc (
				    begintimestring.maxlen * sizeof (char));
  fsec2hmsf (*begintime, begintimestring.string);
  begintimestring.y = 17;
  begintimestring.x = 59;
  begintimestring.w = 18;
  begintimestring.cursorpos = strlen (begintimestring.string);
  begintimestring.firstcharonscreen = 0;

  endtimestring.maxlen = 500;
  endtimestring.string = (char *) malloc (
				      endtimestring.maxlen * sizeof (char));
  fsec2hmsf (*endtime, endtimestring.string);
  endtimestring.y = 18;
  endtimestring.x = 59;
  endtimestring.w = 18;
  endtimestring.cursorpos = strlen (endtimestring.string);
  endtimestring.firstcharonscreen = 0;

  prev_button.text = " < Previous screen ";
  prev_button.y = 20;
  prev_button.x = 5;
  prev_button.selected = FALSE;

  next_button.text = " Next screen > ";
  next_button.y = 20;
  next_button.x = 60;
  next_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 36;
  cancel_button.selected = FALSE;

  tracktimes_check.text = "";	/* see below */
  tracktimes_check.y = 16;
  tracktimes_check.x = 4;
  tracktimes_check.selected = FALSE;

  beginend_check.text = "";	/* see below */
  beginend_check.y = 16;
  beginend_check.x = 42;
  beginend_check.selected = FALSE;

  clearscreen (SIGNPR_FILTMENU_HEADERTEXT);

  do
    {
      header (SIGNPR_FILTMENU_HEADERTEXT);

      if (*usetracktimes)
	tracktimes_check.text = "[X] Split tracks";
      else
	tracktimes_check.text = "[ ] Split tracks";

      if (*usebeginendtime)
	beginend_check.text = "[X] Use begin and end times";
      else
	beginend_check.text = "[ ] Use begin and end times";

      if (focus == 2)
	tracktimes_check.selected = TRUE;
      else
	tracktimes_check.selected = FALSE;

      if (focus == 3)
	beginend_check.selected = TRUE;
      else
	beginend_check.selected = FALSE;

      if (focus == 6)
	prev_button.selected = TRUE;
      else
	prev_button.selected = FALSE;

      if (focus == 7)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 8)
	next_button.selected = TRUE;
      else
	next_button.selected = FALSE;

      filtlist->hasfocus = (focus == 0);
      selectedfilts->hasfocus = (focus == 1);

      scrollmenu_display (filtlist);
      mybox (filtlist->y - 1, filtlist->x - 1,
	     filtlist->h + 2, filtlist->w + 2);
      mvprintw (filtlist->y - 1, filtlist->x + 1,
		"Available filters:");

      scrollmenu_display (selectedfilts);
      mybox (selectedfilts->y - 1, selectedfilts->x - 1,
	     selectedfilts->h + 2, selectedfilts->w + 2);
      mvprintw (selectedfilts->y - 1, selectedfilts->x + 1,
		"Selected filters:");

      stringinput_display (&begintimestring);
      mvprintw (begintimestring.y, begintimestring.x - 12,
		"Begin time:");

      stringinput_display (&endtimestring);
      mvprintw (endtimestring.y, endtimestring.x - 12,
		"End time  :");

      button_display (&prev_button);
      mybox (prev_button.y - 1, prev_button.x - 1,
	     3, strlen (prev_button.text) + 2);
      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&next_button);
      mybox (next_button.y - 1, next_button.x - 1,
	     3, strlen (next_button.text) + 2);

      button_display (&tracktimes_check);
      button_display (&beginend_check);

      if (focus == 0)
	{
	  strcpy (helphelpline, emptyhelpline);

	  i = 0;
	  while (i < strlen (helptexts[filtlist->selected])
		 && i < maxhelplength)
	    {
	      helphelpline[i + 1] = helptexts[filtlist->selected][i];
	      i++;
	    }

	  helpline (helphelpline);
	}
      else
	helpline (helplines[focus]);

      if (focus == 4)
	stringinput_display (&begintimestring);
      else if (focus == 5)
	stringinput_display (&endtimestring);
      else
	move (0, 79);

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* filtlist */
	  if (scrollmenu_stdkeys (in_ch, filtlist) >= 0)
	    {
	      if (selectedfilts->number < MAX_FILTERS)
		{
		  selectedfilts->items[number_of_filters] =
		    filtlist->items[filtlist->selected];
		  filter_type[number_of_filters] =
		    filtnumbers[filtlist->selected];
		  parampointerarray[number_of_filters] =
		    (parampointer_t) malloc (
					      sizeof (param_t));
		  param_defaults (
				   parampointerarray[number_of_filters],
				   filtnumbers[filtlist->selected]);
		  number_of_filters++;
		  selectedfilts->number = number_of_filters;
		  selectedfilts->selected = number_of_filters - 1;
		}
	      else
		error_window ("The maximum number of filters has been \
reached. No more filters can be added.");
	    }
	  else
	    switch (in_ch)
	      {
	      case KEY_LEFT:
		focus--;
		break;
	      case KEY_RIGHT:
		focus++;
		break;
	      }
	  break;

	case 1:		/* selectedfilts */
	  if (scrollmenu_stdkeys (in_ch, selectedfilts) >= 0)
	    {
	      if (number_of_filters > 0)
		;
	      param_screen (
			     parampointerarray[selectedfilts->selected],
			     filter_type[selectedfilts->selected]);
	    }
	  else
	    switch (in_ch)
	      {
	      case 'u':
	      case 'U':
		if (selectedfilts->selected > 0)
		  {
		    helpcharptr = selectedfilts->items[
					       selectedfilts->selected - 1];
		    selectedfilts->items[selectedfilts->selected - 1] =
		      selectedfilts->items[selectedfilts->selected];
		    selectedfilts->items[selectedfilts->selected] =
		      helpcharptr;

		    helpint = filter_type[selectedfilts->selected - 1];
		    filter_type[selectedfilts->selected - 1] =
		      filter_type[selectedfilts->selected];
		    filter_type[selectedfilts->selected] =
		      helpint;

		    helpparampointer = parampointerarray[
					       selectedfilts->selected - 1];
		    parampointerarray[selectedfilts->selected - 1] =
		      parampointerarray[selectedfilts->selected];
		    parampointerarray[selectedfilts->selected] =
		      helpparampointer;

		    selectedfilts->selected--;
		  }
		break;

	      case 'd':
	      case 'D':
		if (selectedfilts->selected <
		    number_of_filters - 1)
		  {
		    helpcharptr = selectedfilts->items[
					       selectedfilts->selected + 1];
		    selectedfilts->items[selectedfilts->selected + 1] =
		      selectedfilts->items[selectedfilts->selected];
		    selectedfilts->items[selectedfilts->selected] =
		      helpcharptr;

		    helpint = filter_type[selectedfilts->selected + 1];
		    filter_type[selectedfilts->selected + 1] =
		      filter_type[selectedfilts->selected];
		    filter_type[selectedfilts->selected] =
		      helpint;

		    helpparampointer = parampointerarray[
					       selectedfilts->selected + 1];
		    parampointerarray[selectedfilts->selected + 1] =
		      parampointerarray[selectedfilts->selected];
		    parampointerarray[selectedfilts->selected] =
		      helpparampointer;

		    selectedfilts->selected++;
		  }
		break;

	      case KEY_BACKSPACE:
	      case 127:
	      case 'r':
	      case 'R':
		if (number_of_filters > 0)
		  {
		    free (parampointerarray[selectedfilts->selected]);

		    for (i = selectedfilts->selected;
			 i < number_of_filters - 1; i++)
		      {
			selectedfilts->items[i] =
			  selectedfilts->items[i + 1];

			filter_type[i] = filter_type[i + 1];

			parampointerarray[i] =
			  parampointerarray[i + 1];
		      }
		    number_of_filters--;
		    selectedfilts->number = number_of_filters;
		  }
		break;

	      case KEY_LEFT:
		focus--;
		break;
	      case KEY_RIGHT:
		focus++;
		break;
	      }
	  break;

	case 2:		/* Use tracksplitting data */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	    case ' ':
	    case 'x':
	    case 'X':
	      *usetracktimes = 1 - *usetracktimes;
	      if (*usetracktimes)
		*usebeginendtime = 0;
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

	case 3:		/* Use begin/end times */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	    case ' ':
	    case 'x':
	    case 'X':
	      *usebeginendtime = 1 - *usebeginendtime;
	      if (*usebeginendtime)
		*usetracktimes = 0;
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

	case 4:		/* begin time */
	  stringinput_stdkeys (in_ch, &begintimestring);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      strcpy (tempstring, begintimestring.string);
	      if (!hmsf2fsec (tempstring, &tempdouble))
		{
		  error_window ("Enter the time in the format hours:\
minutes:seconds.fractions, for example 0:04:26.740");
		  begintimestring.cursorpos =
		    strlen (begintimestring.string);
		}
	      else
		{
		  *begintime = tempdouble;
		  fsec2hmsf (tempdouble, begintimestring.string);
		  begintimestring.cursorpos =
		    strlen (begintimestring.string);
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

	case 5:		/* end time */
	  stringinput_stdkeys (in_ch, &endtimestring);
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      strcpy (tempstring, endtimestring.string);
	      if (!hmsf2fsec (tempstring, &tempdouble))
		{
		  error_window ("Enter the time in the format hours:\
minutes:seconds.fractions, for example 0:04:26.740");
		  endtimestring.cursorpos =
		    strlen (endtimestring.string);
		}
	      else
		{
		  *endtime = tempdouble;
		  fsec2hmsf (tempdouble, endtimestring.string);
		  endtimestring.cursorpos =
		    strlen (endtimestring.string);
		  focus = 8;
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


	case 6:		/* < Previous */
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

	case 7:		/* Cancel */
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

	case 8:		/* Next > */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      if (number_of_filters < 1)
		{
		  error_window ("No filters have been selected.");
		  focus = 0;
		  break;
		}

	      strcpy (tempstring, begintimestring.string);
	      if (!hmsf2fsec (tempstring, &tempdouble))
		{
		  error_window ("Enter the begin time in the format hours:\
minutes:seconds.fractions, for example 0:04:26.740");
		  begintimestring.cursorpos =
		    strlen (begintimestring.string);
		  focus = 4;
		  break;
		}

	      *begintime = tempdouble;

	      strcpy (tempstring, endtimestring.string);
	      if (!hmsf2fsec (tempstring, &tempdouble))
		{
		  error_window ("Enter the end time in the format hours:\
minutes:seconds.fractions, for example 0:04:26.740");
		  endtimestring.cursorpos =
		    strlen (endtimestring.string);
		  focus = 5;
		  break;
		}

	      *endtime = tempdouble;

	      if (*begintime > *endtime)
		{
		  error_window ("The begin time is larger than the end time.");
		  fsec2hmsf (*begintime, begintimestring.string);
		  begintimestring.cursorpos =
		    strlen (begintimestring.string);
		  fsec2hmsf (*endtime, endtimestring.string);
		  endtimestring.cursorpos =
		    strlen (endtimestring.string);
		  focus = 4;
		  break;
		}

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

      if (focus > 8)
	focus -= 9;
      if (focus < 0)
	focus += 9;
    }
  while (dont_stop);

  return returnval;
}
