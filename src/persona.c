/*------------------------------------------------------------------.
| Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License as published by |
| the Free Software Foundation; either version 2 of the License, or |
| (at your option) any later version.                               |
|                                                                   |
| Heroes is distributed in the hope that it will be useful, but     |
| WITHOUT ANY WARRANTY; without even the implied warranty of        |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU |
| General Public License for more details.                          |
|                                                                   |
| You should have received a copy of the GNU General Public License |
| along with this program; if not, write to the Free Software       |
| Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          |
| 02111-1307 USA                                                    |
`------------------------------------------------------------------*/

#include "system.h"
#include "persona.h"
#include "debugmsg.h"
#include "rsc_files.h"
#include "fopenlock.h"
#include "vars.h"

static uid_t sys_uid;
static gid_t sys_gid;
static uid_t user_uid;
static gid_t user_gid;

static void
print_persona (void)
{
  dmsg (D_SYSTEM, "Current persona: EUID=%d EGID=%d", geteuid (), getegid ());
}

void
init_persona (void)
{
  sys_uid = geteuid ();
  sys_gid = getegid ();
  user_uid = getuid ();
  user_gid = getgid ();
  print_persona ();
  user_persona ();

  /* Define some variable to indicate the special permission,
     so we can use conditionnals in the config files.  */
  if (sys_uid != user_uid || sys_gid != sys_uid) {
    var_define ("suid-or-sgid", "true");
    var_define (sys_uid != user_uid ? "suid" : "sgid", "true");
  }
}

void
user_persona (void)
{
  dmsg (D_SYSTEM, "switching to the user persona");
#if _POSIX_SAVED_IDS && HAVE_SETEGID
  seteuid (user_uid);
  setegid (user_gid);
#else
  setreuid (user_uid, sys_uid);
  setregid (user_gid, sys_gid);
#endif
  print_persona ();
}

void
sys_persona (void)
{
  dmsg (D_SYSTEM, "switching to the system persona");
#if _POSIX_SAVED_IDS && HAVE_SETEGID
  seteuid (sys_uid);
  setegid (sys_gid);
#else
  setreuid (sys_uid, user_uid);
  setregid (sys_gid, user_gid);
#endif
  print_persona ();
}

static char *
sys_persona_if_needed (const char *rsc, const char *mode)
{
  char *sysdir;
  char *file;
  int syslen;
  bool sec = true;

  file = get_non_null_rsc_file_secure (rsc, &sec);

  /* If the file is open for reading only, we don't care
     about switching the persona.  */
  if (mode[0] == 'r' && mode[1] != '+')
    return file;

  sysdir = get_non_null_rsc_file_secure ("sys-dir", &sec);

  /* Don't even consider changing the persona if the
     resource hasn't been setup in a secure way.  */
  if (sec) {
    /* Switch the the system personal if the file is located
       beyond sysdir.  */
    syslen = strlen (sysdir);
    if (strncmp (file, sysdir, syslen) == 0)
      sys_persona ();
  }

  free (sysdir);
  return file;
}

FILE *
persona_fopenlock (const char *rsc, const char *mode)
{
  char *n = sys_persona_if_needed (rsc, mode);
  FILE *f = fopenlock (n, mode);
  free (n);
  return f;
}
