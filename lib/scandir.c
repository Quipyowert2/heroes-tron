#include "config.h"
#include <stdlib.h>
#include <dirent.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif

/* From Mattias Engdegård <f91-men@nada.kth.se>. */
/* reimplementation of scandir, a BSDism */

int 
scandir (const char *dir, struct dirent ***namelist,
	 int (*select)(const struct dirent *),
	 int (*compar)(const struct dirent **, const struct dirent **))
{
  int n = 0, nalloc = 0;
  struct dirent *de, **list = NULL;
  DIR *d = opendir (dir);
  if (!d)
    return -1;
  while ((de = readdir (d)) != NULL) {
    if (select (de)) {
      if (n == nalloc)
	list = realloc (list, (nalloc += 8) * sizeof (struct dirent *));
#ifdef D_NAME_IS_POINTER
      list[n] = malloc (sizeof (struct dirent));
      memcpy (list[n], de, sizeof (struct dirent));
      list[n]->d_name = strdup (de->d_name);
#else
      list[n] = malloc (de->d_reclen);
      memcpy (list[n], de, de->d_reclen);
#endif
      n++;
    }
  }
  *namelist = list;
  if (list)
    qsort (list, n, sizeof (struct dirent *),
	   (int(*)(const void*,const void*))compar);
  return n;
}
