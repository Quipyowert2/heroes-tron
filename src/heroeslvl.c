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
#include "error.h"
#include "lvl.h"

const char *program_name;	/* argv[0] */
int exit_status = 0;		/* $? */

struct options_t {
  bool print_directions;	/* --print d */
  bool print_filename;		/* --print f */
  bool print_header;		/* --print h */
  bool print_walls;		/* --print w */
  bool print_types;		/* --print t */
  bool print_type_keys;		/* --print T */
  bool print_tunnels;		/* --print @ */
  bool print_tile_details;	/* --print i */
  const char *indent;		/* Set to a prefix output on every
				   lines, e.g. "  ". (--indent) */
} options = {
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  ""
};

static void
default_options (void)
{
  options.print_filename = true;
  options.print_header = true;
  options.indent = "  ";
}

static void
version (void)
{
  puts ("heroeslvl (Heroes) " VERSION "\n");
  printf ("Copyright (C) %d  Alexandre Duret-Lutz.\n", 2001);
  puts ("This is free software; see the source for copying conditions.  "
	"There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS"
	" FOR A PARTICULAR PURPOSE.");
  exit (0);
}

static void
usage (int status)
{
  if (status) {
    fprintf (stderr, "Try '%s --help' for more information.\n", program_name);
    exit (status);
  }

  printf ("Usage: %s [OPTIONS]... level\n\n", program_name);
  puts ("Heroeslvl is a tool used to inspect Heroes' level files.\n");
  puts ("\
Mandatory arguments to long options are mandatory for short options too.\n");
  puts ("\
  -v, --version               display version number\n\
  -h, --help                  display this help");
  puts ("\
  -p, --print=WHAT            select information to display.  WHAT should be\n\
                                one or more of these characters:\n\
                                  d   print square directions\n\
                                  f   print filename\n\
                                  h   print header\n\
                                  i   print tile details\n\
                                  t   print square type map\n\
                                  T   print type keys\n\
                                  w   print square wall map\n\
                                  @   print tunnels");
  puts ("\
  -i, --indent                indent everything but the filename");
  puts ("");
  puts ("When no options are given, the default is -ipfh.");
  puts ("Report bugs to <heroes-bugs@lists.sourceforge.net>.");
  exit (status);
}

const struct option long_options[] = {
  {"help",		no_argument,		NULL,	'h'},
  {"indent",		no_argument,		NULL,   'i'},
  {"print",		required_argument,	NULL,	'p'},
  {"version",		no_argument,		NULL,	'v'},
  {NULL,		0,			NULL,	0}
};

static void
decode_print_options (const char *flags)
{
  while (*flags) {
    switch (*flags) {
    case 'd':
      options.print_directions = true;
      break;
    case 'f':
      options.print_filename = true;
      break;
    case 'h':
      options.print_header = true;
      break;
    case 'i':
      options.print_tile_details = true;
      break;
    case 't':
      options.print_types = true;
      break;
    case 'T':
      options.print_type_keys = true;
      break;
    case 'w':
      options.print_walls = true;
      break;
    case '@':
      options.print_tunnels = true;
      break;
    default:
      error (0, 0, "Unknown --print selection '%c'.", *flags);
      usage (1);
    }
    ++flags;
  }
}

static void
decode_switches (int argc, char **argv)
{
  int c;
  /* If DEFAULT_PRINT is still true after processing all the options,
     that means the default printing options should be used.  */
  bool default_print = true;

  while ((c = getopt_long (argc, argv, "hip:v", long_options, NULL)) != -1) {
    switch (c) {
    case 'h':
      usage (0);
    case 'v':
      version ();
    case 'i':
      options.indent = "  ";
      break;
    case 'p':
      decode_print_options (optarg);
      default_print = false;
      break;
    default:
      usage (1);
    }
  }

  if (default_print)
    default_options ();
}

static const char *
dir_to_string (dir_t dir)
{
  switch (dir) {
  case D_UP:
    return "up";
  case D_RIGHT:
    return "right";
  case D_DOWN:
    return "down";
  case D_LEFT:
    return "left";
  }
  exit_status = 4;
  return "error";
}

static void
print_header (const level_t *lvl)
{
  printf ("%sheight:\t%d tiles\t(%d squares)\n", options.indent,
	  lvl->tile_height, lvl->square_height);
  printf ("%swidth:\t%d tiles\t(%d squares)\n", options.indent,
	  lvl->tile_width, lvl->square_width);
  if (lvl->tile_height_wrap == DONT_WRAP)
    printf ("%sY-wrap:\tno\n", options.indent);
  else
    printf ("%sY-wrap:\t%d tiles\t(%d squares)\n", options.indent,
	    lvl->tile_height_wrap, lvl->square_height_wrap);
  if (lvl->tile_width_wrap == DONT_WRAP)
    printf ("%sX-wrap:\tno\n", options.indent);
  else
    printf ("%sX-wrap:\t%d tiles\t(%d squares)\n", options.indent,
	    lvl->tile_width_wrap, lvl->square_width_wrap);
  printf ("%ssound track alias:\t%s\n",
	  options.indent, lvl_sound_track (lvl));
  printf ("%stile map basename:\t%s\n",
	  options.indent, lvl_tile_sprite_map_basename (lvl));
  printf ("%sstarting squares and directions (y x dir):\n", options.indent);
  {
    int i;
    for (i = 0; i < 4; ++i) {
      square_coord_pair_t co;
      dir_t di;
      lvl_start_position (lvl, i, &co, &di);
      printf ("%s  #%d: %2d %2d %s\n", options.indent, i + 1,
	      co.y, co.x, dir_to_string (di));
    }
  }
}

static char
type_to_char (dir_t dir)
{
  switch (dir) {
  case T_NONE:
    return ' ';
  case T_STOP:
    return 'h';
  case T_SPEED:
    return 's';
  case T_TUNNEL:
    return 't';
  case T_BOOM:
    return 'b';
  case T_ANIM:
    return 'a';
  case T_ICE:
    return 'i';
  case T_DUST:
    return 'd';
  case T_OUTWAY:
    return 'X';
  default:
    exit_status = 4;
    return '?';
  }
}

const char *type_names[T_MAXTYPE] = {
  "none",
  "stop",
  "speed",
  "tunnel",
  "boom",
  "anim",
  "ice",
  "dust",
  "outway"
};

static void
print_type_keys (void)
{
  int i;
  const int ncols = 3;

  for (i = 0; i < T_MAXTYPE; ++i) {
    if (i % ncols == 0)
      printf ("%s", options.indent);
    printf ("'%c' %-20s", type_to_char (i), type_names[i]);
    if (i % ncols == ncols - 1)
      puts ("");
  }
  if (i % ncols != 0)
    puts ("");
}

static void
print_square_types (const level_t *lvl)
{
  square_coord_t y, x;
  square_index_t idx;
  for (y = 0, idx = 0; y < lvl->square_height; ++y) {
    printf ("%s|", options.indent);
    for (x = 0; x < lvl->square_width; ++x, ++idx)
      putchar (type_to_char (lvl->square_type[idx]));
    printf ("|%2u\n", y);
  }
}

static void
print_dir_mask (dir_mask_t dm)
{
  int nw = 0;

  if (dm & DM_UP)
    ++nw;
  if (dm & DM_RIGHT)
    ++nw;
  if (dm & DM_DOWN)
    ++nw;
  if (dm & DM_LEFT)
    ++nw;

  if (nw)
    printf ("%d", nw);
  else
    putchar (' ');
}

static void
print_square_walls (const level_t *lvl)
{
  square_coord_t y, x;
  square_index_t idx;
  for (y = 0, idx = 0; y < lvl->square_height; ++y) {
    printf ("%s|", options.indent);
    for (x = 0; x < lvl->square_width; ++x, ++idx)
      print_dir_mask (lvl->square_walls_out[idx]);
    printf ("|%2u\n", y);
  }
}

static char
dir_to_char (dir_t d)
{
  switch (d) {
  case D_UP:
    return '^';
  case D_RIGHT:
    return '>';
  case D_DOWN:
    return '.';
  case D_LEFT:
    return '<';
  default:
    return 'X';
  }
}

static void
print_square_directions (const level_t *lvl)
{
  square_coord_t y, x;
  square_index_t idx;
  for (y = 0, idx = 0; y < lvl->square_height; ++y) {
    printf ("%s|", options.indent);
    for (x = 0; x < lvl->square_width; ++x, ++idx)
      putchar (dir_to_char (lvl->square_direction[idx]));
    printf ("|%2u\n", y);
  }
}

static void
print_tunnels (const level_t *lvl)
{
  square_coord_t y, x;
  square_index_t idx;
  int tunbr = 0;		/* Number of tunnels.  */
  int curtun = 0;		/* Curent tunnel index.  */
  square_index_t *outputs;

  for (idx = 0; idx < lvl->square_count; ++idx)
    if (lvl->square_type[idx] == T_TUNNEL)
      ++tunbr;

  if (tunbr == 0) {
    printf ("%sNo tunnels.\n", options.indent);
    return;
  }

  XMALLOC_ARRAY (outputs, tunbr);

  for (curtun = 0, idx = 0; idx < lvl->square_count; ++idx)
    if (lvl->square_type[idx] == T_TUNNEL)
      outputs[curtun++] = idx;

  printf ("%sTunnels:\n", options.indent);
  printf ("%s  NBR  SRCIDX   Y   X  DIR      DESTIDX\n", options.indent);

  for (curtun = 0, y = 0, idx = 0; y < lvl->square_height; ++y)
    for (x = 0; x < lvl->square_width; ++x, ++idx)
      if (lvl->square_type[idx] == T_TUNNEL) {
	dir_t d = lvl->square_direction[idx];
	square_index_t outidx = lvl->square_move[d][idx];

	/* Search the number of the output tunnel.  */
	int i;
	for (i = 0; i < tunbr; ++i)
	  if (outputs[i] == outidx)
	    break;

	printf ("%s  #%-2d  %6d  %3d %3d %-8s %7d",
		options.indent, ++curtun, idx, y, x,
		dir_to_string (d), lvl->square_move[d][idx]);
	if (i < tunbr)
	  printf ("\t#%d", i + 1);
	puts ("");
      }

  free (outputs);
}

static const char *
anim_kind_to_str (anim_kind_t k)
{
  switch (k) {
  case A_NONE:
    return "still";
  case A_LOOP:
    return "loop";
  case A_PINGPONG:
    return "pingpong";
  }
  assert (0);
}

static void
print_tile_details (level_t *lvl)
{
  tile_index_t i;

  printf ("%sTILE  Y  X    TYPE      SPRITE    OVERLAY  ANIM-TYPE FRM DEL\n",
	  options.indent);
  for (i = 0; i < lvl->tile_count; ++i) {
    unsigned int o, c, d;
    anim_kind_t k;

    printf ("%s%4u %2u %2u %-10s 0x%08x", options.indent, i,
	    TILE_INDEX_TO_COORD_Y (lvl, i),
	    TILE_INDEX_TO_COORD_X (lvl, i),
	    type_names[lvl_tile_type (lvl, i)],
	    lvl_tile_sprite_offset (lvl, i));

    o = lvl_tile_sprite_overlay_offset (lvl, i);
    if (o)
      printf (" 0x%08x", o);
    else
      printf ("           ");

    lvl_animation_info (lvl, i, &c, &d, &k);
    printf (" %-10s", anim_kind_to_str (k));
    if (k)
      printf (" %2u %2u", c, d);
    else
      printf ("      ");

    puts ("");
  }
}

static void
process (const char *filename)
{
  int err;
  level_t lvl;

  err = lvl_load_file (filename, &lvl, true);
  if (err) {
    error (0, err, "cannot load %s", filename);
    exit_status = 3;
  } else {
    if (options.print_filename)
      printf ("File:\t%s\n", filename);
    if (options.print_header)
      print_header (&lvl);
    if (options.print_type_keys)
      print_type_keys ();
    if (options.print_types)
      print_square_types (&lvl);
    if (options.print_walls)
      print_square_walls (&lvl);
    if (options.print_directions)
      print_square_directions (&lvl);
    if (options.print_tunnels)
      print_tunnels (&lvl);
    if (options.print_tile_details)
      print_tile_details (&lvl);
  }
}

int
main (int argc, char *argv[])
{
  mtrace (); /* GNU libc's malloc debugging facility */
  program_name = argv[0];

  decode_switches (argc, argv);
  if (optind == argc) {
    error (0, 0, "Missing filename.");
    usage (1);
  }

  /* Process all files.  */
  for (; optind < argc; ++optind) {
    process (argv[optind]);
  }

  return exit_status;
}
