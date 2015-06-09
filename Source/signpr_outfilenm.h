/* Signal Processing - Get outfile-name - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_OUTFILENM_H
#define HAVE_SIGNPR_OUTFILENM_H


#define SIGNPR_OUTFILE_HEADERTEXT "Signal Processing - Destination File"

int signproc_select_outfile (char *startdir, char *selectedfile);
/* Returns 0: canceled, 1: PrevScreen, 2: NextScreen */


#endif /* HAVE_SIGNPR_OUTFILENM_H */
