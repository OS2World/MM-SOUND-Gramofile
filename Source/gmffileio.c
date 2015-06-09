

THESE ROUTINES ARE NOT USED
YET (MAYBE NEVER...)


/* .GMF file I/O

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/*#include "gmffileio.h"
 */
#include <stdio.h>
#include <stdlib.h>

/* structure = better?
   typedef struct {
   FILE *file;
   char filename[250];
   ...?
   } gmffile_t;
 */

     FILE *create_gmf (char *filename)
/* Makes new .gmf file, writes comment-headers */
{
}

FILE *
open_gmf (char *filename)
/* Opens a .gmf file for in/output. Returns NULL if not successful */
{
  FILE *gmffile;

  gmffile = fopen (filename, "r");

  return gmffile;
}

int
close_gmf (FILE * gmffile)
{
  return fclose (gmffile);
}

int
read_line_from_gmf (FILE * gmffile, char **line)
/* Reads 1 line from gmf file, terminated by 10 or 13 or EOF.
   1: successful, 0: not (EOF). line will be malloc'ed.
   If successful, file pointer will be on start of next line */
{
  long oldpos;
  long numofchars = 0;
  int i;

  oldpos = ftell (gmffile);

  do
    {
      i = fgetc (gmffile);
      numofchars++;
    }
  while (i != 10 && i != 13 && i != EOF);

  /* numofchars is including 10, 13 or EOF, so always >= 1 */

  if (numofchars == 1 && i == EOF)
    return 0;			/* End of file */

  *line = (char *) malloc (numofchars * sizeof (char));

  if (fseek (gmffile, oldpos, SEEK_SET))
    return 0;

  for (i = 0; i < numofchars; i++)
    (*line)[i] = fgetc (gmffile);

  (*line)[numofchars - 1] = '\0';

  return 1;
}

char *
strip_spaces (char *instring)
/* Strips spaces from beginning and end. Return-string will be malloc'ed */
{
  char *outstring;
  char *startptr;
  char *endptr;
  char *tempcharptr;

  startptr = instring;

  while ((*startptr == ' ' || *startptr == 9)
	 && *startptr != '\0')
    startptr++;
  /* startptr now on real start of text */

  endptr = startptr;
  while (*endptr != '\0')
    endptr++;
  /* endptr now on last character */

  while ((*endptr == ' ' || *endptr == 9 || *endptr == '\0')
	 && endptr > startptr)
    endptr--;
  /* endptr now on real end of text */

  outstring = (char *) malloc (((endptr + 2) - startptr) * sizeof (char));
  outstring[(endptr + 1) - startptr] = '\0';

  tempcharptr = outstring;

  while (startptr <= endptr)
    {
      *tempcharptr = *startptr;
      tempcharptr++;
      startptr++;
    }

  return outstring;
}

void
del_comments (char *line)
/* strip_comments -> '\0' if starting with '#'. So `line' is modified. */
{
  if (line[0] = '#')
    line[0] = '\0';
}


int
search_identifier (FILE * gmffile, char *identifier)
/* Returns 1 if found, 0 if not found */
{
  rewind (gmffile);

}

int
read_simpval_from_gmf (FILE * gmffile, char *identifier, char *value)
/* Reads simple value; `value' will be malloc'ed.
   Returns 1: OK, 0: failure (not present, etc.) */
{
}

void
write_simpval_to_gmf (FILE * gmffile, char *identifier, char *value)
/* Writes simple value `identifier = value', discarding old value if present */
{
}

int
start_read_list_from_gmf (FILE * gmffile, char *identifier)
/* Start reading a list. Returns 1: OK, 0: failure (not present, etc.) */
{
}

int
read_listitem_from_gmf (FILE * gmffile, char *value)
/* Read one value from a list that has been started using
   start_read_list_from_gmf(). `value' will be malloc'ed.
   Returns 1: OK, 0: failure (no more items, etc.) */
{
}

void
start_write_list_to_gmf (FILE * gmffile, char *identifier)
/* Start writing a (new) list, discarding old contents if present */
{
}

void
write_listitem_to_gmf (FILE * gmffile, char *value)
/* Writes one item to list opened by start_write_list_to_gmf() */
{
}

void
stop_write_list_to_gmf (FILE * gmffile)
/* Closes writing of the list opened by start_write_list_to_gmf() */
{
}


main ()
{
  char *line = 0;
  char *line2 = 0;
  FILE *gmffile;

  gmffile = open_gmf ("/tmp/test.gmf");

  while (read_line_from_gmf (gmffile, &line))
    {
      printf ("%p>%s<\n", line, line);
      line2 = strip_spaces (line);
      printf ("%p>%s<\n", line2, line2);
      free (line);
      free (line2);
    }

  close_gmf (gmffile);

}
