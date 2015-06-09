/* Splash Screen

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

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
splashscreen (void)
{
  char splashtext[] = "\
\n\
  #######                                            ####### ##   ###\n\
 ##                                                 ##             ##\n\
 ##        ## ####   #####   ## ### ###    #####    ##      ###    ##    #####\n\
 ##  ####   ##   ##      ##   ##  ##  ##  ##   ##   #####    ##    ##   ##   ##\n\
 ##     ##  ##       ######   ##  ##  ##  ##   ##   ##       ##    ##   ######\n\
 ##     ##  ##      ##   ##   ##  ##  ##  ##   ##   ##       ##    ##   ##\n\
  #######   ##       ######   ##  ##  ##   #####    ##      ####  ####   #####\n\
\n\
                                                                   Version 1.6\n\
________________________________________________________________________________     \n\
        Recording  -  Playback  -  Track Splitting  -  Signal Processing\n\
\n\
\n\
\n\
   Copyright (C) 1998 J.A. Bezemer\n\
\n\
   This program is free software; you are encouraged to redistribute it under\n\
   the terms of the GNU General Public License.\n\
\n\
   This program comes with ABSOLUTELY NO WARRANTY. See the GNU General Public\n\
   License (e.g. in the file named `COPYING') for more details.\
";

/* int i;
   for (i=0; i<strlen(splashtext); i++)
   addch(splashtext[i]=='#' ? ' ' | A_REVERSE : splashtext[i]);
 */

  clear ();
  refresh ();

  usleep (500000);

  addstr (splashtext);
  move (0, 79);
  refresh ();

  sleep (3);
}
