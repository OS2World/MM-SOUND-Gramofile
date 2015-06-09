/* GramoFile - Main

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "mainmenu.h"
#include "splashscr.h"
#include <signal.h>
#include <unistd.h>
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
init_curses (void)
{
  initscr ();
  keypad (stdscr, TRUE);
  nonl ();
  cbreak ();
  noecho ();

  /* Xterms erase everything after the first refresh,
     so refresh one time before anything is added. */
  refresh ();

  return;
}

static void
finishmenu (int sig)
{
  endwin ();
  exit (0);
}

void
main (void)
{
  char startdir[250];
  char *helpcharptr;

  signal (SIGINT, finishmenu);

  init_curses ();

  splashscreen ();

  helpcharptr = getcwd (startdir, 250);
  if (helpcharptr == NULL)
    strcpy (startdir, "/");
  else
    strcat (startdir, "/");

  mainmenu (startdir);

  finishmenu (0);
}
