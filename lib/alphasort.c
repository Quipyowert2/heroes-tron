#include "config.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#include <dirent.h>

/* From Mattias Engdegård <f91-men@nada.kth.se>. */
/* reimplementation of alphasort, a BSDism */

int 
alphasort (const struct dirent **a, const struct dirent **b)
{
  return strcmp ((*a)->d_name, (*b)->d_name);
}
