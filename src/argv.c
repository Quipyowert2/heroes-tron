/*------------------------------------------------------------------------.
| Copyright (C) 2000 Alexandre Duret-Lutz <duret_g@epita.fr>              |
|                                                                         |
| This file is part of Heroes.                                            |
|                                                                         |
| Heroes is free software; you can redistribute it and/or modify it under |
| the terms of the GNU General Public License as published by the Free    |
| Software Foundation; either version 2 of the License, or (at your       |
| option) any later version.                                              |
|                                                                         |
| Heroes is distributed in the hope that it will be useful, but WITHOUT   |
| ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   |
| FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   |
| for more details.                                                       |
|                                                                         |
| You should have received a copy of the GNU General Public License along |
| with this program; if not, write to the Free Software Foundation, Inc., |
| 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   |
`------------------------------------------------------------------------*/

#include "common.h"
#include "const.h"
#include "argv.h"
#include "sound.h"
#include "display.h"
#include "misc.h"
#include "debugmsg.h"
#include "errors.h"
#include "rsc_files_hash.h"

int snap = 0;
int cpuon = 1;
int nosfx = 0;
int joyoff = 0;
int devparm = 0;
int loadulevel = 0;
int directmenu = 0;
int reinitsco = 0;
int reinitopt = 0;
int reinitsav = 0;
int x10sav = 0;
int doublefx = 1;
int swapside = 1;
char* level_name;
int mono = 0;
int bits8 = 0;
int hqmix = 0;
int stretch = 1;
int nosound = 0;
int even_lines = 0;

static void
version (void)
{
  puts ("Heroes " VERSION "\n");
  puts ("Copyright (c) 1997,1998,2000 "
	"Alexandre Duret-Lutz, Romuald Genevois,\n"
	"Alexandre Liverneaux and Philippe Meisburger.\n");
  puts ("This is free software; see the source for copying conditions.  "
	"There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS"
	" FOR A PARTICULAR PURPOSE.");
}

static void
list (char *word)
{
  if (!word) {
    puts ("Use `-lWORD' or `--list=WORD' where WORD can be:\n"
	  "  resources\t\tprint the resources list\n"
	  "  sound-drivers\t\tprint the sound driver lists");
    return;
  }
  if (!strcasecmp (word,"resources") ||
      !strcasecmp (word,"rsc")) {
    print_rsc_files ();
  } else if (!strcasecmp (word,"sound-drivers") ||
	     !strcasecmp (word,"sd")) {
    print_drivers_list ();
  } else {
    /* Unknown WORD, print usage. */
    list (0);
  }
}

static void
print_help (char* argv0)
{
  printf ("Usage: %s [OPTIONS]...\n\n",argv0);
  puts ("Heroes is a game like nibbles but different.\n"
	"\nGeneral options:\n"
	"      --version\t\t"	    "    display version number\n"
	"  -h, --help\t\t"	    "    display this help\n"
	"  -q, --quiet\t\t"	    "    don't print warning messages\n"
	"  -Q, --really-quiet\t"    "    don't even print error messages\n"
	"  -v, --verbose=OPTIONS\t" "    enable debugging messages\n"
	"  -l, --list=WORD\t"	    
                           "    show some internal information; WORD can be\n"
	"\t\t\t"		    "      `resources' or `sound-drivers'.\n"
	"\nSound options:\n"
	"  -d, --driver=N[,OPTIONS]" 
	                  "  use Nth driver for sound output (0: autodetect)\n"
	"  -S, --no-sound\t"        "    disable sound\n"
	"  -X, --no-sfx\t\t"        "    disable sound-effects\n"
	"  -m, --mono\t\t"          "    non-stereo output\n"
	"  -8, --8bits\t\t"	    "    8bits sound output\n"
	"  -i, --high-quality\t"    "    high quality mixer\n"
	"\nDisplay options:\n"	
	"  -G, --gfx-options=OPTIONS" 
                                    " options to give to the display driver\n"
	"  -F, --full-screen\t"     "    full screen mode\n"
	"  -2, --double\t\t"        "    stretch the display twofold\n"
	"  -3, --triple\t\t"        "    stretch the display threefold\n"
	"  -e, --even-lines\t"      "    display only even-lines\n"
	"\nMiscellaneous options:\n"
	"      --cpu-off\t\t"	    "    disable computer opponents\n"
	"      --default-scores\t"  "    restore default scores file\n"
	"      --default-options\t" "    restore default options file\n"
	"      --default-saves\t"   "    restore default saves file\n"
	"  -s, --swap-sides\t"      "    swap sides in two player mode\n"
	"      --no-double-fx\t"    
                           "    disable superposition of rotozoom and waves\n"
	"  -g, --go\t\t"            "    skip the introduction\n"
	"  -J, --no-joystick\t"     "    disable joystick handling\n"
	"\n"
	"These options can be set in your file ~/.heroes/heroesrc (which is "
        "read\nbefore parsing other command line options) using a line like "
	"this:\n\n"
	"  Options: -gs -d3,buffer=11,count=4\n"
	"\n"
	"Report bugs to <heroes-bugs@lists.sourceforge.net>.");
}

const struct option long_options[] = {
  {"8bits",		no_argument,       NULL,	'8'},
  {"cpu-off",		no_argument,       &cpuon,	0},
  {"default-options",	no_argument,       &reinitopt,	1},
  {"default-saves",	no_argument,       &reinitsav,	1},
  {"default-scores",	no_argument,       &reinitsco,	1},
  {"devparm",		no_argument,       &devparm,	1},
  {"double",		no_argument,       NULL,	'2'},
  {"driver",		required_argument, NULL,	'd'},
  {"drivers-info",	no_argument,       NULL,	'n'},
  {"even-lines",	no_argument,       NULL,	'e'},
  {"full-screen",	no_argument,	   NULL,	'F'},
  {"gfx-options",	required_argument, NULL,	'G'},
  {"go",		no_argument,       NULL,	'g'},
  {"help",		no_argument,       NULL,	'h'},
  {"high-quality",	no_argument,       NULL,	'i'},
  {"list",		optional_argument, NULL,	'l'},
  {"load",		required_argument, NULL,	'L'},
  {"mono",		no_argument,       NULL,	'm'},
  {"no-double-fx",	no_argument,       &doublefx,	0},
  {"no-joystick",	no_argument,       &joyoff,	'J'},
  {"no-sfx",		no_argument,       NULL,	'X'},
  {"no-sound",		no_argument,       NULL,	'S'},
  {"quiet",		no_argument,	   NULL,	'q'},
  {"really-quiet",	no_argument,	   NULL,	'Q'},
  {"snap",		no_argument,       &snap,	0},
  {"swap-sides",	no_argument,       NULL,	's'},
  {"triple",		no_argument,       NULL,	'3'},
  {"verbose",		required_argument, NULL,	'v'},
  {"version",		no_argument,       NULL,	'v'},
  {"x10-saves",		no_argument,       &x10sav,	1},
  {NULL,		0,		   NULL,	0}
};

int
parse_argv (int argc, char **argv)
{
  int c;

  /* Reset optind so that getopt is reinitialized. */
  optind = 0;

  for (;;) {
    int option_index = 0;

    c = getopt_long (argc, argv, "238d:eFgG:hiJl::L:mnqQsSv::X", 
		     long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case 'v':
      if (!optarg) {
	version ();
	return 1;
      } else {
	dmsg_parse_string (optarg);
	break;
      }
    case 'h':
      print_help (argv[0]);
      return 1;
    case 's':
      swapside = 0;
      break;
    case 'm':
      mono = 1;
      break;
    case '8':
      bits8 = 1;
      break;
    case 'a':
      hqmix = 1;
      break;
    case 'X':
      nosfx = 1;
      break;
    case 'l':
      list (optarg);
      return 1;
    case 'L':
      level_name = strdup (optarg);
      level_name = strappend (level_name, ".lvl");
      loadulevel = 1;
      break;
    case 'g':
      directmenu = 1;
      break;
    case 'n':
      wmsg ("-n is an obsolete option, you should use --list=sound-drivers");
      print_drivers_list ();
      return 1;
    case 'd': 
      decode_sound_options (optarg, argv[0]);
      break;
    case 'G':
      set_display_params (optarg);
      break;
    case 'F':
      set_full_screen_mode ();
      break;
    case 'J':
      joyoff = 1;
      break;
    case '2':
      stretch = 2;
      break;
    case '3':
      stretch = 3;
      break;
    case 'e':
      even_lines = 1;
      break;
    case 'i':
      hqmix = 1;
      break;
    case 'S':
      nosound = 1;
      break;
    case '?':
      /* getopt_long already printed an error message. */
    case 0:
      break;
    case 'Q':
      disable_emsg = 1;
      /* no break: 'Q' imply 'q' */
    case 'q':
      disable_wmsg = 1;
      break;

    default:
      abort ();
    }
  }
  return 0;
}
