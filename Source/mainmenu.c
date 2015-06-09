/* Main menu

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/* Mouse support here only? Kinda confusing... So don't support mouse. */
#define DONT_USE_MOUSE

#include "mainmenu.h"
#include "reclp_main.h"
#include "textwindow.h"
#include "signpr_main.h"
#include "clrscr.h"
#include "helpline.h"
#include "errorwindow.h"
#include "playwav.h"
#include "tracksplit.h"
#include <stdlib.h>
#include <unistd.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

#ifdef __EMX__
#define usleep(a) _sleep2(a/1000)
#endif

void
displaymenu_onscreen (char **options, char **helptext,
		      int number, int selected,
		      int y, int x, int distance)
{				/* (y,x): upper left position */
  int i;

  for (i = 0; i < number; i++)
    {
      mvprintw (y + i * distance, x, "%d.", i + 1);

      if (i == selected)
	attron (A_STANDOUT);
      mvaddstr (y + i * distance, x + 3, options[i]);
      if (i == selected)
	attroff (A_STANDOUT);
    }

  mvaddstr (16, 1, options[selected]);

  display_textwin (helptext[selected], 17, 2, 5, 76);

  move (0, 79);

  refresh ();
}


int
displaymenu (char **options, char **helptext, int number, int preselected)
/* Returns:  -1   if Escape pressed,
   >=0  Enter or 1..9 */
{
  int i;
  int selected;
  int exitfunc = 0;
  int in_ch;

#ifdef NCURSES_MOUSE_VERSION
  MEVENT mouse_event;
#endif

#ifdef NCURSES_MOUSE_VERSION
#ifndef DONT_USE_MOUSE
  mousemask (ALL_MOUSE_EVENTS, NULL);
#endif
#endif

  selected = preselected;

  do
    {
      displaymenu_onscreen (options, helptext, number, selected,
			    DISPLAYMENU_Y,
			    DISPLAYMENU_X,
			    DISPLAYMENU_DISTANCE);

      in_ch = getch ();

#ifdef NCURSES_MOUSE_VERSION
      if (in_ch == KEY_MOUSE)
	{
	  getmouse (&mouse_event);

	  if (mouse_event.bstate == BUTTON1_CLICKED
	      ||
	      mouse_event.bstate == BUTTON1_DOUBLE_CLICKED)
	    {
	      for (i = 0; i < number; i++)
		if (mouse_event.y == DISPLAYMENU_Y +
		    i * DISPLAYMENU_DISTANCE
		    &&
		    mouse_event.x >= DISPLAYMENU_X
		    &&
		    mouse_event.x < DISPLAYMENU_X + 3 +
		    strlen (options[i])
		  )
		  {
		    displaymenu_onscreen (options, helptext,
					  number,
					  i,
					  DISPLAYMENU_Y,
					  DISPLAYMENU_X,
					  DISPLAYMENU_DISTANCE);
            usleep (100000);
		    selected = i;
		    exitfunc = TRUE;
		  }

	    }
	}
#endif /* NCURSES_MOUSE_VERSION */

      switch (in_ch)
	{
	case 'k':		/* Support some `less' keys. But only here... */
	case 'K':
	case KEY_UP:
	  selected--;
	  break;

	case 'j':
	case 'J':
	case KEY_DOWN:
	  selected++;
	  break;

	case 9:		/* TAB */
	  selected++;
	  if (selected >= number)
	    selected = 0;
	  break;

	case 13:		/* Enter/Return */
	case KEY_ENTER:
	  exitfunc = TRUE;
	  break;
	}

      if (in_ch >= '1' && in_ch <= '9')
	{
	  selected = in_ch - '1';
	  if (selected < number)
	    {
	      displaymenu_onscreen (options, helptext, number,
				    selected,
				    DISPLAYMENU_Y,
				    DISPLAYMENU_X,
				    DISPLAYMENU_DISTANCE);
	      usleep (100000);
	      exitfunc = TRUE;
	    }
	}

      if (selected < 0)
	selected = 0;
      if (selected >= number)
	selected = number - 1;

      if (in_ch == 27		/* Escape */
	  || in_ch == 'q' || in_ch == 'Q' || in_ch == '0')
	{
	  selected = -1;
	  exitfunc = TRUE;
	}
    }
  while (!exitfunc);

#ifdef NCURSES_MOUSE_VERSION
  mousemask (0, NULL);
#endif

  return selected;
}

void
mainmenu (char *startdir)
{
  int selected = 0;

#define MAINMENU_OPTIONS 6
  char *menu_options[MAINMENU_OPTIONS] =
  {
    "Record audio to a sound file           ",
    "[Copy sound from an audio CD to a file]",
    "Locate tracks                          ",
    "Process the audio signal               ",
    "[Write an audio CD]                    ",
    "Play a sound file                      "};

  char *menu_helptext[MAINMENU_OPTIONS]
  =
  {
    "With this option, audio from various sources (like gramophone records) \
can be recorded (sampled). The digital audio data is stored in a sound file \
(.wav format) on the harddisk.",

    "This option is not implemented yet. You can use the `cdda2wav' \
or `cdparanoia' program to copy digital audio from a CD to a sound file \
(.wav format) on the harddisk.",

    "The starts and ends of tracks in a large sound file can be \
detected automatically with this option. The `process signal' \
option uses the resulting .tracks file to actually split tracks.",

    "With this option, the digital audio from a sound file on the harddisk \
can be processed. For example, ticks may be filtered out. If track \
separation points are computed (previous option), separate audio files \
will be generated, each containing one track.",

    "This option is not implemented yet. You can use the `cdrecord' \
or `xcdroast' program to create CDs from sound files on the harddisk.",

    "(Parts of) sound files can be played with this option."
  };


  do
    {
      clearscreen ("Main Menu");
      helpline (
		 " Arrows/TAB: Navigate           Enter: Select option           0/Q/Escape: Exit ");

      selected = displaymenu (menu_options, menu_helptext,
			      MAINMENU_OPTIONS, selected);

      switch (selected)
	{
	case 0:
	  record_from_lp (startdir);
	  break;

	case 1:
	  error_window ("This option has not yet been implemented.");
	  break;

	case 2:
	  tracksplit_main (startdir);
	  break;

	case 3:
	  signproc_main (startdir);
	  break;

	case 4:
	  error_window ("This option has not yet been implemented.");
	  break;

	case 5:
	  playwav_main (startdir);
	  break;
	}
    }
  while (selected != -1);
}
