/* Dir/file menus - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_DIRFILEMENU_H
#define HAVE_DIRFILEMENU_H


#include "scrollmenu.h"

extern void dirfilemenu (char *basedir, scrollmenu_t * menu);
		/* NOTE: basedir must contain last '/', like "/home/" */

extern int dirfilemenu_process_select (scrollmenu_t * menu, char *dirfile);
		/* Returns: 0 if file, then complete filename in dirfile
		   1 if dir, then new complete dirname in dirfile */


#endif /* HAVE_DIRFILEMENU_H */
