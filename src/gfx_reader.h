/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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

#include "system.h"
#include "timer.h"
#include "keys_heroes.h"
#include "pcx.h"
#include "font_help.h"
#include "errors.h"
#include "display.h"
#include "rsc_files.h"
#include "debugmsg.h"
#include "endian.h"
#include "fader.h"
#include "scrtools.h"

static int nbr_lines;
static u8_t *txtptr;
static u8_t **strptr;
static pcx_image_t font_help_img, help_pics_img;

#ifdef SDF
/* end scroller */
#undef xbuf
#define xbuf 320
#define bufhelp (page+10*320)
#else
#define bufhelp corner[0]
#endif

static void
copy_rect_transp_help (const pixel_t *src, int dest, int xt)
{
  int j, k;
  pixel_t* dest2 = bufhelp + 5 * xbuf + dest;
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

static void
copy_rect_transp_help_with_glenz (const pixel_t* src, int dest, int xt)
{
  int j, k;
  pixel_t c;
  pixel_t* dest2 = dest + bufhelp + 5 * xbuf;
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

static void
copy_rect_transp_help_full_glenz (const pixel_t* src, int dest, int xt, int c)
{
  int j, k;
  pixel_t* dest2 = dest + bufhelp + 5 * xbuf;
  pixel_t* glenzline;
  glenzline = glenz[c];

  for (j = 10; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      if (*src != 0)
	*dest2 = glenzline[(int) *dest2];
      src++;
      dest2++;
    }
    src += 320 - xt;
    dest2 += xbuf - xt;
  }
}

static void
draw_text_help (u8_t *texte, int posx, int posy, char cent, int largeur)
{
  static const int colorhelp[6] = { 255, 111, 127, 143, 159, 16 };
  int i, j, c, color;
  signed int k, l, d = -1, nbrspc = 0, spclrg = 0;
  pixel_t* dest = bufhelp + posx + posy * xbuf;
  u8_t *src = texte;
  for (; *src != 0; src++)
    if (*src < 128) {
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
  u8_t *src;
  int justify2, posx, minx, maxx;
  char justify;
  int i;
  char imgalign;
  signed char glenz_color = -1;
  pixel_t *imgsrc;
  int imgxsize;
  int ligne, nextligne = 2, curligne = 20, ldec;
  keycode_t t;
#ifdef SDF
  int slices;
  htimer_t reader_htimer;
#endif

#ifndef SDF
  std_white_fadein (&tile_set_img.palette);
#else
  reader_htimer = new_htimer (T_LOCAL, HZ (70));
  std_black_fadein (&background_img.palette);
#endif

  do {
#ifndef SDF
    background_menu ();
#define scroll_speed 8
#else
#define scroll_speed 64
    slices = read_htimer (reader_htimer);
    render_background (slices);
    while (slices) {
      if ((nextligne * 10 == curligne) && (nextligne + 20 < nbr_lines))
	++nextligne;
      --slices;
    }
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
	  imgsrc = (u8_t *)GETWORD(src);
	  src += 4;
	  imgxsize = GETHALFWORD(src);
	  src += 2;
	  src += *src;
	  if (glenz_color == -1) {
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
	    }
	  } else {
	    if (imgalign == 0 || imgalign == 20) {
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						minx, imgxsize, glenz_color);
	      minx += imgxsize;
	    } else if (imgalign == 2 || imgalign == 22) {
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 6 +
						maxx - imgxsize, imgxsize,
						glenz_color);
	      maxx -= imgxsize;
	    } else if (imgalign == 1 || imgalign == 21)
	      copy_rect_transp_help_full_glenz (imgsrc,
						(i * 10 - ldec) * xbuf + 5 +
						((minx + maxx - imgxsize) >>
						 1), imgxsize, glenz_color);
	    glenz_color = -1;
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
	  glenz_color = 7;
	if (*src == 161)
	  glenz_color = 2;
	if (*src == 162)
	  glenz_color = 3;
	if (*src == 163)
	  glenz_color = 4;
	if (*src == 164)
	  glenz_color = 5;
	if (*src == 165)
	  glenz_color = 6;
	if (*src == 166)
	  glenz_color = 0;
	if (*src == 167)
	  glenz_color = 1;
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
    vsynch ();
#ifndef SDF
    aff_buffer ();
#else
    display_page ();
#endif
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
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
      default:
	/* NOP */
      }
     } else
       t = 0;
  } while (t != HK_Escape);
#ifdef SDF
  free_htimer (reader_htimer);
#endif
}


static void
graphic_reader (void)
{
  int ftaille;
  FILE *f;
  int oldi, i, j;
  int adresse;
  unsigned char tmp1[10], tmp2[10];

  dmsg (D_SECTION, "graphic reader");

  nbr_lines = 0;

  pcx_load_from_rsc ("help-font", &font_help_img);
  pcx_load_from_rsc ("help-pictures-img", &help_pics_img);
  {
#ifndef SDF
    char *t = get_non_null_rsc_file ("help-txt");
#else
    char *t = get_non_null_rsc_file ("end-scroller-txt");
#endif
    dmsg (D_FILE, "open file %s", t);
    f = fopen (t, "rb");
    free (t);
  }
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
      SETWORD((unsigned char *)(txtptr + oldi), adresse);
      oldi += 4;
      {
	  unsigned short s = atol ((char *) &tmp1);
	  SETHALFWORD((unsigned char *)(txtptr + oldi), s);
      }
      oldi += 2;
      *(txtptr + oldi) = i - oldi;
    }
  show_help ();
  img_free (&font_help_img);
  img_free (&help_pics_img);
  free (txtptr);
  free (strptr);
}
