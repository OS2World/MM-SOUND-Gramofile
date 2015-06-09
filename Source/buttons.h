/* Buttons - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_BUTTONS_H
#define HAVE_BUTTONS_H


typedef struct
  {
    char *text;
    int y;
    int x;
    int selected;
  }
button_t;

extern void button_display (button_t * button);


#endif /* HAVE_BUTTONS_H */
