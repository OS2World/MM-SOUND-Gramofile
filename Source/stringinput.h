/* String input - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_STRINGINPUT_H
#define HAVE_STRINGINPUT_H


#define TREAT_DEL_AS_BACKSPACE


typedef struct
  {
    char *string;
    int y;
    int x;
    int w;
    int maxlen;
    int cursorpos;
    int firstcharonscreen;
  }
stringinput_t;

extern void stringinput_display (stringinput_t * data);

extern void stringinput_stdkeys (int key, stringinput_t * data);


#endif /* HAVE_STRINGINPUT_H */
