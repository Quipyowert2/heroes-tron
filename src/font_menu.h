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


#define font_pos          0
#define font_first_ascii  32	/* number du premier caractère (ASCII) */
#define font_transp_color 0	/* couleur transparante */
#define font_height       10	/* hauteur de chaque caractère */

char font_width[69] =		/* largeur de chaque caractère */
						       /* 8 */
{ 5, 8, 15, 20, 20, 20, 20, 8, 11, 11, 20, 16, 9, 16, 8, 15, 20, 20, 20, 20, 20,
/*        !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  */

  20, 20, 20, 20, 20, 8, 8, 15, 16, 15, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 8,
/*     5  6  7  8  9  :  ;  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  */

  20, 20, 20, 22, 20, 20, 20, 20, 20, 20, 20, 20, 20, 22, 20, 20, 20, 11, 15,
  11, 12,
/*     J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  */

  16, 0, 12, 12, 12, 12
};
/*     _  `  a  b  c  d  */
