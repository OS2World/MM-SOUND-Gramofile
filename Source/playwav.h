/* Playing of a sound file - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_PLAYWAV_H
#define HAVE_PLAYWAV_H


#define PLAYWAV_HEADERTEXT "Play a sound file"

int playwav_select_file (char *startdir, char *selectedfile,
			 int *usebeginendtime, double *begintime,
			 double *endtime);
/* Returns 0: canceled, 1: OK */

void playwav_main (char *startdir);


#endif /* HAVE_PLAYWAV_H */
