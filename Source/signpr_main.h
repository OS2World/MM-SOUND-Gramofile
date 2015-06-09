/* Signal Processing Main - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPROC_MAIN_H
#define HAVE_SIGNPROC_MAIN_H


#include "scrollmenu.h"

#define SIGNPR_PROCESSING_HEADERTEXT "Signal Processing"

typedef struct
  {
    long begin;
    long end;
  }
beginendsample_t;

int signproc_get_options (char *startdir, char *infilename, char *outfilename,
			  scrollmenu_t * filtlist, int *filtnumbers,
			  char **helptexts, scrollmenu_t * selectedfilts,
			  int *usetracktimes, int *usebeginendtime,
			  double *begintime, double *endtime);

int load_track_times (char *filename, beginendsample_t * tracktimes,
		      int *number_of_tracks);

void signproc_main (char *startdir);


#endif /* HAVE_SIGNPROC_MAIN_H */
