/* Text in a 'window' - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_TEXTWINDOW_H
#define HAVE_TEXTWINDOW_H


#define DISPLAYMENU_MAXTEXTLEN   800

extern void display_textwin (char *text, int y, int x, int h, int w);
			/* (y,x): upper left position of 'text block'
			   h = heigth, w = width of block */


#endif /* HAVE_TEXTWINDOW_H */
