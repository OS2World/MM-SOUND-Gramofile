/* Error window - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_ERRORWINDOW_H
#define HAVE_ERRORWINDOW_H


#define ERROR_WINDOW_Y 9
#define ERROR_WINDOW_X 20
#define ERROR_WINDOW_H 5
#define ERROR_WINDOW_W 40

extern void error_window_display (char *text, char *buttontext);

extern void error_window (char *text);


#endif /* HAVE_ERRORWINDOW_H */
