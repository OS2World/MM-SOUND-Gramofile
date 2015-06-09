/* Tracksplitting - Parameters - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_TRACKSPLIT_PARAMMENU_H
#define HAVE_TRACKSPLIT_PARAMMENU_H


#define TRACKSPLIT_PARAMMENU_HEADERTEXT "Track Location - Parameters"

int tracksplit_parammenu (int *make_use_rms, int *make_graphs, long *blocklen,
			  int *global_silence_factor,
			  int *local_silence_threshold,
			  int *min_silence_blocks,
			  int *min_track_blocks, int *extra_blocks_start,
			  int *extra_blocks_end);
/* Returns 0: canceled, 1: PrevScreen, 2: NextScreen */


#endif /* HAVE_TRACKSPLIT_PARAMMENU_H */
