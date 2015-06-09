/* Scrolling Menus

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "scrollmenu.h"
#include <stdlib.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif


void
scrollmenu_display (scrollmenu_t * menu)
{
  int i, j, x1, y1;

  if (menu->selected >= menu->number)
    menu->selected = menu->number - 1;
  /* May get selected=-1 if number==0... */
  if (menu->selected < 0)	/* ...so this one must be last! */
    menu->selected = 0;
  if (menu->selected < menu->firstonscreen)
    menu->firstonscreen = menu->selected;
  if (menu->selected >= menu->firstonscreen + menu->h)
    menu->firstonscreen = menu->selected - menu->h + 1;

  for (i = 0; i < menu->h; i++)
    {
      if (menu->firstonscreen + i == menu->selected && menu->hasfocus)
	attron (A_STANDOUT);

      if (menu->firstonscreen + i < menu->number)
	mvaddnstr (menu->y + i,
		   menu->x,
		   menu->items[menu->firstonscreen + i],
		   menu->w);
      else
	move (menu->y + i, menu->x);

      getyx (stdscr, y1, x1);

      for (j = x1; j < menu->x + menu->w; j++)
	addch (' ');

      if (menu->firstonscreen + i == menu->selected && menu->hasfocus)
	attroff (A_STANDOUT);
    }

  move (menu->y + menu->selected - menu->firstonscreen, menu->x);
}

int
scrollmenu_stdkeys (int key, scrollmenu_t * menu)
/* Returns >0: item was selected;
   Returns -1 if nothing serious has happened.
 */

{
  int returnval = -1;

  switch (key)
    {
    case 'k':			/* Well, `less'-keys here also... */
    case 'K':
    case KEY_UP:
      (menu->selected)--;
      break;

    case 'j':
    case 'J':
    case KEY_DOWN:
      (menu->selected)++;
      break;

    case KEY_NPAGE:
      if (menu->selected < menu->firstonscreen +
	  menu->h - 1)
	menu->selected = menu->firstonscreen +
	  menu->h - 1;
      else
	menu->selected += menu->h - 1;
      break;

    case KEY_PPAGE:
      if (menu->selected > menu->firstonscreen)
	menu->selected = menu->firstonscreen;
      else
	menu->selected -= (menu->h - 1);
      break;

    case 13:
    case KEY_ENTER:
      returnval = menu->selected;
      break;
    }

  if (menu->selected < 0)
    menu->selected = 0;
  if (menu->selected >= menu->number)
    menu->selected = menu->number - 1;

  return returnval;
}

void
scrollmenu_delete_menu (scrollmenu_t * menu)
{
  int i;

  if (menu->items != NULL)
    {
      for (i = 0; i < menu->number; i++)
	free (menu->items[i]);

      free (menu->items);
      menu->items = NULL;
    }
}
