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
#include "getopt.h"
#include "const.h"
#include "argv.h"
#include "sound.h"
#include "display.h"
#include "misc.h"
#include "debugmsg.h"

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
version ()
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
print_help (char* argv0)
{
  printf ("Usage: %s [OPTIONS]...\n\n",argv0);
  puts ("Heroes is a game like nibbles but different.\n\n"
	"  -v, --version\t\t"	    "    display version number\n"
	"  -h, --help\t\t"	    "    display this help\n"
	"\nSound options:\n"
	"  -n, --drivers-info\t"    "    print the sound drivers list\n"
	"  -d, --driver=N[,OPTIONS]" 
	                       "  use Nth driver for output (0: autodetect)\n"
	"  -S, --no-sound\t\t"      "    disable sound\n"
	"  -X, --no-sfx\t\t"        "    disable sound-effects\n"
	"  -m, --mono\t\t"          "    non-stereo output\n"
	"  -8, --8bits\t\t"	    "    8bits sound output\n"
	"  -q, --high-quality\t"    "    high quality mixer\n"
	"\nDisplay options:\n"	
	"  -G, --gfx-options=OPTIONS" 
                                    " options to give to the display driver\n"
	"  -F, --full-screen\t"     "    full screen mode\n"
	"  -2, --double\t\t"        "    stretch the display twofold\n"
	"  -3, --triple\t\t"        "    stretch the display threefold\n"
	"  -e, --even-lines\t\t"    "    display only even-lines\n"
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
  {"version",		0, 0,		'v'},
  {"help",		0, 0,		'h'},
  {"cpu-off",		0, &cpuon,	0},
  {"default-scores",	0, &reinitsco,	1},
  {"default-options",	0, &reinitopt,	1},
  {"default-saves",	0, &reinitsav,	1},
  {"x10-saves",		0, &x10sav,	1},
  {"devparm",		0, &devparm,	1},
  {"snap",		0, &snap,	0},
  {"no-joystick",	0, &joyoff,	'J'},
  {"mono",		0, 0,		'm'},
  {"8bits",		0, 0,		'8'},
  {"high-quality",	0, 0,		'q'},
  {"swap-sides",	0, 0,		's'},
  {"no-sfx",		0, 0,		'X'},
  {"no-double-fx",	0, &doublefx,	0},
  {"load",		1, 0,		'l'},
  {"go",		0, 0,		'g'},
  {"drivers-info",	0, 0,		'n'},
  {"driver",		0, 0,		'd'},
  {"no-sound",		0, 0,		'S'},
  {"gfx-options",	1, 0,		'G'},
  {"full-screen",	1, 0,		'F'},
  {"double",		0, 0,		'2'},
  {"triple",		0, 0,		'3'},
  {"even-lines",	0, 0,		'e'},
  {0,			0, 0,		0}
};

int
parse_argv (int argc, char **argv)
{
  int c;

  /* Reset optind so that getopt is reinitialized. */
  optind = 0;

  for (;;) {
    int option_index = 0;

    c = getopt_long (argc, argv, "vhm8qsXl:gnd:G:JF23Se", 
		     long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case 'v':
      version ();
      return 1;
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
      level_name = strdup (optarg);
      level_name = strappend (level_name, ".lvl");
      loadulevel = 1;
      break;
    case 'g':
      directmenu = 1;
      break;
    case 'n':
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
    case 'q':
      hqmix = 1;
      break;
    case 'S':
      nosound = 1;
      break;
    case '?':
      /* getopt_long already printed an error message. */
    case 0:
      break;
      
    default:
      abort ();
    }
  }
  return 0;
}
