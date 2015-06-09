/* Check file - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_CHECKFILE_H
#define HAVE_CHECKFILE_H


#define FILE_EXISTS		1	/* The string contains the name 
					   of an existing file */
#define DIR_EXISTS		2	/* The string contains the name
					   of an existing directory */
#define DIR_OK_NEW_FILE		3	/* The string contains the name
					   of a new file, in an already
					   existing directory */
#define DIR_WRONG		4	/* The string contains the name
					   of a file with a non-existing
					   directory-path */

extern void strip_trailing_slashes (char *path);

extern int checkfile (char *filename);


#endif /* HAVE_CHECKFILE_H */
