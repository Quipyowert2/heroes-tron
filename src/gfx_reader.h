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


#include <stdio.h>
#include <string.h>
#include "keys_heroes.h"
#include "pcx.h"
#include "font_help.h"
#include "errors.h"
#include "display.h"
#include "config.h"

static int nbr_lines;
static unsigned char *txtptr;
//static char  *txtpos;
static unsigned char **strptr;
static image_ font_help_img, help_pics_img;

#ifndef __HEROES__

#define xbuf 320
static char *bufhelp2;
//#define bufhelp (bufhelp2-10*320)
#define bufhelp bufhelp2

#else

#ifdef SDF
// Scroll De Fin
#undef xbuf
#define xbuf 320
#define bufhelp (page+10*320)
#else
#define bufhelp corner[0]
#endif

#endif

static void
copy_rect_transp_help (char *src, int dest, int xt)
{
  int j, k;
  char *dest2 = dest + bufhelp + 5 * xbuf;
  for (j = 10; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      if (*src != 0)
	*dest2 = *src;
      src++;
      dest2++;
    }
    src += 320 - xt;
    dest2 += xbuf - xt;
  }
}

#ifdef __HEROES__
static void
copy_rect_transp_help_with_glenz (unsigned char *src, int dest, int xt)
{
  int j, k;
  unsigned char c;
  unsigned char *dest2 = dest + bufhelp + 5 * xbuf;
  for (j = 10; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      c = *src;
      if (c != 0) {
	if (c == 1)
	  *dest2 = glenz[0][*dest2];
	else if (c == 111)
	  *dest2 = glenz[2][*dest2];
	else if (c == 127)
	  *dest2 = glenz[3][*dest2];
	else if (c == 143)
	  *dest2 = glenz[4][*dest2];
	else if (c == 159)
	  *dest2 = glenz[5][*dest2];
	else if (c == 15)
	  *dest2 = glenz[7][*dest2];
	else if (c == 16)
	  *dest2 = glenz[6][*dest2];
	else
	  *dest2 = *src;
      }
      src++;
      dest2++;
    }
    src += 320 - xt;
    dest2 += xbuf - xt;
  }
}
#endif

static void
copy_rect_transp_help_full_glenz (unsigned char *src, int dest, int xt, int c)
{
  int j, k;
  unsigned char *dest2 = dest + bufhelp + 5 * xbuf;
#ifdef __HEROES__
  unsigned char *glenzline;
  glenzline = glenz[c];
#endif

  for (j = 10; j != 0; j--) {
    for (k = xt; k != 0; k--) {
#ifdef __HEROES__
      if (*src != 0)
	*dest2 = glenzline[(int) *dest2];
#else
      if (*src != 0)
	*dest2 = 2;
#endif
      src++;
      dest2++;
    }
    src += 320 - xt;
    dest2 += xbuf - xt;
  }
}

static void
draw_text_help (unsigned char *texte, int posx, int posy, char cent,
		int largeur)
{
  static const int colorhelp[6] = { 255, 111, 127, 143, 159, 16 };
  int i, j, c, color;
  signed int k, l, d = -1, nbrspc = 0, spclrg = 0;
  unsigned char *dest = /*corner[0] */ bufhelp + posx + posy * xbuf;
  unsigned char *src = texte;
  for (; *src != 0; src++)
    if (*src < 128) {
//   if (*src>='a' && *src<='z') *src-=32;
      i = (*src - ' ');
      if (i == 0 && largeur != 0)
	nbrspc++;
      else
	d += help_font_width[i];
    }
  if (largeur != 0) {
    spclrg = largeur - d;
    d = largeur;
  } else
    nbrspc = 0;
  if (cent == 0)
    d = 0;
  if (cent == 1)
    d = -(d >> 1);
  if (cent == 2)
    d = -d;

  dest += d;

  color = colorhelp[0];
  for (; *texte != 0; texte++) {
    i = (*texte - ' ');
    if (i >= 140 - 32 && i < 146 - 32)
      color = colorhelp[i - 140 + 32];
    else if (nbrspc != 0 && i == 0) {
      k = spclrg / nbrspc;
      src += k;
      dest += k;
      spclrg -= k;
      nbrspc--;
    } else {
      src = font_help_img.buffer + ((i & 31) * 8) + (i >> 5) * 320 * 9;
      for (j = help_font_width[i]; j != 0; j--) {
	for (k = 320 * 8, l = xbuf * 8; k >= 0;) {
	  c = *(src + k);
	  k -= 320;
	  if (c != 0) {
	    if (c != 255)
	      *(dest + l) = 82;
	    else
	      *(dest + l) = color;
	  }
	  l -= xbuf;
	}
	dest++;
	src++;
      }
    }
  }
}

static void
show_help (void)
{
  unsigned char *src;
  int justify2, posx, minx, maxx;
  char justify;
  int i;
  char imgalign;
  signed char glenz = -1;
  unsigned char *imgsrc;
  int imgxsize;
  int ligne, nextligne = 2, curligne = 20, ldec;
  int t;

#ifdef __HEROES__
#ifndef SDF
  memset (pal.global, 63, 768);
#else
  frame_cur = frame_old;
#endif
  p = 64;
#endif

  do {
#ifndef __HEROES__
    memset (bufhelp, 0, 320 * 200);
#else
#ifndef SDF
    background_menu ();
#define scroll_speed 8
#else
#define scroll_speed 64
    fr = frame_cur - frame_old;
    frame_cur = frame_old;
    render_background (fr);
    while (fr) {
      if ((nextligne * 10 == curligne) && (nextligne + 20 < nbr_lines))
	++nextligne;
      --fr;
    }
#endif
#endif
    if (curligne < nextligne * 10)
      curligne += 1 + (nextligne * 10 - curligne) / scroll_speed;
    else if (curligne > nextligne * 10)
      curligne += (nextligne * 10 - curligne) / scroll_speed - 1;
    ligne = curligne / 10;
    ldec = (curligne % 10) - 5;
    for (i = -1; i != 20; i++) {
      src = strptr[ligne + i];
      posx = 0;
      justify = 0;
      minx = 0;
      maxx = 310;
      justify2 = 0;
      while ((*src >= 130 && *src < 140) || (*src >= 150)) {
	if (*src == 137 || *src == 138 || *src == 139
	    || *src == 157 || *src == 158 || *src == 159) {
	  imgalign = (*src) - 137;
	  src++;
	  imgsrc = *((char **) src);
	  src += 4;
	  imgxsize = *((short int *) src);
	  src += 2;
	  src += *src;
	  if (glenz == -1) {
	    if (imgalign == 0) {
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 5 + minx,
				     imgxsize);
	      minx += imgxsize;
	    } else if (imgalign == 2) {
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 6 + maxx -
				     imgxsize, imgxsize);
	      maxx -= imgxsize;
	    } else if (imgalign == 1) {
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 5 +
				     ((minx + maxx - imgxsize) >> 1),
				     imgxsize);
	    } else if (imgalign == 20) {
#ifndef __HEROES__
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 5 + minx,
				     imgxsize);
	      minx += imgxsize;
	    } else if (imgalign == 22) {
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 6 + maxx -
				     imgxsize, imgxsize);
	      maxx -= imgxsize;
	    } else if (imgalign == 21) {
	      copy_rect_transp_help (imgsrc,
				     (i * 10 - ldec) * xbuf + 5 +
				     ((minx + maxx - imgxsize) >> 1),
				     imgxsize);
#else
	      copy_rect_transp_help_with_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						minx, imgxsize);
	      minx += imgxsize;
	    } else if (imgalign == 22) {
	      copy_rect_transp_help_with_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 6 +
						maxx - imgxsize, imgxsize);
	      maxx -= imgxsize;
	    } else if (imgalign == 21) {
	      copy_rect_transp_help_with_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						((minx + maxx - imgxsize) >>
						 1), imgxsize);
#endif
	    }
	  } else {
	    if (imgalign == 0 || imgalign == 20) {
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						minx, imgxsize, glenz);
	      minx += imgxsize;
	    } else if (imgalign == 2 || imgalign == 22) {
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 6 +
						maxx - imgxsize, imgxsize,
						glenz);
	      maxx -= imgxsize;
	    } else if (imgalign == 1 || imgalign == 21)
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						((minx + maxx - imgxsize) >>
						 1), imgxsize, glenz);
	    glenz = -1;
	  }
	}
	if (*src == 134)
	  minx += 5;
	if (*src == 135)
	  minx += 30;
	if (*src == 136)
	  maxx -= 5;
	if (*src == 154)
	  minx = 0;
	if (*src == 155)
	  maxx -= 30;
	if (*src == 156)
	  maxx = 310;
	if (*src > 130 && *src <= 133)
	  justify = (*src) - 131;
	if (*src == 130)
	  justify2 = maxx - minx;
	if (*src == 160)
	  glenz = 7;
	if (*src == 161)
	  glenz = 2;
	if (*src == 162)
	  glenz = 3;
	if (*src == 163)
	  glenz = 4;
	if (*src == 164)
	  glenz = 5;
	if (*src == 165)
	  glenz = 6;
	if (*src == 166)
	  glenz = 0;
	if (*src == 167)
	  glenz = 1;
	src++;
      }
      if (justify == 0)
	posx = minx;
      if (justify == 1)
	posx = (minx + maxx) / 2;
      if (justify == 2)
	posx = maxx;

      draw_text_help (src, 5 + posx, (i * 10 - ldec) + 5, justify, justify2);
    }
#ifndef __HEROES__
    vsynchro ();
    _fmemcpy (screen, bufhelp, 320 * 200);
    if (key_ready ()) {
      t = get_key ();
#else
#ifndef SDF
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
#else
    vsynch ();
    if (p < 64) {
      p += fr;
      if (p > 64)
	p = 64;
      set_pal_fade (p);
    }
    display_page ();
#endif
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
#endif
      switch (t) {
      case HK_Down:
	if (nextligne + 20 < nbr_lines)
	  nextligne++;
	break;
      case HK_Up:
	if (nextligne > 1)
	  nextligne--;
	break;
      case HK_PageDown:
	if (nextligne + 39 < nbr_lines)
	  nextligne += 19;
	else
	  nextligne = nbr_lines - 20;
	break;
      case HK_PageUp:
	if (nextligne > 19)
	  nextligne -= 19;
	else
	  nextligne = 1;
	break;
      case HK_End:
	nextligne = nbr_lines - 20;
	break;
      case HK_Home:
	nextligne = 1;
	break;
      }
//#ifdef __HEROES__
    } else
      t = 0;
//#endif
  } while (t != HK_Escape);
}


static void
graphic_reader ()
{
  int ftaille;
  FILE *f;
  int oldi, i, j;
  int adresse;
  unsigned char tmp1[10], tmp2[10];

  nbr_lines = 0;

  pcx_load (spritedir "fontread.pcx", &font_help_img);
  pcx_load (spritedir "helpics.pcx", &help_pics_img);
#ifndef __HEROES__
  bufhelp2 = malloc (320 * 220);
#endif
#ifndef SDF
  f = fopen (textdir "heroes.hlp", "rb");
#else
  f = fopen (textdir "heroes.sdl", "rb");
#endif
  fseek (f, 0, SEEK_END);
  ftaille = ftell (f);
  fseek (f, 0, SEEK_SET);
  txtptr = malloc (ftaille);
  fread (txtptr, ftaille, 1, f);
  fclose (f);
  for (i = ftaille; i >= 0; i--)
    if (*(txtptr + i) == 10)
      nbr_lines++;
  strptr = malloc ((nbr_lines + 2) * sizeof (char *));
  *strptr = txtptr;
  j = 1;
  for (i = 0; i < ftaille; i++) {
    if (*(txtptr + i) == 10) {
      strptr[j++] = txtptr + i + 1;
      *(txtptr + i) = 0;
    } else if (*(txtptr + i) == 13)
      *(txtptr + i) = 0;
  }
  for (i = 0; i < ftaille; i++)
    if (*(txtptr + i) == 137 || *(txtptr + i) == 138 || *(txtptr + i) == 139
	|| *(txtptr + i) == 157 || *(txtptr + i) == 158
	|| *(txtptr + i) == 159) {
      i += 2;
      oldi = i - 1;
      for (j = 0; *(txtptr + i) != ','; j++, i++)
	tmp1[j] = *(txtptr + i);
      tmp1[j] = 0;
      i++;
      for (j = 0; *(txtptr + i) != ','; j++, i++)
	tmp2[j] = *(txtptr + i);
      tmp2[j] = 0;
      i++;
      adresse =
	(int) help_pics_img.buffer + atol ((char *) &tmp1) +
	atol ((char *) &tmp2) * 320;
      for (j = 0; *(txtptr + i) != ')'; j++, i++)
	tmp1[j] = *(txtptr + i);
      tmp1[j] = 0;
      *((long int *) (txtptr + oldi)) = adresse;
      oldi += 4;
      *((short int *) (txtptr + oldi)) = (short int) atol ((char *) &tmp1);
      oldi += 2;
      *(txtptr + oldi) = i - oldi;
    }
#ifndef __HEROES__
  set_pal (&font_help_img.palette.global, 0, 768);
#endif
  show_help ();
  img_free (&font_help_img);
  img_free (&help_pics_img);
  free (txtptr);
#ifndef __HEROES__
  free (bufhelp);
#endif
  free (strptr);
}

#ifndef __HEROES__
main ()
{
  modevga (G320x200x256);
  graphic_reader ();
  modevga (TEXT);
}
#endif
