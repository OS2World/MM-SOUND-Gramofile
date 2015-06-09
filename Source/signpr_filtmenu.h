/* Signal Processing - Filter Menu - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_FILTMENU_H
#define HAVE_SIGNPR_FILTMENU_H


#include "scrollmenu.h"

#define SIGNPR_FILTMENU_HEADERTEXT  "Signal Processing - Filter Selection"

void add_to_filterlist (scrollmenu_t * filtlist, int *filtnumbers,
			char **helptexts, int filternumber, char *filtername,
			char *helptext);

void make_filterlist (scrollmenu_t * filtlist, int *filtnumbers,
		      char **helptexts);

int signproc_select_filters (scrollmenu_t * filtlist,
			     int *filtnumbers,
			     char **helptexts,
			     scrollmenu_t * selectedfilts,
			     int *usetracktimes, int *usebeginendtime,
			     double *begintime, double *endtime);


#endif /* HAVE_SIGNPR_FILTMENU_H */
