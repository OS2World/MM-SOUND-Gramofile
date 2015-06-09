/* Dir/file menus

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "dirfilemenu.h"
#include "scrollmenu.h"
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

int alphasort(struct dirent **d1, struct dirent **d2);

void
dirfilemenu (char *basedir, scrollmenu_t * menu)
/* NOTE: basedir must contain last '/', like "/home/" */
{
  struct dirent **namelist;
  int n;
  char mybasedir[250];
  char helpstring[250];
  char *firstslash;
  int i, j, k;
  int indent = 0;
  int isadir;			/* 0 if no dir, 1 if dir. */
  struct stat filestats;

  n = scandir (basedir, &namelist, NULL, alphasort);

  if (n < 0)
    menu->number = 0;
  else
    {
      menu->items = (char **) malloc ((n + 100) * sizeof (char *));

      for (i = 0; i < n + 100; i++)
	menu->items[i] = NULL;

      strcpy (mybasedir, basedir);
      i = 0;

      do
	{
	  firstslash = strchr (mybasedir, '/');
	  if (firstslash != NULL)
	    {
	      menu->items[i] =
		(char *) malloc ((firstslash - mybasedir + 2 + indent) *
				 sizeof (char));
	      for (k = 0; k < indent; k++)
		menu->items[i][k] = ' ';

	      strncpy (menu->items[i] + indent, mybasedir,
		       firstslash - mybasedir + 1);
	      menu->items[i][firstslash - mybasedir + 1 + indent] = '\0';

	      strcpy (helpstring, firstslash + 1);
	      strcpy (mybasedir, helpstring);

	      i++;
	      indent++;
	    }
	}
      while (strlen (mybasedir) > 0);

      menu->last_of_1st_part = i - 1;

      for (j = 0; j < n; j++)
	{
	  if (strcmp (namelist[j]->d_name, ".") &&
	      strcmp (namelist[j]->d_name, ".."))
	    {
	      strcpy (helpstring, basedir);
	      strcat (helpstring, namelist[j]->d_name);
	      stat (helpstring, &filestats);

	      if (S_ISDIR (filestats.st_mode))
		isadir = 1;
	      else
		isadir = 0;

	      menu->items[i] =
		(char *) malloc (
			(strlen (namelist[j]->d_name) + 1 + indent + isadir)
				  * sizeof (char));

	      for (k = 0; k < indent; k++)
		menu->items[i][k] = ' ';

	      strcpy (menu->items[i] + indent, namelist[j]->d_name);

	      if (isadir)
		strcat (menu->items[i], "/");

	      i++;
	    }
	}
      menu->number = i;
    }
}

int
dirfilemenu_process_select (scrollmenu_t * menu, char *dirfile)
/* Returns: 0 if file, then complete filename in dirfile
   1 if dir, then new complete dirname in dirfile */
{
  int i;

  if (menu->selected <= menu->last_of_1st_part)
    /* parent dir */
    {
      dirfile[0] = '\0';

      for (i = 0; i <= menu->selected; i++)
	strcat (dirfile, menu->items[i] + i);

      menu->selected++;
      return 1;
    }
  else
    /* this dir */
    {

      dirfile[0] = '\0';

      for (i = 0; i <= menu->last_of_1st_part; i++)
	strcat (dirfile, menu->items[i] + i);

      strcat (dirfile, menu->items[menu->selected]
	      + menu->last_of_1st_part + 1);

      if (menu->items[menu->selected]
	  [strlen (menu->items[menu->selected]) - 1] == '/')
	{			/* subdir */
	  menu->selected = menu->last_of_1st_part + 2;
	  return 1;
	}
      else			/* file */
	return 0;
    }
}
