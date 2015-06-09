/* Track Splitting - Get file-name

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "tracksplit_filenm.h"
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

#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


int
tracksplit_select_file (char *startdir, char *selectedfile)
/* Returns 0: canceled, 2: NextScreen */
{
  scrollmenu_t dirfilelist;
  stringinput_t string;
  button_t next_button, cancel_button;
  int dont_stop = TRUE;
  int returnval = 0;
  int focus = 1;
  int in_ch;
  int i;
  char helpstring[500];
  char *charpointer;
  struct stat filestats;
  int oldselected;

  char *helplines[4] =
  {
    " Select name of sound file for track splitting.                 TAB: Next field",
    " Enter name of sound file for track splitting.                  TAB: Next field",
    " Back to main menu.                                             TAB: Next field",
    " To Track Splitting - Parameters.                               TAB: Next field"};

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

  next_button.text = " Next screen > ";
  next_button.y = 20;
  next_button.x = 60;
  next_button.selected = FALSE;

  cancel_button.text = " Cancel ";
  cancel_button.y = 20;
  cancel_button.x = 36;
  cancel_button.selected = FALSE;

  clearscreen (TRACKSPLIT_FILE_HEADERTEXT);

  do
    {
      header (TRACKSPLIT_FILE_HEADERTEXT);

      if (focus == 2)
	cancel_button.selected = TRUE;
      else
	cancel_button.selected = FALSE;

      if (focus == 3)
	next_button.selected = TRUE;
      else
	next_button.selected = FALSE;

      dirfilelist.hasfocus = (focus == 0);

      scrollmenu_display (&dirfilelist);
      mybox (dirfilelist.y - 1, dirfilelist.x - 1,
	     dirfilelist.h + 2, dirfilelist.w + 2);
      mvprintw (dirfilelist.y - 1, dirfilelist.x + 1,
		"Files and directories:");

      stringinput_display (&string);
      mybox (string.y - 1, string.x - 1, 3, string.w + 2);
      mvprintw (string.y - 1, string.x + 1, "File name:");

      button_display (&cancel_button);
      mybox (cancel_button.y - 1, cancel_button.x - 1,
	     3, strlen (cancel_button.text) + 2);
      button_display (&next_button);
      mybox (next_button.y - 1, next_button.x - 1,
	     3, strlen (next_button.text) + 2);

      helpline (helplines[focus]);

      if (focus == 1)
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
		  focus = 1;
		  string.cursorpos = strlen (string.string);
		  string.firstcharonscreen = 0;
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

	case 1:		/* string */
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
	      else		/* it's a file */
		focus = 3;
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

	case 2:		/* Cancel */
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

	case 3:		/* Next > */
	  if (in_ch == KEY_ENTER || in_ch == 13)
	    switch (checkfile (string.string))
	      {
	      case FILE_EXISTS:
		strcpy (selectedfile, string.string);
		returnval = 2;
		dont_stop = FALSE;
		break;

	      case DIR_EXISTS:
		error_window ("The specified name is of a directory. A \
file name must be specified.");
		string.cursorpos = strlen (string.string);
		focus = 1;
		break;

	      case DIR_OK_NEW_FILE:
	      case DIR_WRONG:
		error_window ("The specified file does not exist.");
		string.cursorpos = strlen (string.string);
		focus = 1;
		break;

	      default:
		error_window ("Fell out of switch, tracksplit_filenm #1");
		break;
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

      if (focus > 3)
	focus = 0;
      if (focus < 0)
	focus = 3;
    }
  while (dont_stop);

  scrollmenu_delete_menu (&dirfilelist);
  free (string.string);

  return returnval;
}
