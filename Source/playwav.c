/* Playing a sound file

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "playwav.h"
#include "scrollmenu.h"
#include "stringinput.h"
#include "buttons.h"
#include "boxes.h"
#include "dirfilemenu.h"
#include "errorwindow.h"
#include "textwindow.h"
#include "checkfile.h"
#include "yesnowindow.h"
#include "helpline.h"
#include "clrscr.h"
#include "secshms.h"
#include "fmtheaders.h"
#include "signpr_main.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
playwav_playit (char *filename, int usebeginendtime, double begintime,
		double endtime)
{
  char shellcmd[500];

  switch (checkfile (filename))
    {
    case FILE_EXISTS:
#ifndef __EMX__
      if (usebeginendtime)
        sprintf (shellcmd, "bplay_gramo -S -s 44100 -b 16 -J %ld -T %ld %s",
                 (long) (begintime * 44100),
                 (long) ((endtime - begintime) * 44100), filename);
      else
        sprintf (shellcmd, "bplay_gramo -S -s 44100 -b 16 %s", filename);
      /* defaults for raw files (but no -r, so .wav's supply their own
         parameters) - you can even listen to executables in CD quality (: */
#else
      if (usebeginendtime)
        sprintf (shellcmd, "playwave.cmd  %ld %ld %s",
                 (long) (begintime),
                 (long) ((endtime - begintime)), filename);
      else
        sprintf (shellcmd, "playwave.cmd  -1 -1 %s", filename);
#endif

      
      def_prog_mode ();		/* save terminal state */
      
      system (shellcmd);

      reset_prog_mode ();	/* reset terminal state */
      clear ();

      break;

    case DIR_EXISTS:
      error_window ("The specified name is of a directory. A \
file name must be specified.");
      break;

    case DIR_OK_NEW_FILE:
    case DIR_WRONG:
      error_window ("The specified file does not exist.");
      break;

    default:
      error_window ("Fell out of switch, playwav #2");
      break;
    }
}

#define PLAY_TRACK	1
#define PLAY_BEGINNING	2
#define PLAY_END	3
#define PLAY_BEFORE	4
#define PLAY_AFTER	5

/* These may be fractional, like 5.2, if you like that better. */
#define BEGINNING_SECS	5
#define END_SECS	4
#define BEFORE_SECS	3
#define AFTER_SECS	3

void
playwav_track (char *filename, int track, int action)
{
  beginendsample_t tracktimes[100];
  int number_of_tracks;
  double begintime, endtime;
  char helpstring[200];

  if (!load_track_times (filename, tracktimes, &number_of_tracks))
    return;

  if (track > number_of_tracks || track < 1)
    {
      sprintf (helpstring, "There are only %d tracks.", number_of_tracks);
      error_window (helpstring);
      return;
    }

  switch (action)
    {
    case PLAY_TRACK:
      begintime = tracktimes[track].begin;
      endtime = tracktimes[track].end;
      /* use samples here, convert to seconds after switch */
      break;

    case PLAY_BEGINNING:
      begintime = tracktimes[track].begin;
      endtime = tracktimes[track].begin + BEGINNING_SECS * 44100;
      break;

    case PLAY_END:
      begintime = tracktimes[track].end - END_SECS * 44100;
      endtime = tracktimes[track].end;
      break;

    case PLAY_BEFORE:
      begintime = tracktimes[track].begin - BEFORE_SECS * 44100;
      endtime = tracktimes[track].begin;
      break;

    case PLAY_AFTER:
      begintime = tracktimes[track].end;
      endtime = tracktimes[track].end + AFTER_SECS * 44100;
      break;

    default:
      error_window ("Fell out of switch, playwav #3");
      return;
    }

  if (begintime < 0)
    begintime = 0;
  if (endtime < 0)
    endtime = 0;
  if (begintime >= endtime)
    return;

  /* samples -> seconds here */
  playwav_playit (filename, 1, begintime / 44100, endtime / 44100);
}


int
playwav_select_file (char *startdir, char *selectedfile,
		     int *usebeginendtime, double *begintime,
		     double *endtime)
/* Returns 0: canceled, 1: OK */
{
  scrollmenu_t dirfilelist;
  stringinput_t string;
  button_t ok_button, cancel_button;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus;
  int in_ch;
  int i;
  char helpstring[500];
  char *charpointer;
  struct stat filestats;
  int oldselected;
  double tempdouble = 0;
  long templong;
  char tempstring[250];
  stringinput_t trackstring, begintimestring, endtimestring;
  button_t beginend_check;
  struct stat buf;

  char *helplines[8] =
  {
    " Select name of sound file to be played.                        TAB: Next field",
    " Enter: Play track    B/E: Beginning/End    F/A: beFore/After    +/-: Prev/Next",
    " Play only a part of the sound file.                            TAB: Next field",
    " Enter begin time of part to be played.                         TAB: Next field",
    " Enter end time of part to be played.                           TAB: Next field",
    " Enter name of sound file to be played.                         TAB: Next field",
    " Back to main menu.                                             TAB: Next field",
    " Play the specified (part of the) sound file.                   TAB: Next field"};

  dirfilelist.y = 3;
  dirfilelist.x = 5;
  dirfilelist.h = 12;
  dirfilelist.w = 32;
  dirfilelist.firstonscreen = 0;
  dirfilemenu (startdir, &dirfilelist);
  dirfilelist.selected = dirfilelist.last_of_1st_part + 1;

  string.maxlen = 500;
  string.string = (char *) malloc (string.maxlen * sizeof (char));
  if (selectedfile[0] == '\0')
    strcpy (string.string, startdir);
  else
    strcpy (string.string, selectedfile);
  string.y = 17;
  string.x = 5;
  string.w = 70;
  string.cursorpos = strlen (string.string);
  string.firstcharonscreen = strlen (string.string) - string.w + 2;
  if (string.firstcharonscreen < 0)
    string.firstcharonscreen = 0;

  ok_button.text = " Play ";
  ok_button.y = 20;
  ok_button.x = 69;
  ok_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 5;
  cancel_button.selected = FALSE;

  trackstring.maxlen = 500;
  trackstring.string = (char *) malloc (
					 trackstring.maxlen * sizeof (char));
  strcpy (trackstring.string, "1");
  trackstring.y = 11;
  trackstring.x = 54;
  trackstring.w = 10;
  trackstring.cursorpos = strlen (trackstring.string);
  trackstring.firstcharonscreen = 0;

  begintimestring.maxlen = 500;
  begintimestring.string = (char *) malloc (
				    begintimestring.maxlen * sizeof (char));
  fsec2hmsf (*begintime, begintimestring.string);
  begintimestring.y = 14;
  begintimestring.x = 59;
  begintimestring.w = 18;
  begintimestring.cursorpos = strlen (begintimestring.string);
  begintimestring.firstcharonscreen = 0;

  endtimestring.maxlen = 500;
  endtimestring.string = (char *) malloc (
				      endtimestring.maxlen * sizeof (char));
  fsec2hmsf (*endtime, endtimestring.string);
  endtimestring.y = 15;
  endtimestring.x = 59;
  endtimestring.w = 18;
  endtimestring.cursorpos = strlen (endtimestring.string);
  endtimestring.firstcharonscreen = 0;

  beginend_check.text = "";	/* see below */
  beginend_check.y = 13;
  beginend_check.x = 42;
  beginend_check.selected = FALSE;

  clearscreen (PLAYWAV_HEADERTEXT);

  if (selectedfile[0] == '\0')
    focus = 0;
  else
    focus = 5;

  do
    {
      if (*usebeginendtime)
	beginend_check.text = "[X] Use begin and end times";
      else
	beginend_check.text = "[ ] Use begin and end times";

      if (focus == 2)
	beginend_check.selected = TRUE;
      else
	beginend_check.selected = FALSE;

      if (focus == 6)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 7)
	ok_button.selected = TRUE;
      else
	ok_button.selected = FALSE;

      dirfilelist.hasfocus = (focus == 0);

      scrollmenu_display (&dirfilelist);
      mybox (dirfilelist.y - 1, dirfilelist.x - 1,
	     dirfilelist.h + 2, dirfilelist.w + 2);
      mvprintw (dirfilelist.y - 1, dirfilelist.x + 1,
		"Files and directories:");

      button_display (&beginend_check);

      stringinput_display (&trackstring);
      mvprintw (trackstring.y, trackstring.x - 7,
		"Track:");

      stringinput_display (&begintimestring);
      mvprintw (begintimestring.y, begintimestring.x - 12,
		"Begin time:");

      stringinput_display (&endtimestring);
      mvprintw (endtimestring.y, endtimestring.x - 12,
		"End time  :");

      stringinput_display (&string);
      mybox (string.y - 1, string.x - 1, 3, string.w + 2);
      mvprintw (string.y - 1, string.x + 1, "File name:");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&ok_button);
      mybox (ok_button.y - 1, ok_button.x - 1,
	     3, strlen (ok_button.text) + 2);

      header (PLAYWAV_HEADERTEXT);
      helpline (helplines[focus]);

      /* this really should be a switch... */
      if (focus == 1)
	stringinput_display (&trackstring);
      else if (focus == 3)
	stringinput_display (&begintimestring);
      else if (focus == 4)
	stringinput_display (&endtimestring);
      else if (focus == 5)
	stringinput_display (&string);
      else
	move (0, 79);

      refresh ();

      in_ch = getch ();

      switch (focus)
	{
	case 0:		/* dirfilelist */
	  if (scrollmenu_stdkeys (in_ch, &dirfilelist) >= 0)
	    {
	      oldselected = dirfilelist.selected;
	      i = dirfilemenu_process_select (&dirfilelist,
					      helpstring);
	      if (i == 0)	/* filename in helpstring */
		{
		  strcpy (string.string, helpstring);
		  focus = 7;
		  string.cursorpos = strlen (string.string);
		  string.firstcharonscreen = 0;

		  if (!stat (helpstring, &buf))
		    {
		      *begintime = 0;
		      if (buf.st_size < sizeof (wavhead))
			*endtime = 0;
		      else
			*endtime = (buf.st_size - sizeof (wavhead))
			  / (2 * 2 * 44100.);

		      fsec2hmsf (*begintime, begintimestring.string);
		      begintimestring.cursorpos =
			strlen (begintimestring.string);
		      begintimestring.firstcharonscreen = 0;

		      fsec2hmsf (*endtime, endtimestring.string);
		      endtimestring.cursorpos = strlen (endtimestring.string);
		      endtimestring.firstcharonscreen = 0;
		    }
		}
	      else
		/* dir in helpstring */
		{
		  scrollmenu_delete_menu (&dirfilelist);
		  dirfilemenu (helpstring, &dirfilelist);
		  if (dirfilelist.number == 0)
		    {
		      error_window (
				   "No permission to read this directory.");
		      scrollmenu_delete_menu (&dirfilelist);
		      dirfilemenu (startdir, &dirfilelist);
		      dirfilelist.selected = oldselected;
		    }
		  else
		    {
		      strcpy (startdir, helpstring);
		      dirfilelist.firstonscreen = 0;

		      charpointer = strrchr (string.string, '/');
		      if (charpointer != NULL)
			strcat (helpstring, charpointer + 1);
		      else
			strcat (helpstring, string.string);
		      strcpy (string.string, helpstring);
		    }
		}
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

	case 1:		/* tracks */
	  /* exclude keys like b,e,f,a,+,- */
	  if ((in_ch >= '0' && in_ch <= '9') || in_ch > 127)
	    stringinput_stdkeys (in_ch, &trackstring);

	  templong = atol (trackstring.string);

	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	      playwav_track (string.string, templong, PLAY_TRACK);
	      break;

	    case 'b':
	    case 'B':
	      playwav_track (string.string, templong, PLAY_BEGINNING);
	      break;

	    case 'e':
	    case 'E':
	      playwav_track (string.string, templong, PLAY_END);
	      break;

	    case 'f':
	    case 'F':
	      playwav_track (string.string, templong, PLAY_BEFORE);
	      break;

	    case 'a':
	    case 'A':
	      playwav_track (string.string, templong, PLAY_AFTER);
	      break;

	    case '+':
	    case '=':
	      if (templong < 99)
		templong++;
	      sprintf (trackstring.string, "%ld", templong);
	      trackstring.cursorpos = strlen (trackstring.string);
	      trackstring.firstcharonscreen = 0;
	      break;

	    case '-':
	    case '_':
	      if (templong > 1)
		templong--;
	      sprintf (trackstring.string, "%ld", templong);
	      trackstring.cursorpos = strlen (trackstring.string);
	      trackstring.firstcharonscreen = 0;
	      break;

	    case KEY_UP:
	      focus--;
	      break;
	    case KEY_DOWN:
	      focus++;
	      break;
	    }

	  break;

	case 2:		/* Use begin/end times */
	  switch (in_ch)
	    {
	    case KEY_ENTER:
	    case 13:
	    case ' ':
	    case 'x':
	    case 'X':
	      *usebeginendtime = 1 - *usebeginendtime;
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

	case 3:		/* begin time */
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

	case 4:		/* end time */
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
		  focus = 5;
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


	case 5:		/* string */
	  stringinput_stdkeys (in_ch, &string);
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      strcpy (helpstring, string.string);

	      /* cut away last '/'-s */
	      while (strlen (helpstring) > 0 &&
		     helpstring[strlen (helpstring) - 1] == '/')
		helpstring[strlen (helpstring) - 1] = '\0';

	      strcat (helpstring, "/");

	      if (!stat (helpstring, &filestats) &&
		  S_ISDIR (filestats.st_mode))
		{
		  strcpy (startdir, helpstring);
		  scrollmenu_delete_menu (&dirfilelist);
		  dirfilemenu (startdir, &dirfilelist);
		  dirfilelist.firstonscreen = 0;
		  dirfilelist.selected =
		    dirfilelist.last_of_1st_part + 1;
		  strcpy (string.string, startdir);
		  string.cursorpos = strlen (string.string);
		  focus = 0;
		}
	      else
		/* it's a file */
		{
		  focus = 7;

		  if (!stat (string.string, &buf))
		    {
		      *begintime = 0;
		      if (buf.st_size < sizeof (wavhead))
			*endtime = 0;
		      else
			*endtime = (buf.st_size - sizeof (wavhead))
			  / (2 * 2 * 44100.);

		      fsec2hmsf (*begintime, begintimestring.string);
		      begintimestring.cursorpos =
			strlen (begintimestring.string);
		      begintimestring.firstcharonscreen = 0;

		      fsec2hmsf (*endtime, endtimestring.string);
		      endtimestring.cursorpos = strlen (endtimestring.string);
		      endtimestring.firstcharonscreen = 0;
		    }
		}
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

	case 6:		/* Cancel */
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

	case 7:		/* OK */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    {
	      strcpy (tempstring, begintimestring.string);
	      if (!hmsf2fsec (tempstring, &tempdouble))
		{
		  error_window ("Enter the begin time in the format hours:\
minutes:seconds.fractions, for example 0:04:26.740");
		  begintimestring.cursorpos =
		    strlen (begintimestring.string);
		  focus = 3;
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
		  focus = 4;
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
		  focus = 3;
		  break;
		}

	      switch (checkfile (string.string))
		{
		case FILE_EXISTS:
		  /* strcpy (selectedfile, string.string);
		     returnval = 1;
		     dont_stop = FALSE; */

		  playwav_playit (string.string, *usebeginendtime, *begintime,
				  *endtime);
		  break;

		case DIR_EXISTS:
		  error_window ("The specified name is of a directory. A \
file name must be specified.");
		  string.cursorpos = strlen (string.string);
		  focus = 5;
		  break;

		case DIR_OK_NEW_FILE:
		case DIR_WRONG:
		  error_window ("The specified file does not exist.");
		  string.cursorpos = strlen (string.string);
		  focus = 5;
		  break;

		default:
		  error_window ("Fell out of switch, playwav #1");
		  break;
		}

	    }			/* if ENTER */
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

      if (focus > 7)
	focus = 0;
      if (focus < 0)
	focus = 7;
    }
  while (dont_stop);

  scrollmenu_delete_menu (&dirfilelist);
  free (string.string);

  return returnval;
}

void
playwav_main (char *startdir)
{
  char filename[250];
/*  char shellcmd[500]; */
  int usebeginendtime = 0;
  double begintime = 0, endtime = 0;

  filename[0] = '\0';

  while (1)
    {
      if (!playwav_select_file (startdir, filename, &usebeginendtime,
				&begintime, &endtime))
	return;

#if 0
/* not used any more - integrated in select_file */
      def_prog_mode ();		/* save terminal state */

      if (usebeginendtime)
	sprintf (shellcmd, "bplay_gramo -S -s 44100 -b 16 -J %ld -T %ld %s",
		 (long) (begintime * 44100),
		 (long) ((endtime - begintime) * 44100), filename);
      else
	sprintf (shellcmd, "bplay_gramo -S -s 44100 -b 16 %s", filename);
      /* defaults for raw files (but no -r, so .wav's supply their own
         parameters */

      system (shellcmd);

      reset_prog_mode ();	/* reset terminal state */
#endif /* 0 */

    }
}
