/* Main menu - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_MAINMENU_H
#define HAVE_MAINMENU_H


#define DISPLAYMENU_Y		3
#define DISPLAYMENU_X		5
#define DISPLAYMENU_DISTANCE	2


void displaymenu_onscreen (char **options, char **helptext,
			   int number, int selected,
			   int y, int x, int distance);
				/* (y,x): upper left position */

int displaymenu (char **options, char **helptext, int number, int preselected);

void mainmenu (char *startdir);


#endif /* HAVE_MAINMENU_H */
