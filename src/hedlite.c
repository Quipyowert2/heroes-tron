/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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


void fatalog (char *ptr);
#define __HEDIT__
#define __HEDITver__ "1.5"
#define __HEDLITE__

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#include <sys\types.h>
#include <direct.h>
//#include <time.h>

#include "display.h"
#include "vgatxt.h"
#include "pcx.h"
#include "errors.h"
#include "fastmem.h"
#include "keyboard_map.h"
#include "mouse.h"
#include "reader.h"

#include "main_font_img.h"
#include "structs.h"

#include "hedlite.h"
#include "config.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

/************************ une paire de fonctions *********************/
tunnel_mod ();
anim_mod ();
fn0 ();
/*********************************************************************/

static image_ heditrsc, tile_set_img;

//#define img2vram(image) fastmem4((image)->buffer,screen,64000/4);
#define config "hedit.ini"

unsigned short int xdalles = 0, ydalles = 0, xdallesdec = 0;
unsigned short int xplan = 0, yplan = 0;
unsigned int tempd = 0xffffffff;
unsigned int xplandec = 0, yplandec = 0;
unsigned sprhide = 0, afftests = 0;

static unsigned char notestmouse = 0;
static level_header_t hplaninfo = { 0, 0, -1, -1 };
static char tile_set_name[128] = rscdir;
static char dallepie[128] = rscdir;
static char levelnom[128] = nivdir;
static char levelnomshort[13];
static char pcxnom[13];
static char nombre[5];
static char entree[20];
static FILE *ftmp;
static char *ligne = "\xFF\x00 ";
//static int htmp,htmp2;

static int scdec320[4] = { 0, 12, 3200, 3212 };

/****************************/

static
tile_t (*level_map)[];		// pointeur sur le level_map du niveau
     static tile_info_t (*ddef)[];	// pointeur sur la descro des tile_set_img
     static char (*outwaymap)[];
     static char (*hdradar)[];
     static char (*hdcolli)[];
/*************************** Attention les dents */

     void (*fnptr[type_nbr]) (int, int, int) =
{
&fn0, &fn0, &fn0, &tunnel_mod, &fn0, &anim_mod, &fn0, &fn0, &fn0};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static void
fatalog (char *ptr)
{
//   time_t tt;
//   auto char tmp[26];
//   tt=time(NULL);
//   fprintf(hlog,"\nABDNORMAL END on %s\tFATAL: ",_ctime((time_t*)&tt,(char*)htmp));
//   fprintf(hlog,ptr);
//   fclose(hlog);
  fatal_error (ptr);
}

static
partiel4 (xs, ys, xd, yd, xc, yc, source)
     short int xs, ys, xd, yd, xc, yc;
     image_ *source;
{
  int i = source->xt;
  int j;
  char *src = (source->buffer) + (i * ys) + xs;
  char *dest = screen + xc + yc * 320;
  for (j = yd; j > 0; j--) {
    fastmem4 (src, dest, xd >> 2);
    src += i;
    dest += 320;
  }
}

static
copy_tile (int src_, char *dest)
{
  int i = tile_set_img.xt;
  int j;
  char *src = (tile_set_img.buffer) + src_;
  for (j = 20; j > 0; j--) {
    fastmem4 (src, dest, 24 >> 2);
    src += i;
    dest += 320;
  }
}

static
partiel4c (int src_, char *dest)
{
  int i = tile_set_img.xt;
  int j;
  char *src = (tile_set_img.buffer) + src_;
  for (j = 20; j > 0; j--) {
    fastmem4 (src, dest, 24 >> 2);
    src += i;
    dest += hplaninfo.xt * 24;
  }
}

static
copy_tile_transp (int src_, char *dest)
{
  int i = tile_set_img.xt;
  int j, k;
  char *src = (tile_set_img.buffer) + src_;
  for (j = 20; j > 0; j--) {
    for (k = 24; k > 0; k--) {
      if (*src != 0)
	*dest = *src;
      src++;
      dest++;
    }
    src += i - 24;
    dest += 320 - 24;
  }
}

static
dalletranspc (int src_, char *dest)
{
  int i = tile_set_img.xt;
  int j, k;
  char *src = (tile_set_img.buffer) + src_;
  for (j = 20; j > 0; j--) {
    for (k = 24; k > 0; k--) {
      if (*src != 0)
	*dest = *src;
      src++;
      dest++;
    }
    src += i - 24;
    dest += hplaninfo.xt * 24 - 24;
  }
}

static
copy_square_transp (char *src, char *dest)
{
  int j, k;
  for (j = 10; j > 0; j--) {
    for (k = 12; k > 0; k--) {
      if (*src != 0)
	*dest = *src;
      src++;
      dest++;
    }
    src += 320 - 12;
    dest += 320 - 12;
  }
}

static
sousdalletranspc (char *src, char *dest)
{
  int j, k;
  for (j = 10; j > 0; j--) {
    for (k = 12; k > 0; k--) {
      if (*src != 0)
	*dest = *src;
      src++;
      dest++;
    }
    src += 320 - 12;
    dest += hplaninfo.xt * 24 - 12;
  }
}

static
partiel2 (xs, ys, xd, yd, xc, yc, source)
     short int xs, ys, xd, yd, xc, yc;
     image_ *source;
{
  int i = source->xt;
  int j;
  char *src = (source->buffer) + (i * ys) + xs;
  char *dest = screen + xc + yc * 320;
  for (j = yd; j > 0; j--) {
    fastmem2 (src, dest, xd >> 1);
    src += i;
    dest += 320;
  }
}

static
cadre (x0, y0, xd, yd, col)
     short int x0, y0, xd, yd;
     char col;
{
  int i;
  char *dest = screen + y0 * 320 + x0;
  for (i = xd; i > 0; i--) {
    *(dest + yd * 320) = col;
    *dest++ = col;
  };
  for (i = yd; i >= 0; i--) {
    *(dest - xd) = col;
    *dest = col;
    dest += 320;
  };
}

static
cadrept (x0, y0, xd, yd, col1, col2)
     short int x0, y0, xd, yd;
     char col1, col2;
{
  int i;
  char *dest = screen + y0 * 320 + x0;
  for (i = (xd >> 1); i > 0; i--) {
    *(dest + (yd - 1) * 320) = col1;
    *dest++ = col2;
    *(dest + (yd - 1) * 320) = col2;
    *dest++ = col1;
  };
  xd--;
  dest--;
  for (i = (yd >> 1); i > 0; i--) {
    *(dest - xd) = col2;
    *dest = col1;
    dest += 320;
    *(dest - xd) = col1;
    *dest = col2;
    dest += 320;
  };
}

static
draw_text (char *texte, int posx, int posy, char coul, char cent)
{
  char i, j;
  int k, d = -1;
  char *dest = screen + posx + posy * 320;
  char *src = texte;
  for (; *src != 0; src++) {
    i = *src - font_first_ascii;
    d += font_width[i] + 1;
  }
  if (cent == 0)
    d = 0;
  if (cent == 1)
    d = -(d >> 1);
  if (cent == 2)
    d = -d;
  dest += d;
  for (; *texte != 0; texte++) {
    i = *texte - font_first_ascii;
    src = heditrsc.buffer + font_pos + ((int) (i) << 2);
    for (j = font_width[i]; j > 0; j--) {
      for (k = 320 * (font_height - 1); k >= 0; k -= 320)
	if (*(src + k) != font_transp_color)
	  *(dest + k) = coul;
      dest++;
      src++;
    }
    dest++;
  }
}

/************** TROP DE PARAMETRES *************//*$$$$ */
static
transpa (char *source, char *dest, int xt, int yt, char coul)
{
  int x, y;
  for (y = yt; y > 0; y--) {
    for (x = xt; x > 0; x--)
      if (*source != coul)
	(*dest++ = *source++);
      else {
	dest++;
	source++;
      }
    source += 320 - xt;
    dest += 320 - xt;
  };
}

static
transpac (char *source, char *dest, int xt, int yt, char coul)
{
  int x, y;
  for (y = yt; y > 0; y--) {
    for (x = xt; x > 0; x--)
      if (*source != coul)
	(*dest++ = *source++);
      else {
	dest++;
	source++;
      }
    source += 320 - xt;
    dest += hplaninfo.xt * 24 - xt;
  };
}

static
carre (int x, int y, char c)
{
  long int *dest = (long int *) (screen + x + y * 320);
  *dest = 0;
  if (c) {
    *(dest + 80) = 0x80800;
    *(dest + 80 * 2) = 0x80800;
  } else {
    *(dest + 80) = 0;
    *(dest + 80 * 2) = 0;
  }
  *(dest + 80 * 3) = 0;
}

static
affgt (int t)
{
  switch ((*level_map)[t].type) {
  case t_speed:
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.param[0] * 12 + 1, screen + 294 + 88 * 320,
	     10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.param[1] * 12 + 1, screen + 306 + 88 * 320,
	     10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.param[2] * 12 + 1, screen + 294 + 99 * 320,
	     10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.param[3] * 12 + 1, screen + 306 + 99 * 320,
	     10, 9, 71);
//               partiel4(60,112,30,27,290,112,&heditrsc);
    partiel4 (0, 112, 30, 27, 290, 112, &heditrsc);

//x            ultoa(1+((*level_map)[t].info.param[4]>>4),&nombre,10);
//x            draw_text(&nombre,311,179,8,1);
    if (((*level_map)[t].info.param[4] >> 4) > 0) {
      partiel2 (60, 168, 30, 32, 290, 168, &heditrsc);
      ultoa ((*level_map)[t].info.param[4] & 0xf, &nombre, 10);
      draw_text (&nombre, 311, 193, 8, 1);
    } else
      partiel2 (0, 168, 30, 32, 290, 168, &heditrsc);
    break;
  case t_dust:
  case t_ice:
  case t_boom:
  case t_stop:
    carre (297, 91, (*level_map)[t].info.param[0]);
    carre (297 + 12, 91, (*level_map)[t].info.param[1]);
    carre (297, 91 + 10, (*level_map)[t].info.param[2]);
    carre (297 + 12, 91 + 10, (*level_map)[t].info.param[3]);
//               partiel4(30,112,30,27,290,112,&heditrsc);
    partiel4 (0, 112, 30, 27, 290, 112, &heditrsc);
  case t_outway:		//x partiel2(30,168,30,32,290,168,&heditrsc);
//x            ultoa((*level_map)[t].info.param[4]&0xf,&nombre,10);
//x            draw_text(&nombre,311,193,8,1);
//x            if ((*level_map)[t].info.param[4]&0xf>1) {
//x            ultoa(1+((*level_map)[t].info.param[4]>>4),&nombre,10);
//x            draw_text(&nombre,311,179,8,1); }
    if (((*level_map)[t].info.param[4] >> 4) > 0) {
      partiel2 (60, 168, 30, 32, 290, 168, &heditrsc);
      ultoa ((*level_map)[t].info.param[4] & 0xf, &nombre, 10);
      draw_text (&nombre, 311, 193, 8, 1);
    } else
      partiel2 (0, 168, 30, 32, 290, 168, &heditrsc);
    break;
  case t_tunnel:
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.tunnel.direction * 12 + 1,
	     screen + 294 + 88 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.tunnel.direction * 12 + 1,
	     screen + 306 + 88 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.tunnel.direction * 12 + 1,
	     screen + 294 + 99 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 30 + 20 * 320 +
	     (*level_map)[t].info.tunnel.direction * 12 + 1,
	     screen + 306 + 99 * 320, 10, 9, 71);
//               partiel2(0,168,30,32,290,168,&heditrsc);
    partiel4 (120, 112, 30, 27, 290, 112, &heditrsc);
//               ultoa((*level_map)[t].info.tunnel.tempo,&nombre,10);
//               draw_text(&nombre,311,119,8,1);
    ultoa ((*level_map)[t].info.tunnel.output / hplaninfo.xt, &nombre, 10);
    draw_text (&nombre, 307, 133, 8, 0);
    ultoa ((*level_map)[t].info.tunnel.output % hplaninfo.xt, &nombre, 10);
    draw_text (&nombre, 302, 133, 8, 2);
    break;
  case t_anim:
    partiel4 (150 + 60, 112, 30, 27, 290, 112, &heditrsc);
//x            ultoa((*level_map)[t].info.anim.frame_nbr+1,&nombre,10);
//x            draw_text(&nombre,311,119,8,1);
    ultoa ((*level_map)[t].info.anim.speed, &nombre, 10);
    draw_text (&nombre, 311, 133, 8, 1);
    partiel2 (0, 168, 30, 32, 290, 168, &heditrsc);
    break;
  default:
    partiel4 (0, 112, 30, 27, 290, 112, &heditrsc);
    partiel2 (0, 168, 30, 32, 290, 168, &heditrsc);
    break;

  }
}

fn0 ()
{
}				/*************************************//*$$$$ */

static
tunnel_mod (int i, int x, int y)
{
// unsigned char m;

/*
 if (y<112)
 {
  if (y>98)  y=y-99;  else y=y-88;
  if (x>=305) x=x-305; else x=x-293;
  if (x>11 || y>8 || x<0 || y<0) return;
  m=spd_test[y][x] &0xf;
   (*level_map)[i].info.tunnel.direction=m;
 }
 else
 {
  if (y<126 && x<305)
  { (*level_map)[i].info.tunnel.tempo=(*level_map)[i].info.tunnel.tempo + ((y<119)?+1:-1);
  } else
*/
  if (y > 126 && y < 133 && tempd != 0xffffffff)
    (*level_map)[i].info.tunnel.output = tempd;
// }
}

static
anim_mod (int i, int x, int y)
{
  unsigned char m;

  if (y >= 112 && x < 305) {
    m = ((y - 112) / 7);
    {
      if (m >= 2)

	(*level_map)[i].info.anim.speed =
	  (*level_map)[i].info.anim.speed + ((m == 2) ? +1 : -1);
//    else     (*level_map)[i].info.anim.frame_nbr= ((*level_map)[i].info.anim.frame_nbr + ((m==0)?+1:-1))&63;
    }
  }
}

static
anim_mod_bcl (int i, int x, int y)
{
  unsigned char m;

  if (y >= 172 && x < 305) {
    m = ((y - 172) / 7);
    {
      if (m >= 2 && ((*level_map)[i].info.param[4] >> 4) > 0)

	(*level_map)[i].info.param[4] =
	  ((*level_map)[i].
	   info.param[4]) & 0xf0 | (((*level_map)[i].info.param[4] +
				     ((m == 2) ? +1 : -1)) & 0xf);
//    else     (*level_map)[i].info.param[4]=((*level_map)[i].info.param[4])&0x0f | ((((*level_map)[i].info.param[4]+ ((m==0)?+16:-16))&0xf0));
    }
  }
}

static
majd ()
{
  partiel4 (xdalles, 0, 144, 200, 145, 0, &tile_set_img);
  cadre (145 + xdallesdec, ydalles, 23, 19, 15);
  partiel2 (0, 64, 30, 6, 290, 64, &heditrsc);
  ultoa ((xdalles + xdallesdec) / 24, &nombre, 10);
  draw_text (&nombre, 302, 64, 15, 2);
  ultoa (ydalles / 20, &nombre, 10);
  draw_text (&nombre, 307, 64, 15, 0);
}

static
remove_comments (char (*str)[])
{
  int i;
  for (i = 0; ((*str)[i] != ' ') && ((*str)[i] != '.'); i++);
  (*str)[i] = 0;
}

static int
readconfig ()
{
  FILE *fconf;
  char tmp[256];
  char *tmpptr = (char *) tmp;
// int i;

// fprintf(hlog,"Reading config... ");

  if ((fconf = fopen (config, "rt")) == NULL)
    fatalog ("Did not manage to open " config);
  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  remove_comments (tmpptr);
  hplaninfo.xt = atol (tmpptr);
  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  if ((tmp[0] & 223) == 89)	// 89="Y"
  {
    if ((hplaninfo.xt & (hplaninfo.xt - 1)) == 0)
      hplaninfo.xwrap = hplaninfo.xt - 1;
    else
      fatalog ("Cannot use X-wraping");
  };
  if (hplaninfo.xt < 15 && hplaninfo.xwrap == -1)
    fatalog ("Minimal number of column: 15 (unless there is X-Wrapping)");

  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  remove_comments (tmpptr);
  hplaninfo.yt = atol (tmpptr);
  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  if ((tmp[0] & 223) == 89)	// 89="Y"
  {
    if ((hplaninfo.yt & (hplaninfo.yt - 1)) == 0)
      hplaninfo.ywrap = hplaninfo.yt - 1;
    else
      fatalog ("Cannot use Y-wraping");
  };
  if (hplaninfo.yt < 11 && hplaninfo.ywrap == -1)
    fatalog ("Minimal number of row: 11 (unless there is Y-Wrapping)");

  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  remove_comments (tmpptr);
  strcpy (&hplaninfo.tile_set_name, tmpptr);
  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  remove_comments (tmpptr);
  strcpy (&hplaninfo.soundtrack_name, tmpptr);
  if (fgets (tmpptr, 256, fconf) == NULL)
    return (-1);
  remove_comments (tmpptr);
  strcpy (&levelnomshort, tmpptr);
  fclose (fconf);

  strlwr (&levelnomshort);
  strlwr (&hplaninfo.tile_set_name);
  strlwr (&hplaninfo.soundtrack_name);

// fprintf(hlog,"done\n");
  return (NULL);
}

static
affplan (int xloc, int yloc, char c)
{
  int i, j;
  int k, l, m, n;
  char *dest = screen;
  int xx, yy = 7;
  for (k = yloc, l = 10; l > 0; l--, k = ((k + 1) & hplaninfo.ywrap)) {
    m = k * hplaninfo.xt;
    xx = 12;
    for (i = xloc, j = 6 + c; j > 0; j--, i = ((i + 1) & hplaninfo.xwrap)) {
      copy_tile ((*level_map)[i + m].number, dest);
      if (sprhide == 0) {
	for (n = 0; n < 4; n++)
	  if (i + m == hplaninfo.start[n])
	    copy_square_transp (heditrsc.buffer + (16 + (n << 4)) * 320 +
				256 + (hplaninfo.start_way[n] & 0xf0),
				dest +
				square_offset_320[hplaninfo.
						  start_way[n] & 0xf]);
	if ((*level_map)[i + m].sprite != 0)
	  copy_tile_transp ((*level_map)[i + m].sprite, dest);
      }
      if (afftests)
	if ((*level_map)[i + m].type == t_outway)
	  transpa (heditrsc.buffer + 10 * 320 + 222, dest, 24, 20, 0);
	else
	  for (n = 0; n < 4; n++)
	    if ((*level_map)[i + m].collision[n] == 0xf)
	      copy_square_transp (heditrsc.buffer + 10 * 320 + 222 +
				  scdec320[n], dest + square_offset_320[n]);
	    else
	      copy_square_transp (heditrsc.buffer + 10 * 320 + 30 +
				  (*level_map)[i + m].collision[n] * 12,
				  dest + square_offset_320[n]);
      if (i + m == tempd)
	cadrept (xx - 12, yy - 7, 24, 20, 8, 15);
      dest += 24;
      xx += 24;
    }
    dest += 20 * 320 - 24 * (6 + c);
    yy += 20;
  }
};

static
majg ()
{
  affplan (xplan, yplan, 0);
  {
    copy_tile ((*level_map)
	       [((xplan + xplandec / 24) & hplaninfo.xwrap) +
		((yplan + yplandec / 20) & hplaninfo.ywrap) *
		hplaninfo.xt].number, screen + 293 + 88 * 320);
    affgt (curdallep ());
    partiel2 (0, 71, 30, 13, 290, 71, &heditrsc);
    if ((*level_map)[curdallep ()].sprite != 0)
      draw_text ("[S]", 309, 71, 8, 0);
    draw_text (type_name[(*level_map)[curdallep ()].type], 304, 78, 8, 1);
  }
  cadre (xplandec, yplandec, 23, 19, 8);
  partiel2 (0, 57, 30, 6, 290, 57, &heditrsc);
  ultoa (((xplan + xplandec / 24) & hplaninfo.xwrap), &nombre, 10);
  draw_text (&nombre, 302, 57, 8, 2);
  ultoa (((yplan + yplandec / 20) & hplaninfo.ywrap), &nombre, 10);
  draw_text (&nombre, 307, 57, 8, 0);
}

static int
curdallep ()
{
  int i =
    ((xplan + xplandec / 24) & hplaninfo.xwrap) +
    ((yplan + yplandec / 20) & hplaninfo.ywrap) * hplaninfo.xt;
  return i;
}

static int
curdallepg (int c)
{
  int x = c % hplaninfo.xt;
  if (x > 0)
    return (c - 1);
  else if (hplaninfo.xwrap != -1)
    return (c + hplaninfo.xt - 1);
  return -1;
}

static int
curdalleph (int c)
{
  int y = c / hplaninfo.xt;
  if (y > 0)
    return (c - hplaninfo.xt);
  else if (hplaninfo.ywrap != -1)
    return (c + hplaninfo.xt * (hplaninfo.yt - 1));
  return -1;
}

static int
curdallepd (int c)
{
  int x = c % hplaninfo.xt;
  if (x + 1 < hplaninfo.xt)
    return (c + 1);
  else if (hplaninfo.xwrap != -1)
    return (c - hplaninfo.xt + 1);
  return -1;
}

static int
curdallepb (int c)
{
  int y = c / hplaninfo.xt;
  if (y + 1 < hplaninfo.yt)
    return (c + hplaninfo.xt);
  else if (hplaninfo.ywrap != -1)
    return (c % hplaninfo.xt);
  return -1;
}

static int
curdalled ()
{
  int i =
    (xdalles + xdallesdec) / 24 + (ydalles / 20) * (tile_set_img.xt / 24);
  return i;
}

static
departfix ()
{
  unsigned int d = curdallep (), i;
  signed int x, y;
  static unsigned char c = 0;
  unsigned char l = 0, m, flag;

  copy_tile ((*level_map)
	     [((xplan + xplandec / 24) & hplaninfo.xwrap) +
	      ((yplan + yplandec / 20) & hplaninfo.ywrap) *
	      hplaninfo.xt].number, screen + 293 + 88 * 320);

  while (mouse12 () != 0);
  do {
    transpa (heditrsc.buffer + 31 + 20 * 320, screen + 294 + 88 * 320, 10, 9,
	     71);
    for (i = 0; i < 4; i++)
      if (hplaninfo.start[i] == d && (hplaninfo.start_way[i] & 0xf) == 0)
	transpa (heditrsc.buffer + 31 + (hplaninfo.start_way[i] >> 4) * 12 +
		 (29 + i * 9) * 320, screen + 294 + 88 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 31 + 20 * 320, screen + 306 + 88 * 320, 10, 9,
	     71);
    for (i = 0; i < 4; i++)
      if (hplaninfo.start[i] == d && (hplaninfo.start_way[i] & 0xf) == 1)
	transpa (heditrsc.buffer + 31 + (hplaninfo.start_way[i] >> 4) * 12 +
		 (29 + i * 9) * 320, screen + 306 + 88 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 31 + 20 * 320, screen + 294 + 99 * 320, 10, 9,
	     71);
    for (i = 0; i < 4; i++)
      if (hplaninfo.start[i] == d && (hplaninfo.start_way[i] & 0xf) == 2)
	transpa (heditrsc.buffer + 31 + (hplaninfo.start_way[i] >> 4) * 12 +
		 (29 + i * 9) * 320, screen + 294 + 99 * 320, 10, 9, 71);
    transpa (heditrsc.buffer + 31 + 20 * 320, screen + 306 + 99 * 320, 10, 9,
	     71);
    for (i = 0; i < 4; i++)
      if (hplaninfo.start[i] == d && (hplaninfo.start_way[i] & 0xf) == 3)
	transpa (heditrsc.buffer + 31 + (hplaninfo.start_way[i] >> 4) * 12 +
		 (29 + i * 9) * 320, screen + 306 + 99 * 320, 10, 9, 71);
    partiel4 (90, 112, 30, 27, 290, 112, &heditrsc);
    partiel4 (96 + c * 5, 139, 4, 4, 296 + c * 5, 112, &heditrsc);
    mouseon ();
    while (mouse12 () == 0 && key_ready () == 0);
    if (mouse1 ()) {
      x = mousex ();
      y = mousey ();
      if (y < 112) {
	l = 0;
	if (y > 98) {
	  l = 2;
	  y = y - 99;
	} else
	  y = y - 88;
	if (x >= 305) {
	  l++;
	  x = x - 305;
	} else
	  x = x - 293;
	if (!(x > 11 || y > 8 || x < 0 || y < 0)) {
	  flag = 0;
	  m = dir_test[y][x] & 0x3;	/* &0x3 évite les mauvaises surprises */
	  for (i = 0; i < 4; i++)
	    if (hplaninfo.start[i] == d
		&& (hplaninfo.start_way[i] & 0xf) == l && i != c)
	      flag = 1;
	  if (flag == 0) {
	    hplaninfo.start[c] = d;
	    hplaninfo.start_way[c] = (m << 4) + l;
	    affplan (xplan, yplan, 0);
	  }
	  while (mouse12 () != 0);
	}
      } else {
	if (y < 116 && x >= 296 && x <= 316)
	  c = (x - 296) / 5;
	while (mouse12 () != 0);
      }
    }
    mouseoff ();
  }
  while (mousex () > 290 && (!key_ready ()) && mouse2 () == 0);
  majg ();
}

/*
static collisionsd()
{
  unsigned int d=curdalled();
  signed int x,y;

  partiel4(xdalles+xdallesdec,ydalles,24,20,293,88,&tile_set_img);
  partiel4(30,112,30,27,290,112,&heditrsc);

  while (mouse12()!=0);
 do{
  carre(291,91,   (*ddef)[d].collision[0]&c_left);
  carre(303,91,   (*ddef)[d].collision[1]&c_left | (*ddef)[d].collision[0]&c_right);
  carre(315,91,   (*ddef)[d].collision[1]&c_right);
  carre(291,101,(*ddef)[d].collision[2]&c_left);
  carre(303,101,(*ddef)[d].collision[3]&c_left | (*ddef)[d].collision[2]&c_right);
  carre(315,101,(*ddef)[d].collision[3]&c_right);
  carre(297,86,   (*ddef)[d].collision[0]&c_up);
  carre(297,96,   (*ddef)[d].collision[0]&c_down | (*ddef)[d].collision[2]&c_up);
  carre(297,106,  (*ddef)[d].collision[2]&c_down);
  carre(309,86,   (*ddef)[d].collision[1]&c_up);
  carre(309,96,   (*ddef)[d].collision[1]&c_down | (*ddef)[d].collision[3]&c_up);
  carre(309,106,  (*ddef)[d].collision[3]&c_down);

  mouseon();
  while (mouse12()==0 && key_ready()==0 );
  if (mouse1() && mousex()>290)
  {
    x=mousex();
    y=mousey();
    if (x>=291 && x<=294)
    { if (y>=91 && y<=94) (*ddef)[d].collision[0]^=c_left;
      else if (y>=101 && y<=104) (*ddef)[d].collision[2]^=c_left;
    }
    if (x>=297 && x<=300)
    { if (y>=86 && y<=89) (*ddef)[d].collision[0]^=c_up;
      if (y>=96 && y<=99){(*ddef)[d].collision[0]^=c_down;
                          (*ddef)[d].collision[2]^=c_up;}
      if(y>=106 && y<=109)(*ddef)[d].collision[2]^=c_down;
    }
    if (x>=303 && x<=306)
    { if (y>=91 && y<=94) {(*ddef)[d].collision[0]^=c_right;
                           (*ddef)[d].collision[1]^=c_left;}
      else if (y>=101 && y<=104) {(*ddef)[d].collision[2]^=c_right;
                                  (*ddef)[d].collision[3]^=c_left;}
    }
    if (x>=309 && x<=312)
    { if (y>=86 && y<=89) (*ddef)[d].collision[1]^=c_up;
      if (y>=96 && y<=99){(*ddef)[d].collision[1]^=c_down;
                          (*ddef)[d].collision[3]^=c_up;}
      if(y>=106 && y<=109)(*ddef)[d].collision[3]^=c_down;
    }
    if (x>=315 && x<=319)
    { if (y>=91 && y<=94) (*ddef)[d].collision[1]^=c_right;
      else if (y>=101 && y<=104) (*ddef)[d].collision[3]^=c_right;
    }
    if (y>=112 && y<119) {(*ddef)[d].collision[0]=0xf;
                          (*ddef)[d].collision[1]=0xf;
                          (*ddef)[d].collision[2]=0xf;
                          (*ddef)[d].collision[3]=0xf;}
    if (y>=119 && y<126) {(*ddef)[d].collision[0]=0x0;
                          (*ddef)[d].collision[1]=0x0;
                          (*ddef)[d].collision[2]=0x0;
                          (*ddef)[d].collision[3]=0x0;}
    while (mouse12()!=0);
  }
  mouseoff();
 }
 while(mousex()>290 && (!key_ready()) && mouse2()==0 );
 majd();
 cadre(292,87,25,21,0);
 cadre(291,86,27,23,71);
 if (mousex()>290) notestmouse=1;
}

static collisionsg()
{
  unsigned int d=curdallep();
  signed int x,y;
  copy_tile((*level_map)[((xplan+xplandec/24)&hplaninfo.xwrap)+((yplan+yplandec/20)&hplaninfo.ywrap)*hplaninfo.xt].number,screen+293+88*320);
  partiel4(30,112,30,27,290,112,&heditrsc);

  while (mouse12()!=0);
 do{
  carre(291,91,  (*level_map)[d].collision[0]&c_left);
  carre(301,91,  (*level_map)[d].collision[0]&c_right);
  carre(305,91,  (*level_map)[d].collision[1]&c_left);
  carre(315,91,  (*level_map)[d].collision[1]&c_right);
  carre(291,101,(*level_map)[d].collision[2]&c_left);
  carre(301,101,(*level_map)[d].collision[2]&c_right);
  carre(305,101,(*level_map)[d].collision[3]&c_left);
  carre(315,101,(*level_map)[d].collision[3]&c_right);
  carre(297,86,   (*level_map)[d].collision[0]&c_up);
  carre(297,94,   (*level_map)[d].collision[0]&c_down);
  carre(297,98,   (*level_map)[d].collision[2]&c_up);
  carre(297,106,  (*level_map)[d].collision[2]&c_down);
  carre(309,86,   (*level_map)[d].collision[1]&c_up);
  carre(309,94,   (*level_map)[d].collision[1]&c_down);
  carre(309,98,   (*level_map)[d].collision[3]&c_up);
  carre(309,106,  (*level_map)[d].collision[3]&c_down);

  mouseon();
  while (mouse12()==0 && key_ready()==0 );
  if (mouse1() && mousex()>290)
  {
    x=mousex();
    y=mousey();
    if (x>=291 && x<=294)
    { if (y>=91 && y<=94) (*level_map)[d].collision[0]^=c_left;
      else if (y>=101 && y<=104) (*level_map)[d].collision[2]^=c_left;
    }
    if (x>=297 && x<=300)
    { if (y>=86 && y<=89) (*level_map)[d].collision[0]^=c_up;
      if (y>=94 && y<=97) (*level_map)[d].collision[0]^=c_down;
      if (y>=98 && y<=101)(*level_map)[d].collision[2]^=c_up;
      if(y>=106 && y<=109)(*level_map)[d].collision[2]^=c_down;
    }
    if (x>=301 && x<=304)
    { if (y>=91 && y<=94) (*level_map)[d].collision[0]^=c_right;
      else if (y>=101 && y<=104) (*level_map)[d].collision[2]^=c_right;
    }
    if (x>=305 && x<=308)
    { if (y>=91 && y<=94) (*level_map)[d].collision[1]^=c_left;
      else if (y>=101 && y<=104) (*level_map)[d].collision[3]^=c_left;
    }
    if (x>=309 && x<=312)
    { if (y>=86 && y<=89) (*level_map)[d].collision[1]^=c_up;
      if (y>=94 && y<=97) (*level_map)[d].collision[1]^=c_down;
      if (y>=98 && y<=101)(*level_map)[d].collision[3]^=c_up;
      if(y>=106 && y<=109)(*level_map)[d].collision[3]^=c_down;
    }
    if (x>=315 && x<=319)
    { if (y>=91 && y<=94) (*level_map)[d].collision[1]^=c_right;
      else if (y>=101 && y<=104) (*level_map)[d].collision[3]^=c_right;
    }
    if (y>=112 && y<119) {(*level_map)[d].collision[0]=0xf;
                          (*level_map)[d].collision[1]=0xf;
                          (*level_map)[d].collision[2]=0xf;
                          (*level_map)[d].collision[3]=0xf;}
    if (y>=119 && y<126) {(*level_map)[d].collision[0]=0x0;
                          (*level_map)[d].collision[1]=0x0;
                          (*level_map)[d].collision[2]=0x0;
                          (*level_map)[d].collision[3]=0x0;}
    while (mouse12()!=0);
  }
  mouseoff();
 }//do
 while(mousex()>290 && (!key_ready()) && mouse2()==0);
 majg();
 cadre(292,87,25,21,0);
 cadre(291,86,27,23,71);
 if (mousex()>290) notestmouse=1;
}
*/
static
write_rle (char *src, int t, FILE * fpcx)
{
  char old, new;
  int i;
  int nbr = 1;

  old = *src++;

  for (i = 1; i < t; i++) {
    new = *src++;
    if (nbr == 63 || (nbr > 1 && (new != old))) {
      putc (nbr | 192, fpcx);
      putc (old, fpcx);
      old = new;
      nbr = 1;
    } else if (old == new && nbr < 63)
      nbr++;
    else {
      if (old < 192)
	putc (old, fpcx);
      else {
	putc (193, fpcx);
	putc (old, fpcx);
      }
      old = new;
    }
  }
  if (nbr == 1) {
    if (old < 192)
      putc (old, fpcx);
    else {
      putc (193, fpcx);
      putc (old, fpcx);
    }
  } else {
    putc (nbr | 192, fpcx);
    putc (old, fpcx);
  }
}

static
save_pcx ()
{
  FILE *fpcx;
  entete_ headpcx;
  int i1, i3, n;
  int j3;
  char *tempc, *dest;
  int sdec[4];

// fprintf(hlog,"\tSaving pcx: %s... ",pcxnom);

  tempc = malloc (hplaninfo.xt * 20 * 24);
  sdec[0] = 0;
  sdec[1] = 12;
  sdec[2] = hplaninfo.xt * 24 * 10;
  sdec[3] = hplaninfo.xt * 24 * 10 + 12;

  headpcx.signature = 10;
  headpcx.version = 5;
  headpcx.rle = 1;
  headpcx.bits_par_pixels = 8;
  headpcx.x = headpcx.y = 0;
  headpcx.largeur = (hplaninfo.xt * 24) - 1;
  headpcx.hauteur = (hplaninfo.yt * 20) - 1;
  headpcx.bytes_par_lignes = headpcx.largeurdpi = hplaninfo.xt * 24;
  headpcx.hauteurdpi = hplaninfo.yt * 20;
  headpcx.palette_genre = headpcx.nbrplans = 1;
  if ((fpcx = fopen (&pcxnom, "wb")) == NULL)
    return;
  fwrite ((char *) &headpcx, 1, sizeof (entete_), fpcx);

  for (i3 = 0; i3 < hplaninfo.yt; i3++) {
    j3 = hplaninfo.xt * i3;
    dest = tempc;
    for (i1 = 0; i1 < hplaninfo.xt; i1++) {
      partiel4c ((*level_map)[i1 + j3].number, dest);
      if (sprhide == 0) {
	for (n = 0; n < 4; n++)
	  if (i1 + j3 == hplaninfo.start[n])
	    sousdalletranspc (heditrsc.buffer + (16 + (n << 4)) * 320 + 256 +
			      (hplaninfo.start_way[n] & 0xf0),
			      dest + sdec[hplaninfo.start_way[n] & 0xf]);
	if ((*level_map)[i1 + j3].sprite != 0)
	  dalletranspc ((*level_map)[i1 + j3].sprite, dest);
      }
      if (afftests)
	if ((*level_map)[i1 + j3].type == t_outway)
	  transpac (heditrsc.buffer + 10 * 320 + 222, dest, 24, 20, 0);
	else
	  for (n = 0; n < 4; n++)
//         sousdalletranspc(heditrsc.buffer+10*320+30+(*level_map)[i1+j3].collision[n]*12,dest+sdec[n]);
	    if ((*level_map)[i1 + j3].collision[n] == 0xf)
	      sousdalletranspc (heditrsc.buffer + 10 * 320 + 222 +
				scdec320[n], dest + sdec[n]);
	    else
	      sousdalletranspc (heditrsc.buffer + 10 * 320 + 30 +
				(*level_map)[i1 + j3].collision[n] * 12,
				dest + sdec[n]);

      dest += 24;
    }
    write_rle (tempc, hplaninfo.xt * 20 * 24, fpcx);
  }
  free (tempc);
  putc (0xC, fpcx);
  for (i1 = 0; i1 < 768; i1++)
    putc (tile_set_img.palette.global[i1] << 2, fpcx);
  fclose (fpcx);
// fprintf(hlog,"done\n");
}

static
planfull ()
{
  int t;
  int xm = 128, ym = 100;
  setmouse (128, 100);
  _fmemset (screen, 0, 64000);
  if (xplan > (hplaninfo.xt - 13) && hplaninfo.xwrap == -1)
    xplan = hplaninfo.xt - 13;
  affplan (xplan, yplan, 7);
  while (mouse123 () != 0);
  do {
    affplan (xplan, yplan, 7);
    t = 0;
    while (key_ready () == 0 && (xm - mousex ()) <= 1
	   && (mousex () - xm) <= 1 && (ym - mousey ()) <= 1
	   && (mousey () - ym) <= 1 && mouse123 () == 0);
    if (key_ready ())
      t = get_key ();
    if (t == 0x4d00 || (mousex () - xm) > 1) {
      if (xplan < (hplaninfo.xt - 13) || hplaninfo.xwrap != -1)
	xplan = ((xplan + 1) & hplaninfo.xwrap);
      setmouse (128, mousey ());
    }
    if (t == 0x4b00 || (xm - mousex ()) > 1) {
      if (xplan > 0 || hplaninfo.xwrap != -1)
	xplan = ((xplan - 1) & hplaninfo.xwrap);
      setmouse (128, mousey ());
    }
    if (t == 0x5000 || (mousey () - ym) > 1) {
      if (yplan < (hplaninfo.yt - 10) || hplaninfo.ywrap != -1)
	yplan = ((yplan + 1) & hplaninfo.ywrap);
      setmouse (mousex (), 100);
    }
    if (t == 0x4800 || (ym - mousey ()) > 1) {
      if (yplan > 0 || hplaninfo.ywrap != -1)
	yplan = ((yplan - 1) & hplaninfo.ywrap);
      setmouse (mousex (), 100);
    }
  } while (t != 0x1c0d && t != 0x3920 && t != 0x11b && mouse123 () == 0);
  _fmemset (screen, 0, 64000);
  partiel2 (0, 0, 30, 200, 290, 0, &heditrsc);
  draw_text (&levelnomshort, 305, 29, 8, 1);
  ultoa (hplaninfo.xt, &nombre, 10);
  draw_text (&nombre, 302, 43, 8, 2);
  ultoa (hplaninfo.yt, &nombre, 10);
  draw_text (&nombre, 307, 43, 8, 0);
  majd ();
  majg ();
}

/*
outwayrecurs(int d,short int x,short int y)
{
        (*outwaymap)[d]=1;
        if (!((*hdcolli)[d]&d_right)) {
                htmp=(x+1)&((hplaninfo.xwrap<<1)+1);
                htmp2=htmp+y*hplaninfo.xt*2;
                if (!(*outwaymap)[htmp2])
                        outwayrecurs(htmp2,htmp,y);
        };
        if (!((*hdcolli)[d]&d_left)) {
                htmp=(x-1)&((hplaninfo.xwrap<<1)+1);
                htmp2=htmp+y*hplaninfo.xt*2;
                if (!(*outwaymap)[htmp2])
                        outwayrecurs(htmp2,htmp,y);
        };
        if (!((*hdcolli)[d]&d_up)) {
                htmp=(y-1)&((hplaninfo.ywrap<<1)+1);
                htmp2=x+htmp*hplaninfo.xt*2;
                if (!(*outwaymap)[htmp2])
                        outwayrecurs(htmp2,x,htmp);
        };
        if (!((*hdcolli)[d]&d_down)) {
                htmp=(y+1)&((hplaninfo.ywrap<<1)+1);
                htmp2=x+htmp*hplaninfo.xt*2;
                if (!(*outwaymap)[htmp2])
                        outwayrecurs(htmp2,x,htmp);
        };

} */

static char
outwayinit ()
{
  outwaymap = malloc (hplaninfo.xt * hplaninfo.yt * 4);
  if (outwaymap == NULL)
    return (1);
  if ((hdradar = malloc (hplaninfo.xt * hplaninfo.yt * 4)) == NULL)
    return (1);
  if ((hdcolli = malloc (hplaninfo.xt * hplaninfo.yt * 4)) == NULL)
    return (1);
  return (0);
}

static
outwayflag ()
{
  char flag, flag2;
  int i, j, k, l, d, tmp, tmp2;

  _fmemset (outwaymap, 0, hplaninfo.xt * hplaninfo.yt * 4);
  _fmemset (hdradar, 0, hplaninfo.xt * hplaninfo.yt * 4);
  _fmemset (hdcolli, 0, hplaninfo.xt * hplaninfo.yt * 4);

  j = 0;
  l = 0;
  for (k = 0; k < hplaninfo.yt; k++) {
    for (i = 0; i < hplaninfo.xt; i++) {
      (*hdradar)[j] = (*level_map)[i + l].collision[0];
      (*hdradar)[j + 1] = (*level_map)[i + l].collision[1];
      (*hdradar)[j + hplaninfo.xt * 2] = (*level_map)[i + l].collision[2];
      (*hdradar)[j + hplaninfo.xt * 2 + 1] = (*level_map)[i + l].collision[3];
      j += 2;
    }
    j += hplaninfo.xt * 2;
    l += hplaninfo.xt;
  }
  if (hplaninfo.ywrap != -1) {
    for (i = 0; i < hplaninfo.xt * 2; i++) {
      if ((*hdradar)[(hplaninfo.ywrap * 2 + 1) * hplaninfo.xt * 2 + i] &
	  c_down) (*hdcolli)[i] |= d_up;
      if ((*hdradar)[i] & c_up)
	(*hdcolli)[(hplaninfo.ywrap * 2 + 1) * hplaninfo.xt * 2 + i] |= d_down;
    }
  } else
    for (i = 0; i < hplaninfo.xt * 2; i++) {
      (*hdcolli)[i] |= d_up;
      (*hdcolli)[(hplaninfo.yt * 2 - 1) * hplaninfo.xt * 2 + i] |= d_down;
    };
  if (hplaninfo.xwrap != -1) {
    for (i = 0; i < hplaninfo.yt * 2; i++) {
      if ((*hdradar)[i * hplaninfo.xt * 2 + hplaninfo.xwrap * 2 + 1] &
	  c_right) (*hdcolli)[i * hplaninfo.xt * 2] |= d_left;
      if ((*hdradar)[i * hplaninfo.xt * 2] & c_left)
	(*hdcolli)[hplaninfo.xwrap * 2 + 1 + i * hplaninfo.xt * 2] |= d_right;
    }
  } else
    for (i = 0; i < hplaninfo.yt * 2; i++) {
      (*hdcolli)[i * hplaninfo.xt * 2] |= d_left;
      (*hdcolli)[hplaninfo.xt * 2 - 1 + i * hplaninfo.xt * 2] |= d_right;
    };
  j = 0;
  for (k = hplaninfo.yt * 2 - 1; k != 0; k--) {
    for (i = 0; i < hplaninfo.xt * 2; i++) {
      if ((*hdradar)[j + i] & c_down)
	(*hdcolli)[j + i + hplaninfo.xt * 2] |= d_up;
      if ((*hdradar)[j + i + hplaninfo.xt * 2] & c_up)
	(*hdcolli)[j + i] |= d_down;
    }
    j += hplaninfo.xt * 2;
  }
  j = 0;
  for (k = hplaninfo.yt * 2; k != 0; k--) {
    for (i = 0; i < hplaninfo.xt * 2 - 1; i++) {
      if ((*hdradar)[j + i] & c_right)
	(*hdcolli)[j + i + 1] |= d_left;
      if ((*hdradar)[j + i + 1] & c_left)
	(*hdcolli)[j + i] |= d_right;
    }
    j += hplaninfo.xt * 2;
  }

  for (k = 0; k < 4; k++) {
    l = hplaninfo.start_way[k] & 15;
    i = (hplaninfo.start[k] % hplaninfo.xt) * 2;
    j = (hplaninfo.start[k] / hplaninfo.xt) * 2;
    if (l == 1)
      i++;
    else if (l == 2)
      j++;
    else if (l == 3) {
      i++;
      j++;
    }
//                outwayrecurs(i+j*hplaninfo.xt*2,i,j);
    (*outwaymap)[i + j * hplaninfo.xt * 2] = 1;
  }

  k = 0;
  l = 0;
  for (j = 0; j < hplaninfo.yt; j++) {
    for (i = 0; i < hplaninfo.xt; i++, l++)
      if ((*level_map)[l].type == t_tunnel) {
	(*outwaymap)[i * 2 + k] = 1;
	(*outwaymap)[i * 2 + k + 1] = 1;
	(*outwaymap)[i * 2 + k + hplaninfo.xt * 2] = 1;
	(*outwaymap)[i * 2 + k + hplaninfo.xt * 2 + 1] = 1;

//                                outwayrecurs(i*2+k,i*2,j*2);
//                                outwayrecurs(i*2+k+1,i*2+1,j*2);
//                                outwayrecurs(i*2+k+hplaninfo.xt*2,i*2,j*2+1);
//                                outwayrecurs(i*2+k+hplaninfo.xt*2+1,i*2+1,j*2+1);
      }
    k += hplaninfo.xt << 2;
  }

  do {
    flag = 0;
    for (k = 0, j = 0; k < 2 * hplaninfo.yt; k++, j += 2 * hplaninfo.xt)
      for (i = 0; i < 2 * hplaninfo.xt; i++) {
	d = j + i;
	if ((*outwaymap)[d] == 1) {
	  flag2 = 0;
	  if (!((*hdcolli)[d] & d_up)) {
	    tmp = (k - 1) & ((hplaninfo.ywrap << 1) + 1);
	    tmp2 = i + tmp * hplaninfo.xt * 2;
	    if (!(*outwaymap)[tmp2]) {
	      (*outwaymap)[tmp2] = 1;
	      flag2 = 1;
	    }
	  }
	  if (!((*hdcolli)[d] & d_down)) {
	    tmp = (k + 1) & ((hplaninfo.ywrap << 1) + 1);
	    tmp2 = i + tmp * hplaninfo.xt * 2;
	    if (!(*outwaymap)[tmp2]) {
	      (*outwaymap)[tmp2] = 1;
	      flag2 = 1;
	    }
	  }
	  if (!((*hdcolli)[d] & d_right)) {
	    tmp = (i + 1) & ((hplaninfo.xwrap << 1) + 1);
	    tmp2 = tmp + k * hplaninfo.xt * 2;
	    if (!(*outwaymap)[tmp2]) {
	      (*outwaymap)[tmp2] = 1;
	      flag2 = 1;
	    }
	  }
	  if (!((*hdcolli)[d] & d_left)) {
	    tmp = (i - 1) & ((hplaninfo.xwrap << 1) + 1);
	    tmp2 = tmp + k * hplaninfo.xt * 2;
	    if (!(*outwaymap)[tmp2]) {
	      (*outwaymap)[tmp2] = 1;
	      flag2 = 1;
	    }
	  }
	  if (flag2 == 0)
	    (*outwaymap)[d] = 2;
	  else
	    flag = 1;
//        flag|=flag2;
	}


      }
  } while (flag);

  k = 0;
  l = 0;
  for (j = 0; j < hplaninfo.yt; j++) {
    for (i = 0; i < hplaninfo.xt; i++, l++) {
      if (!((*outwaymap)[i * 2 + k]))
	(*level_map)[l].collision[0] = 15;
      if (!((*outwaymap)[i * 2 + k + 1]))
	(*level_map)[l].collision[1] = 15;
      if (!((*outwaymap)[(i + hplaninfo.xt) * 2 + k]))
	(*level_map)[l].collision[2] = 15;
      if (!((*outwaymap)[(i + hplaninfo.xt) * 2 + k + 1]))
	(*level_map)[l].collision[3] = 15;
      if (!((*outwaymap)[i * 2 + k] || (*outwaymap)[i * 2 + 1 + k]
	    || (*outwaymap)[i * 2 + hplaninfo.xt * 2 + k]
	    || (*outwaymap)[i * 2 + hplaninfo.xt * 2 + 1 + k])) {
	if ((*level_map)[l].type != t_anim)
	  (*level_map)[l].type = t_outway;
	(*level_map)[l].collision[0] = 15;
	(*level_map)[l].collision[1] = 15;
	(*level_map)[l].collision[2] = 15;
	(*level_map)[l].collision[3] = 15;
      }
    }
    k += hplaninfo.xt << 2;
  }

}

static
outwayclose ()
{
  free (outwaymap);
  free (hdradar);
  free (hdcolli);
}

static
joueanim ()
{
  int i, j;
  int k, l, m, n;
  int tmp;
  char *dest;
  int xx, yy, t = 0;

  do {
    dest = screen;
    yy = 7;
    vsynchro ();
    for (k = yplan, l = 10; l > 0; l--, k = ((k + 1) & hplaninfo.ywrap)) {
      m = k * hplaninfo.xt;
      xx = 12;
      for (i = xplan, j = 6; j > 0; j--, i = ((i + 1) & hplaninfo.xwrap)) {
	if ((*level_map)[i + m].type == t_anim ||
	    (((*level_map)[i + m].type == t_speed ||
	      (*level_map)[i + m].type == t_boom ||
	      (*level_map)[i + m].type == t_stop ||
	      (*level_map)[i + m].type == t_ice ||
	      (*level_map)[i + m].type == t_outway ||
	      (*level_map)[i + m].type == t_dust)
	     && ((*level_map)[i + m].info.param[4] & 0xf0) != 0)) {
	  if ((*level_map)[i + m].type == t_anim)
	    copy_tile ((*level_map)[i + m].number +
		       24 * ((t / ((*level_map)[i + m].info.anim.speed + 1)) %
			     ((*level_map)[i + m].info.anim.frame_nbr + 1)),
		       dest);
	  else {
	    tmp =
	      ((t / (((*level_map)[i + m].info.param[4] & 15) + 1)) %
	       (((*level_map)[i + m].info.param[4] >> 4) * 2));
	    if (tmp > ((*level_map)[i + m].info.param[4] >> 4) /*+1 */ )
	      tmp = (((*level_map)[i + m].info.param[4] >> 4) * 2) - tmp;
	    copy_tile ((*level_map)[i + m].number + 24 * tmp, dest);
	  }
	  if (sprhide == 0) {
	    for (n = 0; n < 4; n++)
	      if (i + m == hplaninfo.start[n])
		copy_square_transp (heditrsc.buffer + (16 + (n << 4)) * 320 +
				    256 + (hplaninfo.start_way[n] & 0xf0),
				    dest +
				    square_offset_320[hplaninfo.
						      start_way[n] & 0xf]);
	    if ((*level_map)[i + m].sprite != 0)
	      copy_tile_transp ((*level_map)[i + m].sprite, dest);
	  }
	  if (afftests)
	    for (n = 0; n < 4; n++)
	      copy_square_transp (heditrsc.buffer + 10 * 320 + 30 +
				  (*level_map)[i + m].collision[n] * 12,
				  dest + square_offset_320[n]);
	  if (i + m == tempd)
	    cadrept (xx - 12, yy - 7, 24, 20, 8, 15);
	}
	dest += 24;
	xx += 24;
      }
      dest += 20 * 320 - 24 * 6;
      yy += 20;
    }
    t++;
  }
  while (mouse12 () == 0 && key_ready () == 0);

  majg ();
  majd ();
}

static
gestclav (int i)
{
  char t;
  int j, k;
  switch (i) {
  case 0x3b00:
    modevga (TEXT);
//                  spawnl(P_WAIT,"READER.EXE","READER.EXE","HEDLITE.DOC",NULL);
//                  spawnl(P_WAIT,"MEM.EXE",NULL);
    rmain (2, "txt_cfg\\hedlite_.doc", "hedlite.doc");
    modevga (G320x200x256);
    set_pal ((char *) &tile_set_img.palette, 0, 256 * 3);
    partiel2 (0, 0, 30, 200, 290, 0, &heditrsc);
    draw_text (&levelnomshort, 305, 29, 8, 1);
    ultoa (hplaninfo.xt, &nombre, 10);
    draw_text (&nombre, 302, 43, 8, 2);
    ultoa (hplaninfo.yt, &nombre, 10);
    draw_text (&nombre, 307, 43, 8, 0);
    majd ();
    majg ();
    break;
  case 0x7400:
    if (xdalles + 168 < tile_set_img.xt) {
      xdalles += 24;
      majd ();
    } else if (xdallesdec < 120) {
      xdallesdec += 24;
      majd ();
    }
    break;
  case 0x7300:
    if (xdalles > 0) {
      xdalles -= 24;
      majd ();
    } else if (xdallesdec > 0) {
      xdallesdec -= 24;
      majd ();
    }
    break;
  case 0x7700:
    xdalles = 0;
    majd ();
    break;
  case 0x7500:
    xdalles = (tile_set_img.xt / 24) * 24 - 144;
    majd ();
    break;
  case 0x7600:
    if (ydalles < 180) {
      ydalles += 20;
      majd ();
    } else if (xdallesdec < 120) {
      xdallesdec += 24;
      majd ();
    }
    break;
  case 0x8400:
    if (ydalles > 0) {
      ydalles -= 20;
      majd ();
    } else if (xdallesdec > 0) {
      xdallesdec -= 24;
      majd ();
    }
    break;
  case 0x4d00:
    if (xplan < (hplaninfo.xt - 6) || hplaninfo.xwrap != -1) {
      xplan = ((xplan + 1) & hplaninfo.xwrap);
      majg ();
    } else
      gestclav (0x4d36);
    break;
  case 0x4b00:
    if (xplan > 0 || hplaninfo.xwrap != -1) {
      xplan = ((xplan - 1) & hplaninfo.xwrap);
      majg ();
    } else
      gestclav (0x4b34);
    break;
  case 0x5000:
    if (yplan < (hplaninfo.yt - 10) || hplaninfo.ywrap != -1) {
      yplan = ((yplan + 1) & hplaninfo.ywrap);
      majg ();
    } else
      gestclav (0x5032);
    break;
  case 0x4800:
    if (yplan > 0 || hplaninfo.ywrap != -1) {
      yplan = ((yplan - 1) & hplaninfo.ywrap);
      majg ();
    } else
      gestclav (0x4838);
    break;
  case 0x4d36:
    if (xplandec < 120)
      xplandec += 24;
    else if (xplan < (hplaninfo.xt - 6) || hplaninfo.xwrap != -1) {
      xplan = ((xplan + 1) & hplaninfo.xwrap);
    }
    majg ();
    break;
  case 0x4b34:
    if (xplandec > 0)
      xplandec -= 24;
    else if (xplan > 0 || hplaninfo.xwrap != -1) {
      xplan = ((xplan - 1) & hplaninfo.xwrap);
    }
    majg ();
    break;
  case 0x5032:
    if (yplandec < 180)
      yplandec += 20;
    else if (yplan < (hplaninfo.yt - 10) || hplaninfo.ywrap != -1) {
      yplan = ((yplan + 1) & hplaninfo.ywrap);
    }
    majg ();
    break;
  case 0x4838:
    if (yplandec > 0)
      yplandec -= 20;
    else if (yplan > 0 || hplaninfo.ywrap != -1) {
      yplan = ((yplan - 1) & hplaninfo.ywrap);
    }
    majg ();
    break;
//     case 0x0f09: cote^=1;majd();majg();
//                  break;
  case 0x1c0d:
    planfull ();
    break;
  case 0x3920:			//if (((*etatclav)&3)==0)
    //{
    j = curdallep ();
    (*level_map)[j].number = xdalles + xdallesdec + ydalles * (tile_set_img.xt);
    (*level_map)[j].type = (*ddef)[curdalled ()].type;
    (*level_map)[j].info = (*ddef)[curdalled ()].info;
    /*
       (*level_map)[j].collision[0]&=(c_left|c_up);
       (*level_map)[j].collision[1]&=(c_right|c_up);
       (*level_map)[j].collision[2]&=(c_left|c_down);
       (*level_map)[j].collision[3]&=(c_right|c_down);
       (*level_map)[j].collision[0]|=(*ddef)[curdalled()].collision[0];
       (*level_map)[j].collision[1]|=(*ddef)[curdalled()].collision[1];
       (*level_map)[j].collision[2]|=(*ddef)[curdalled()].collision[2];
       (*level_map)[j].collision[3]|=(*ddef)[curdalled()].collision[3];
       if (curdallepg>0)
       { if ((*ddef)[curdalled()].collision[0]&c_left) (*level_map)[curdallepg(j)].collision[1]|=c_right;
       if ((*ddef)[curdalled()].collision[2]&c_left) (*level_map)[curdallepg(j)].collision[3]|=c_right;}
       if (curdalleph>0)
       { if ((*ddef)[curdalled()].collision[0]&c_up) (*level_map)[curdalleph(j)].collision[2]|=c_down;
       if ((*ddef)[curdalled()].collision[1]&c_up) (*level_map)[curdalleph(j)].collision[3]|=c_down;}
       if (curdallepd>0)
       { if ((*ddef)[curdalled()].collision[1]&c_right) (*level_map)[curdallepd(j)].collision[0]|=c_left;
       if ((*ddef)[curdalled()].collision[3]&c_right) (*level_map)[curdallepd(j)].collision[2]|=c_left;}
       if (curdallepb>0)
       { if ((*ddef)[curdalled()].collision[2]&c_down) (*level_map)[curdallepb(j)].collision[0]|=c_up;
       if ((*ddef)[curdalled()].collision[3]&c_down) (*level_map)[curdallepb(j)].collision[1]|=c_up;}
       } else
       if (tempd!=0xfffffff)
       (*level_map)[curdallep()]=(*level_map)[tempd];
     */
    gestclav (0x1769);
    gestclav (0x184f);
    majg ();
    break;
  case 0x1769:			// I
  case 0x1749:			//if (i==0x1769) fprintf(hlog,"\t[i] used\n");
    // else          fprintf(hlog,"\t[I] used\n");
    for (j = hplaninfo.xt * hplaninfo.yt - 1; j >= 0; j--) {
      (*level_map)[j].collision[0] = 0;
      (*level_map)[j].collision[1] = 0;
      (*level_map)[j].collision[2] = 0;
      (*level_map)[j].collision[3] = 0;
    }
    for (j = hplaninfo.xt * hplaninfo.yt - 1; j >= 0; j--) {
      k =
	(((*level_map)[j].number % tile_set_img.xt) / 24) +
	((*level_map)[j].number / (tile_set_img.xt * 20)) * (tile_set_img.xt /
							     24);
      (*level_map)[j].type = (*ddef)[k].type;
      if ((*level_map)[j].type != t_tunnel) {
	if ((*level_map)[j].type == t_anim || i == 0x1749)
	  (*level_map)[j].info = (*ddef)[k].info;
	else {
	  (*level_map)[j].info.param[0] = (*ddef)[k].info.param[0];
	  (*level_map)[j].info.param[1] = (*ddef)[k].info.param[1];
	  (*level_map)[j].info.param[2] = (*ddef)[k].info.param[2];
	  (*level_map)[j].info.param[3] = (*ddef)[k].info.param[3];
	}
      } else
	(*level_map)[j].info.tunnel.direction = (*ddef)[k].info.tunnel.direction;
      (*level_map)[j].collision[0] &= (c_left | c_up);
      (*level_map)[j].collision[1] &= (c_right | c_up);
      (*level_map)[j].collision[2] &= (c_left | c_down);
      (*level_map)[j].collision[3] &= (c_right | c_down);
      (*level_map)[j].collision[0] |= (*ddef)[k].collision[0];
      (*level_map)[j].collision[1] |= (*ddef)[k].collision[1];
      (*level_map)[j].collision[2] |= (*ddef)[k].collision[2];
      (*level_map)[j].collision[3] |= (*ddef)[k].collision[3];
      if (curdallepg (j) > 0) {
	if ((*ddef)[k].collision[0] & c_left)
	  (*level_map)[curdallepg (j)].collision[1] |= c_right;
	if ((*ddef)[k].collision[2] & c_left)
	  (*level_map)[curdallepg (j)].collision[3] |= c_right;
      }
      if (curdalleph (j) > 0) {
	if ((*ddef)[k].collision[0] & c_up)
	  (*level_map)[curdalleph (j)].collision[2] |= c_down;
	if ((*ddef)[k].collision[1] & c_up)
	  (*level_map)[curdalleph (j)].collision[3] |= c_down;
      }
      if (curdallepd (j) > 0) {
	if ((*ddef)[k].collision[1] & c_right)
	  (*level_map)[curdallepd (j)].collision[0] |= c_left;
	if ((*ddef)[k].collision[3] & c_right)
	  (*level_map)[curdallepd (j)].collision[2] |= c_left;
      }
      if (curdallepb (j) > 0) {
	if ((*ddef)[k].collision[2] & c_down)
	  (*level_map)[curdallepb (j)].collision[0] |= c_up;
	if ((*ddef)[k].collision[3] & c_down)
	  (*level_map)[curdallepb (j)].collision[1] |= c_up;
      }
    }
//                  majg();
    break;
  case 0x1c0a:
    joueanim ();
    break;
  case 0x2106:
    for (j = hplaninfo.xt * hplaninfo.yt - 1; j >= 0; j--) {
      (*level_map)[j].number = xdalles + xdallesdec + ydalles * (tile_set_img.xt);
      (*level_map)[j].type = (*ddef)[curdalled ()].type;
      (*level_map)[j].info = (*ddef)[curdalled ()].info;
    }
    break;
  case 0x2100:
    for (j = hplaninfo.xt * hplaninfo.yt - 1; j >= 0; j--)

      (*level_map)[j].number =
	(((j % hplaninfo.xt) + (j / hplaninfo.xt)) & 1) * 20 *
	tile_set_img.xt;
    majg ();
    break;
  case 0x3d00:
    sprhide ^= 1;
    majg ();
    break;
  case 0x4000:
    afftests ^= 1;
    majg ();
    break;
  case 0x1f73:			// S
  case 0x1f53:
    if ((*level_map)[curdallep ()].sprite == 0)

      (*level_map)[curdallep ()].sprite =
	(short int) (xdalles + xdallesdec + ydalles * (tile_set_img.xt));
    else
      (*level_map)[curdallep ()].sprite = 0;
    majg ();
    break;
  case 0x2064:			// D
  case 0x2044:
    t = (*level_map)[curdallep ()].type;
    if (t != t_boom && t != t_anim && t != t_outway)
      departfix ();
    break;
  case 0x1474:			// T
  case 0x1454:
    if (tempd == curdallep ())
      tempd = 0xffffffff;
    else
      tempd = curdallep ();
    majg ();
    break;
  case 0x184f:			// O
  case 0x186f:			//fprintf(hlog,"\t[O] used\n");
    outwayflag ();		/*majg(); */
    break;
  case 0x1970:			// P
  case 0x1950:
    save_pcx ();
    majg ();
    break;
  default:
    if ((i & 0xff00) == 0xb00)
      i = 0x150;
    if (i >= 0x150 && i <= 0xa00) {
      (*level_map)[curdallep ()].type = (i >> 8) - 1;
      (*level_map)[curdallep ()].info.tunnel.output = 0;
      (*level_map)[curdallep ()].info.param[4] = 0;
      majg ();
    }
    break;
  }
}

/*
static rect(x,y,xt,yt,c)
unsigned short int x,y,xt,yt;
unsigned char c;
{
 int j;
 char *dest=screen+x+y*320;

 _fmemset(dest,15,xt);
 dest+=320;
 for (j=yt-2;j>0;j--)
 {
   *dest++=15;
   _fmemset(dest,c,xt-2);
   *(dest+xt-2)=15;
   dest+=319;
 }
 _fmemset(dest,15,xt);
}
*/
static
gestsrs1 ()
{
  int x = mousex (), y = mousey (), i, x2, y2;
  i = curdallep ();

  if (x >= 290) {
    if (y < 19)
      gestclav (0x1c0d);
    if (y >= 144 && y <= 166) {
      if (x > 305)
	gestclav (0x7400);
      else
	gestclav (0x7300);
    }
/*            if (y>=71 && y<=84)
                                              { (*level_map)[i].type=menutype((*level_map)[i].type);
                                                (*level_map)[curdallep()].info.tunnel.output=0;
                                                (*level_map)[curdallep()].info.param[4]=0;
                                                majg(); majd(); }
*/
    if (y >= 86 && y <= 137) {
      (*fnptr[(*level_map)[i].type]) (i, x, y);
      majg ();
    }
    if (y >= 168) {
      if (((*level_map)[i].type == t_speed ||
	   (*level_map)[i].type == t_ice ||
	   (*level_map)[i].type == t_stop ||
	   (*level_map)[i].type == t_dust ||
	   (*level_map)[i].type == t_outway
	   || (*level_map)[i].type == t_boom)) {
	anim_mod_bcl (i, x, y);
	majg ();
      }
    }
  }

  if (x < 144) {
    xplandec = (x / 24) * 24;
    yplandec = (y / 20) * 20;
    majg ();
    if (y < 4) {
      setmouse (x, 8);
      y = 8;
    }
    if (y > 195) {
      setmouse (x, 192);
      y = 192;
    }
    if (x < 4) {
      setmouse (8, y);
      x = 8;
    }
    do {
      x2 = mousex ();
      y2 = mousey ();
      if (x2 - x > 3) {
	setmouse (x, y);
	gestclav (0x4d00);
      } else if (x - x2 > 3) {
	setmouse (x, y);
	gestclav (0x4b00);
      }
      if (y2 - y > 3) {
	setmouse (x, y);
	gestclav (0x5000);
      } else if (y - y2 > 3) {
	setmouse (x, y);
	gestclav (0x4800);
      }
    } while (mouse12 () != 0);
  }
  if (x > 144 && x < 289) {
    xdallesdec = ((x - 145) / 24) * 24;
    ydalles = (y / 20) * 20;
    majd ();
    if (y < 4) {
      setmouse (x, 8);
      y = 8;
    }
    if (y > 195) {
      setmouse (x, 192);
      y = 192;
    }
    if (x < 4) {
      setmouse (8, y);
      x = 8;
    }
    do {
      x2 = mousex ();
      y2 = mousey ();
      if (x2 - x > 3) {
	setmouse (x, y);
	gestclav (0x7400);
      } else if (x - x2 > 3) {
	setmouse (x, y);
	gestclav (0x7300);
      }
      if (y2 - y > 3) {
	setmouse (x, y);
	gestclav (0x7600);
      } else if (y - y2 > 3) {
	setmouse (x, y);
	gestclav (0x8400);
      }
    } while (mouse12 () != 0);
  }

}

static
gestsrs2 ()
{
  int x = mousex (), y = mousey ();
  if (x >= 290) {
    if (y >= 144 && y <= 166)
      if (x > 305)
	gestclav (0x7500);
      else
	gestclav (0x7700);
//            if (y>=86 && y<=108) gestclav(0x2e63);
  }
  if (x < 144) {
    xplandec = (x / 24) * 24;
    yplandec = (y / 20) * 20;
    gestclav (0x3920);
  }
}

static
gestsrs3 ()
{
  int x = mousex (), y = mousey ();
  int a, b;
  if (x < 144) {
    a = xplandec;
    b = yplandec;
    xplandec = (x / 24) * 24;
    yplandec = (y / 20) * 20;
    gestclav (0x1474);
    xplandec = a;
    yplandec = b;
    majg ();
  }

}

static
qwritel (char *phr, int x, int y, unsigned char attrib, int l)
{
  int t;
  qwrite (phr, x, y, attrib);
  t = strlen (phr);
  ligne[1] = l - t;
  qwritep (ligne, x + t, y, attrib);
}

static
select (char *quoi, char *resultat)
{
  struct find_t dirinfo;
  int i = 0, j = 0, k = 0, l, t;
  char (*tableau)[][13];
  char *scrbak;

  scrbak = malloc (8000);
  _fmemcpy (scrbak, screentxt, 8000);
  if (!_dos_findfirst (quoi, 0, (struct find_t *) &dirinfo)) {
    do
      i++;
    while (!_dos_findnext ((struct find_t *) &dirinfo));
  }
  tableau = malloc (13 * (i + 1));
  if (!_dos_findfirst (quoi, 0, (struct find_t *) &dirinfo)) {
    do
      strcpy ((char *) &((*tableau)[j++]), (char *) &dirinfo.name);
    while (!_dos_findnext ((struct find_t *) &dirinfo) && !(j > i));
  }
  i = 0;
  qsort (tableau, j, 13, &strcmp);
  txtcadref (60, 0, 75, 49);
  do {
    if (i < 24)
      k = 0;
    else {
      if (i + 24 > j) {
	if (j > 47)
	  k = j - 48;
	else
	  k = 0;
      } else
	k = i - 24;
    }
    for (l = 0; l < 48; l++)
      if (k + l < j) {
	qwrite ("\x1a", 61, 1 + l, 112 + 7 + 5 * (i == k + l));
	qwrite ((char *) &((*tableau)[k + l]), 62, 1 + l,
		14 + 112 - 64 * (i == k + l));
	t = strlen ((char *) &((*tableau)[k + l]));
	ligne[1] = 12 - t;
	qwritep (ligne, 62 + t, 1 + l, 14 + 112 - 64 * (i == k + l));
	qwrite ("\x1b", 74, 1 + l, 112 + 7 + 5 * (i == k + l));
      }
    t = get_key ();
    switch (t) {
    case 0x5000:
      if (i + 1 < j)
	i++;
      break;
    case 0x4800:
      if (i > 0)
	i--;
      break;
    case 0x5100:
      if (i + 10 < j)
	i += 10;
      else
	i = j - 1;
      break;
    case 0x4900:
      if (i > 10)
	i -= 10;
      else
	i = 0;
      break;
    case 0x4700:
      i = 0;
      break;
    case 0x4f00:
      i = j - 1;
      break;
    }
  }
  while (t != 0x1c0d && t != 0x11b);
  if (t == 0x11b)
    resultat[0] = 0;
  else
    strcpy (resultat, (char *) &((*tableau)[i]));
  free (tableau);
  _fmemcpy (screentxt, scrbak, 8000);
  free (scrbak);

}

static
boiteask (char *question, char *entree, int taille)
{
  char *scrbak;
  int l, x, t, i;
  char tmp[256];

  scrbak = malloc (8000);
  _fmemcpy (scrbak, screentxt, 8000);
  l = strlen (question);
  strcpy ((char *) &tmp, entree);
  if (l < taille)
    l = taille;
  x = 40 - ((l + 4) >> 1);
  txtcadref (x, 22, x + l + 3, 28);
  l = strlen ((char *) &tmp);
  qwrite (question, x + 2, 24, 14 + 112);
  do {
    i = strlen ((char *) &tmp);
    ligne[1] = taille - i;
    cursor (x + 2 + l, 26);
    qwrite ((char *) &tmp, x + 2, 26, 15 + 16);
    qwritep (ligne, x + 2 + i, 26, 15 + 16);
    t = get_key ();
    switch (t) {
    case 0xe08:
      if (l > 0) {
	l--;
	tmp[l] = 0;
      }
      break;
    case 0x1c0d:
    case 0x11b:
      break;
    default:
      if ((char) t > 32 && l < taille) {
	tmp[l] = (char) t;
	l++;
	tmp[l] = 0;
      }
      break;
    }
  } while (t != 0x1c0d && t != 0x11b);
  if (t == 0x1c0d)
    strcpy (entree, (char *) &tmp);
  _fmemcpy (screentxt, scrbak, 8000);
  free (scrbak);
  cursor (0, 50);
}

static char
asknew ()
{
  char *scrbak;
  int l = 0, i, j;

  scrbak = malloc (8000);
  _fmemcpy (scrbak, screentxt, 8000);

  txtcadref (29, 20, 51, 31);
  do {
    do {
      qwrite (" Name   : ", 31, 22, 14 + 112 - 64 * (l == 0));
      qwrite (" Tiles  : ", 31, 23, 14 + 112 - 64 * (l == 1));
      qwrite (" Module : ", 31, 24, 14 + 112 - 64 * (l == 2));
      qwrite (" X-Size : ", 31, 25, 14 + 112 - 64 * (l == 3));
      qwrite (" X-Wrap : ", 31, 26, 14 + 112 - 64 * (l == 4));
      qwrite (" Y-Size : ", 31, 27, 14 + 112 - 64 * (l == 5));
      qwrite (" Y-Wrap : ", 31, 28, 14 + 112 - 64 * (l == 6));
      qwrite ("-=ð[OK]ð=-", 31, 29, 14 + 112 - 64 * (l == 7));
      qwritel (&levelnomshort, 42, 22, 10 + 112, 8);
      qwritel (&hplaninfo.tile_set_name, 42, 23, 10 + 112, 8);
      qwritel (&hplaninfo.soundtrack_name, 42, 24, 10 + 112, 8);
      ultoa (hplaninfo.xt, &nombre, 10);
      qwritel (&nombre, 42, 25, 10 + 112, 5);
      qwrite (((hplaninfo.xwrap != -1) ? "Yes" : "No "), 42, 26, 10 + 112);
      ultoa (hplaninfo.yt, &nombre, 10);
      qwritel (&nombre, 42, 27, 10 + 112, 5);
      qwrite (((hplaninfo.ywrap != -1) ? "Yes" : "No "), 42, 28, 10 + 112);
      i = get_key ();
      if (i == 0x5000)
	l = (l + 1) & 7;
      if (i == 0x4800)
	l = (l - 1) & 7;
      if (i == 0x1c0d)
	switch (l) {
	case 0:
	  boiteask ("Enter level's name:", &levelnomshort, 8);
	  remove_comments (&levelnomshort);
	  strlwr (&levelnomshort);
	  break;
	case 1:
	  select (rscdir "\\*.pcx", &entree);
	  remove_comments (&entree);
	  if (entree[0] != 0)
	    strcpy (&hplaninfo.tile_set_name, &entree);
	  break;
	case 2:
	  select (moddir "\\*.xm", &entree);
	  remove_comments (&entree);
	  if (entree[0] != 0)
	    strcpy (&hplaninfo.soundtrack_name, &entree);
	  break;
	case 3:
	  ultoa (hplaninfo.xt, &entree, 10);
	  boiteask ("Enter X-Size:", &entree, 5);
	  remove_comments (&entree);
	  j = atol (&entree);
	  if (j > 0) {
	    if (j < 15) {
	      if ((j & (j - 1)) == 0) {
		hplaninfo.xt = j;
		hplaninfo.xwrap = j - 1;
	      }
	    } else {
	      hplaninfo.xt = j;
	      hplaninfo.xwrap = j - 1;
	      if ((j & (j - 1)) != 0)
		hplaninfo.xwrap = -1;
	    }
	  }
	  break;
	case 4:
	  if (hplaninfo.xwrap == -1
	      && (hplaninfo.xt & (hplaninfo.xt - 1)) == 0) hplaninfo.xwrap =
	      (hplaninfo.xt - 1);
	  else if (hplaninfo.xwrap != -1 && hplaninfo.xt >= 13)
	    hplaninfo.xwrap = -1;
	  break;
	case 5:
	  ultoa (hplaninfo.yt, &entree, 10);
	  boiteask ("Enter Y-Size:", &entree, 5);
	  remove_comments (&entree);
	  j = atol (&entree);
	  if (j > 0) {
	    if (j < 11) {
	      if ((j & (j - 1)) == 0) {
		hplaninfo.yt = j;
		hplaninfo.ywrap = j - 1;
	      }
	    } else {
	      hplaninfo.yt = j;
	      hplaninfo.ywrap = j - 1;
	      if ((j & (j - 1)) != 0)
		hplaninfo.ywrap = -1;
	    }
	  }
	  break;
	case 6:
	  if (hplaninfo.ywrap == -1
	      && (hplaninfo.yt & (hplaninfo.yt - 1)) == 0) hplaninfo.ywrap =
	      (hplaninfo.yt - 1);
	  else if (hplaninfo.ywrap != -1 && hplaninfo.yt >= 10)
	    hplaninfo.ywrap = -1;
	  break;
	}
    } while (i != 0x1c0d && i != 0x11b);
  } while (l != 7 && i != 0x11b);

  if (i == 0x11b)
    _fmemcpy (screentxt, scrbak, 8000);
  free (scrbak);
  if (i == 0x11b)
    return (0);
  else
    return (1);
}

static char
askold ()
{
  char *scrbak;
  int l = 0, i;

  scrbak = malloc (8000);
  _fmemcpy (scrbak, screentxt, 8000);

  txtcadref (29, 21, 51, 30);
  do {
    do {
      qwrite (" Name   : ", 31, 23, 14 + 112);
      qwrite (" Tiles  : ", 31, 24, 14 + 112 - 64 * (l == 0));
      qwrite (" Module : ", 31, 25, 14 + 112 - 64 * (l == 1));
      qwrite (" X-Wrap : ", 31, 26, 14 + 112 - 64 * (l == 2));
      qwrite (" Y-Wrap : ", 31, 27, 14 + 112 - 64 * (l == 3));
      qwrite ("-=ð[OK]ð=-", 31, 28, 14 + 112 - 64 * (l == 4));
      qwritel (&levelnomshort, 42, 23, 10 + 112, 8);
      qwritel (&hplaninfo.tile_set_name, 42, 24, 10 + 112, 8);
      qwritel (&hplaninfo.soundtrack_name, 42, 25, 10 + 112, 8);
      qwrite (((hplaninfo.xwrap != -1) ? "Yes" : "No "), 42, 26, 10 + 112);
      qwrite (((hplaninfo.ywrap != -1) ? "Yes" : "No "), 42, 27, 10 + 112);
      i = get_key ();
      if (i == 0x5000)
	l = ((l == 4) ? 0 : (l + 1));
      if (i == 0x4800)
	l = ((l == 0) ? 4 : (l - 1));
      if (i == 0x1c0d)
	switch (l) {
	case 0:
	  select (rscdir "\\*.pcx", &entree);
	  remove_comments (&entree);
	  if (entree[0] != 0)
	    strcpy (&hplaninfo.tile_set_name, &entree);
	  break;
	case 1:
	  select (moddir "\\*.xm", &entree);
	  remove_comments (&entree);
	  if (entree[0] != 0)
	    strcpy (&hplaninfo.soundtrack_name, &entree);
	  break;
	case 2:
	  if (hplaninfo.xwrap == -1
	      && (hplaninfo.xt & (hplaninfo.xt - 1)) == 0) hplaninfo.xwrap =
	      (hplaninfo.xt - 1);
	  else if (hplaninfo.xwrap != -1 && hplaninfo.xt >= 15)
	    hplaninfo.xwrap = -1;
	  break;
	case 3:
	  if (hplaninfo.ywrap == -1
	      && (hplaninfo.yt & (hplaninfo.yt - 1)) == 0) hplaninfo.ywrap =
	      (hplaninfo.yt - 1);
	  else if (hplaninfo.ywrap != -1 && hplaninfo.yt >= 11)
	    hplaninfo.ywrap = -1;
	  break;
	}
    } while (i != 0x1c0d && i != 0x11b);
  } while (l != 4 && i != 0x11b);

  _fmemcpy (screentxt, scrbak, 8000);
  free (scrbak);
  if (i == 0x11b)
    return (0);
  else
    return (1);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
hmain (int argc, char *argv1, char *argv2, char *argv3, char *argv4,
       char *argv5, char *argv6)
{
  int i;
  char l = 0;
  char okgo;


  mkdir ("EXTRALVL");
  if (argc != 7) {
    if (mouseinit () != -1)
      fatalog ("Geme a mousy mouse pleez.");
    modevga (TEXT);
    lines50 ();
    cursor (0, 50);
    if ((ftmp = fopen (heddir "HEDITFRM.BIN", "rb")) == NULL)
      fatalog ("Can't open HEDITFRM.BIN");
    fread (screentxt, 8000, 1, ftmp);
    fclose (ftmp);
  }
  for (;;) {
    decalx = 1;
    decaly = 0;
    if (argc != 7) {
      txtcadref (30, 20, 51, 30);
      do {
	okgo = 1;
	l = 0;
/******/
	strcpy (&tile_set_name, rscdir);
	strcpy (&dallepie, rscdir);
	strcpy (&levelnom, nivdir);
	xdalles = 0;
	ydalles = 0;
	xdallesdec = 0;
	xplan = 0;
	yplan = 0;
	tempd = 0xffffffff;
	xplandec = 0;
	yplandec = 0;
	sprhide = 0;
	afftests = 0;
	notestmouse = 0;
/******/

	do {
	  qwrite ("    OPEN LEVEL    ", 32, 23, 14 + 112 - 64 * (l == 0));
	  qwrite (" CREATE NEW LEVEL ", 32, 25, 14 + 112 - 64 * (l == 1));
	  qwrite (" MODIFY LVL-PARAM ", 32, 27, 14 + 112 - 64 * (l == 2));
	  i = get_key ();
	  if (i == 0x11b) {
	    modevga (TEXT);
//                     tt=time(NULL);
//                     fprintf(hlog,"\nNORMAL END on %s\n",_ctime((time_t*)&tt,(char*)tmp));
//                     fclose(hlog);
	    printf ("Heroes Little Editor v" __HEDITver__
		    " (c) 1996-97 RealTech & Olympus\n");
	    printf ("Compiled on " __DATE__ " at " __TIME__ "\n");
//                     do{
//                      i=get_key();
//                      printf("->\%x\n",i);
//                     } while (i!=0x11b);
	    return;
	  }
	  if (i == 0x5000)
	    l = ((l == 2) ? 0 : (l + 1));
	  if (i == 0x4800)
	    l = ((l == 0) ? 2 : (l - 1));
	} while (i != 0x1c0d);
	if (l == 0) {
	  select (nivdir "\\*.lvl", &levelnomshort);
	  if (levelnomshort[0] == 0)
	    okgo = 0;
	  strcat (&levelnom, &levelnomshort);
	  *strchr (&levelnomshort, '.') = 0;
//      if (okgo) fprintf(hlog,"Loading existing level %s:\n",&levelnom);
	}
	if (l == 1) {
	  if (readconfig () != NULL)
	    fatalog (config " error.");
	  okgo = asknew ();
	  strcat (strcat (&levelnom, &levelnomshort), ".lvl");
//      if (okgo) fprintf(hlog,"Creating new level %s:\n",&levelnom);
	}
	if (l == 2) {
	  okgo = 0;
	  select (nivdir "\\*.lvl", &levelnomshort);
	  if (levelnomshort[0] != 0) {
	    strcat (&levelnom, &levelnomshort);
//      fprintf(hlog,"Modifying parameters of %s:\n",&levelnom);
	    *strchr (&levelnomshort, '.') = 0;
	    if (!((ftmp = fopen (levelnom, "rb")) == NULL)) {
	      if (fread (&hplaninfo, sizeof (level_header_t), 1, ftmp) != 1)
		fatalog ("Invalid level file.");
	      fclose (ftmp);
	      if (askold ()) {
//        fprintf(hlog,"þ\t%s SAVED\n",&levelnom);
		ftmp = fopen (levelnom, "r+b");
		strlwr (hplaninfo.soundtrack_name);
		strlwr (hplaninfo.tile_set_name);
		fwrite (&hplaninfo, sizeof (level_header_t), 1, ftmp);
		fclose (ftmp);
	      }			// else
//        fprintf(hlog,"\tcanceled\n",&levelnom);

	    }
	  }
	}
      } while (okgo == 0);
    } else {
      strcpy (&tile_set_name, rscdir);
      strcpy (&dallepie, rscdir);
      strcpy (&levelnom, nivdir);
      xdalles = 0;
      ydalles = 0;
      xdallesdec = 0;
      xplan = 0;
      yplan = 0;
      tempd = 0xffffffff;
      xplandec = 0;
      yplandec = 0;
      sprhide = 0;
      afftests = 0;
      notestmouse = 0;


      strcpy (&levelnomshort, argv1);
      strlwr (&levelnomshort);
      strcat (strcat (&levelnom, &levelnomshort), ".lvl");
      strcat (strcpy (&hplaninfo.tile_set_name, "level"), argv2);
      strcat (strcpy (&hplaninfo.soundtrack_name, "heroes"), argv2);
      hplaninfo.xt = argv3[0] - ' ';
      hplaninfo.yt = argv4[0] - ' ';
      hplaninfo.xwrap = argv5[0] - ' ';
      hplaninfo.ywrap = argv6[0] - ' ';
      if (hplaninfo.xwrap == 1)
	hplaninfo.xwrap = -1;
      if (hplaninfo.ywrap == 1)
	hplaninfo.ywrap = -1;
    }


    modevga (G320x200x256);
    strcat (strcpy (&pcxnom, &levelnomshort), ".pcx");
/************ init du level_map ***********/
    if (!((ftmp = fopen (levelnom, "rb")) == NULL)) {
      if (fread (&hplaninfo, sizeof (level_header_t), 1, ftmp) != 1)
	fatalog ("Invalid level file.");
      if ((level_map = malloc (hplaninfo.xt * hplaninfo.yt * sizeof (tile_t)))
	  == NULL) fatalog ("Not enough memory to allocate for level info");
      if (fread
	  (level_map, sizeof (tile_t), hplaninfo.xt * hplaninfo.yt,
	   ftmp) != (hplaninfo.xt * hplaninfo.yt))
	fatalog ("Invalid level file.");
      fclose (ftmp);
    } else {
      if ((level_map = malloc (hplaninfo.xt * hplaninfo.yt * sizeof (tile_t)))
	  == NULL) fatalog ("Not enough memory to allocate for level info");
      _fmemset (level_map, 0, hplaninfo.xt * hplaninfo.yt * sizeof (tile_t));
      hplaninfo.start[0] = 0;
      hplaninfo.start[1] = 0;
      hplaninfo.start[2] = 0;
      hplaninfo.start[3] = 0;
      hplaninfo.start_way[0] = 0x00;
      hplaninfo.start_way[1] = 0x11;
      hplaninfo.start_way[2] = 0x32;
      hplaninfo.start_way[3] = 0x23;
    }

//fprintf(hlog,"\tUsing %s (PCX,PIE) and %s (XM)\n",hplaninfo.tile_set_name,hplaninfo.soundtrack_name);

/*************************************/

    strcat (strcat (&tile_set_name, &hplaninfo.tile_set_name), ".pcx");
    strcat (strcat (&dallepie, &hplaninfo.tile_set_name), ".pie");

    pcx_load (heddir "edit.pcx", &heditrsc);
    pcx_load (&tile_set_name, &tile_set_img);
/*********** init des ddef ***********/
    if ((ddef = malloc ((tile_set_img.xt / 24) * 10 * sizeof (tile_t))) ==
	NULL) fatalog ("Not enough memory to allocate for tiles-info");
    _fmemset (ddef, 0, (tile_set_img.xt / 24) * 10 * sizeof (tile_info_t));
/*   for (i=0;i<((tile_set_img.xt/24)*10);i++) (*ddef)[i]=.type=0;(*ddef)[i].type=0 */
    if (!((ftmp = fopen (dallepie, "rb")) == NULL))
      fread (ddef, sizeof (tile_info_t), (tile_set_img.xt / 24) * 10, ftmp);
    fclose (ftmp);
/*************************************/
    if (outwayinit ())
      return;
    set_pal ((char *) &tile_set_img.palette, 0, 256 * 3);
    partiel2 (0, 0, 30, 200, 290, 0, &heditrsc);
    strupr (&levelnomshort);
    draw_text (&levelnomshort, 305, 29, 8, 1);
    ultoa (hplaninfo.xt, &nombre, 10);
    draw_text (&nombre, 302, 43, 8, 2);
    ultoa (hplaninfo.yt, &nombre, 10);
    draw_text (&nombre, 307, 43, 8, 0);
    majd ();
    majg ();
    while (key_ready ())
      get_key ();
    mouseon ();
    do {
      while (key_ready () == 0 && mouse123 () == 0);
      mouseoff ();
      if (key_ready ()) {
	i = get_key ();
	gestclav (i);
      } else {
	switch (mouse123 ()) {
	case 1:
	  gestsrs1 ();
	  break;
	case 2:
	  gestsrs2 ();
	  break;
	case 4:
	  gestsrs3 ();
	  break;
	}
	while (mouse123 () != 0);
	notestmouse = 0;
      }
      mouseon ();
    } while (i != 0x11b);
    mouseoff ();
    outwayclose ();
    if (argc != 7) {
      modevga (TEXT);
      lines50 ();
      cursor (0, 50);
      if ((ftmp = fopen (heddir "heditfrm.bin", "rb")) == NULL)
	fatalog ("Can't open HEDITFRM.BIN");
      fread (screentxt, 8000, 1, ftmp);
      fclose (ftmp);

      txtcadref (28, 20, 52, 30);
      l = 0;
      do {
	qwrite (" SAVE LEVEL & TILES  ", 30, 23, 14 + 112 - 64 * (l == 0));
	qwrite (" SAVE LEVEL ONLY     ", 30, 24, 14 + 112 - 64 * (l == 1));
	qwrite (" SAVE TILES ONLY     ", 30, 25, 14 + 112 - 64 * (l == 2));
	qwrite (" DON'T SAVE ANYTHING ", 30, 27, 14 + 112 - 64 * (l == 3));
	i = get_key ();
	if (i == 0x5000)
	  l = (l + 1) & 3;
	if (i == 0x4800)
	  l = (l - 1) & 3;
	if ((i == 0x11b && l != 3)) {
	  l = 3;
	  i = 0;
	}
      } while (i != 0x1c0d && !(i == 0x11b && l == 3));
      if (i == 0x11b)
	l = 3;
//   printf("\%d\n",l);
      if ((l & 3) != 3) {
	txtcadref (27, 20, 53, 30);
	strlwr (hplaninfo.soundtrack_name);
	strlwr (hplaninfo.tile_set_name);
	if ((l & 2) == 0) {
	  if (!((ftmp = fopen (levelnom, "wb")) == NULL)) {
	    if ((fwrite (&hplaninfo, sizeof (level_header_t), 1, ftmp) == 1)
		&&
		(fwrite
		 (level_map, sizeof (tile_t), hplaninfo.xt * hplaninfo.yt,
		  ftmp) == (hplaninfo.xt * hplaninfo.yt))
	      ) {
	      qwrite ("LEVEL SAVED", 29, 24, 10 + 112);
//            fprintf(hlog,"þ\t%s SAVED\n",&levelnom);

	    } else {
	      qwrite ("ERROR! LEVEL NOT SAVED", 29, 24, 12 + 112);
//            fprintf(hlog,"\t%s not saved (ERROR!)\n",&levelnom);
	    }
	    fclose (ftmp);
	  } else {
	    qwrite ("UNABLE TO CREATE LEVEL", 29, 24, 12 + 112);
//            fprintf(hlog,"\t%s not created (ERROR!)\n",&levelnom);
	  }
	} else {
	  qwrite ("LEVEL NOT SAVED", 29, 24, 14 + 112);
//      fprintf(hlog,"\t%s not saved\n",&levelnom);
	}

	if ((l & 1) == 0) {
	  if (!((ftmp = fopen (dallepie, "wb")) == NULL)) {
	    if (fwrite
		(ddef, sizeof (tile_info_t), (tile_set_img.xt / 24) * 10,
		 ftmp) == ((tile_set_img.xt / 24) * 10)) {
	      qwrite ("TILES SAVED", 29, 26, 10 + 112);
//        fprintf(hlog,"þ\t%s SAVED\n",dallepie);
	    } else {
	      qwrite ("ERROR! TILES NOT SAVED", 29, 26, 12 + 112);
//        fprintf(hlog,"\t%s not saved (ERROR!)\n",dallepie);
	    }
	    fclose (ftmp);
	  } else {
	    qwrite ("UNABLE TO CREATE TILES", 29, 26, 12 + 112);
//        fprintf(hlog,"\t%s not created (ERROR!)\n",dallepie);
	  }
	} else {
	  qwrite ("TILES NOT SAVED", 29, 26, 14 + 112);
//      fprintf(hlog,"\t%s not saved\n",dallepie);
	}
	get_key ();
      } else
//      fprintf(hlog,"\tcanceled\n");

	free (ddef);
      free (level_map);
      img_free (&heditrsc);
      img_free (&tile_set_img);

      if ((ftmp = fopen (heddir "HEDITFRM.BIN", "rb")) == NULL)
	fatalog ("Can't open HEDITFRM.BIN");
      fread (screentxt, 8000, 1, ftmp);
      fclose (ftmp);
    } else {
      if (!((ftmp = fopen (levelnom, "wb")) == NULL)) {
	fwrite (&hplaninfo, sizeof (level_header_t), 1, ftmp);
	fwrite (level_map, sizeof (tile_t), hplaninfo.xt * hplaninfo.yt,
		ftmp);
	fclose (ftmp);
      }
      free (ddef);
      free (level_map);
      img_free (&heditrsc);
      img_free (&tile_set_img);
      return;
    }
  }


/*--------------------------------------------------------------------------*/
/*
   printf("hplaninfo.xt=\%d\n",hplaninfo.xt);
   printf("hplaninfo.xwrap=\%d\n",hplaninfo.xwrap);
   printf("hplaninfo.yt=\%d\n",hplaninfo.yt);
   printf("hplaninfo.ywrap=\%d\n",hplaninfo.ywrap);
   printf("hplaninfo.tile_set_name=\%s\n",hplaninfo.tile_set_name);
   printf("hplaninfo.soundtrack_name=\%s\n",hplaninfo.soundtrack_name);
   printf("tile_set_name=\%s\n",&tile_set_name);
   printf("dallepie=\%s -> ",&dallepie);

//   printf("\%d\n",sizeof(tile_t));
   printf("levelnom=\%s -> ",&levelnom);
*/

}
