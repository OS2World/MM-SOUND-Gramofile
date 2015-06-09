/* Record from LP

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "reclp_main.h"
#include "reclp_filenm.h"
#include "errorwindow.h"
#include <stdlib.h>
#include <stdio.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
record_from_lp (char *startdir)
{
  char filename[250];
  char shellcmd[500];

  if (!record_from_lp_get_filename (startdir, filename))
    return;

  def_prog_mode ();		/* save terminal state */
#ifdef __EMX__
  sprintf (shellcmd, "CLS & recwave.cmd %s",
           filename);
#else
  sprintf (shellcmd, "brec_gramo -S -s 44100 -b 16 -t 6000 -w %s",
           filename);
#endif
  system (shellcmd);

  reset_prog_mode ();		/* reset terminal state */
}
