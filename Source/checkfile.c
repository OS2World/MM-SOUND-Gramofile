/* Check file

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */


#include "checkfile.h"
#include <sys/stat.h>
#include <string.h>


/* ---- From GNU shellutils, lib/stripslash.h: */

/* Remove trailing slashes from PATH.
   This is useful when using filename completion from a shell that
   adds a "/" after directory names (such as tcsh and bash), because
   the Unix rename and rmdir system calls return an "Invalid argument" error
   when given a path that ends in "/" (except for the root directory).  */

void
strip_trailing_slashes (char *path)
{
  int last;

  last = strlen (path) - 1;
  while (last > 0 && path[last] == '/')
    path[last--] = '\0';
}
/* ---- End */

int
checkfile (char *filename)
{
  struct stat buf;
  int i;
  char myfilename[250];
  char *slash;

  strip_trailing_slashes (filename);

  i = stat (filename, &buf);

  if (!i)			/* file or dir found */
    {
      if (S_ISDIR (buf.st_mode))
	{
	  if (strcmp (filename, "/"))
	    strcat (filename, "/");
	  return DIR_EXISTS;
	}
      else
	return FILE_EXISTS;
    }
  else
    /* file does not exist, check dir */
    {
      strcpy (myfilename, filename);

/* ---- Adapted from GNU shellutils, src/dirname.c: */

      slash = strrchr (myfilename, '/');
      if (slash == NULL)
	strcpy (myfilename, ".");
      else
	{
	  /* Remove any trailing slashes and final element. */
	  while (slash > myfilename && *slash == '/')
	    --slash;
	  slash[1] = 0;
	}

/* ---- End */

      i = stat (myfilename, &buf);
      if (!i)			/* dir found */
	{
	  if (S_ISDIR (buf.st_mode))
	    return DIR_OK_NEW_FILE;

	  return DIR_WRONG;
	}
      else			/* dir does not exist */
	return DIR_WRONG;

    }				/* else: file not found */
}
