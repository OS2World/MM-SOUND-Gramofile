/* Yes/No window - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_YESNOWINDOW_H
#define HAVE_YESNOWINDOW_H


#define YESNO_WINDOW_Y 9
#define YESNO_WINDOW_X 20
#define YESNO_WINDOW_H 5
#define YESNO_WINDOW_W 40

extern int yesno_window (char *text, char *yestext, char *notext,
			 int preselected);
/* preselected = 1: yes-button selected, 0: no-button selected.
   returns 1 if yes-button selected,
   0 if no-button selected or Escape pressed */


#endif /* HAVE_YESNOWINDOW_H */
