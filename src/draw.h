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

#ifndef HEROES__DRAW__H
#define HEROES__DRAW__H

void draw_text (const unsigned char *texte, int posx, int posy, char cent);
void draw_text_clipped_left (const unsigned char *texte, int posx, int posy,
			     char cent);
void draw_text_clipped_right (const unsigned char *texte, int posx, int posy,
			      char cent);
void draw_text_waving (const unsigned char *texte, int posx, int posy,
		       char cent);
void draw_text_320 (const unsigned char *texte, int posx, int posy,
		    char cent);
void draw_text_waving_320 (const unsigned char *texte, int posx, int posy, char cent);

extern void (*draw_text_array[2]) (const unsigned char *, int, int, char);
extern void (*draw_text_array_320[2]) (const unsigned char *, int, int, char);

void draw_text_bonus (const unsigned char *texte, int posx, int posy, int p);

void draw_deck_text (const unsigned char *texte, int posx, int posy, 
		     char cent);

void copy_rect_transp (const unsigned char *src, unsigned char *dest, int xt,
		       int yt);
void copy_rect_transp_red (const unsigned char *src, unsigned char *dest,
			   int xt, int yt);
void copy_rect_transp_320 (const unsigned char *src, unsigned char *dest,
			   int xt, int yt);
void copy_32x32_transp_z (const unsigned char *src, unsigned char *dest);
void copy_rect_transp_8 (const unsigned char *src, unsigned char *dest,
			 int xt, int yt, char coul);

void copy_rect_4 (const unsigned char *src, unsigned char *dest, int xt,
		  int yt);
void copy_rect_2 (const unsigned char *src, unsigned char *dest, int xt,
		  int yt);
void copy_rect_2_320 (const unsigned char *src, unsigned char *dest, int xt,
		      int yt);
void copy_rect_4_320 (const unsigned char *src, unsigned char *dest, int xt,
		      int yt);

void draw_demo_stick (const pixel_t* dest);
void aff_buffer (void);
void copy_rect_transp_shadow (const unsigned char *src, unsigned char *dest,
			      int xt, int yt);

void init_text_waving_step (void);
void uninit_text_waving_step (void);
void update_text_waving_step (void);

#endif /* HEROES__DRAW__H */
