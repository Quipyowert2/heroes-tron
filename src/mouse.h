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


#ifndef _MOUSE_H_plx_
#define _MOUSE_H_plx_

static unsigned char decalx = 3;	// pour les modes textes.
static unsigned char decaly = 3;	// en mode 0x13, utiliser 1 pour x et 0 pour y

static char mouseexist = 0;	// 0xFF si driver chargé. 0 sinon.
static char mouseshown = 0;	// 1 si ptr affiché. 0 sinon.
signed short int mouseinit ();	// -1 si driver chargé.
void mouseon ();		// affiche la bête
void mouseoff ();		// planque la bête
unsigned short int mousex ();	// Coordonnée X de la boule de poils
unsigned short int mousey ();	// Coordonnée Y de la boule de poils
char mouse1 ();			// 1 si bouton gauche appuyé. 0 sinon.
char mouse2 ();			// 2 si bouton droit appuyé.  0 sinon.
char mouse12 ();		// 3 si boutons g & d appuyés.0 sinon.
char mouse3 ();			// 4 si bouton central appuyé.0 sinon.
char mouse123 ();		// ...
void setmouse (int, int);	// positionne la bête
void setratio (int, int);	// pixel/mickey ratio

#endif
