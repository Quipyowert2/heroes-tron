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


#ifndef __KEYS_HEROES_H__
#define __KEYS_HEROES_H__

#include "config.h"

/* Define keys hard coded into heroes code */

#ifdef HAVE_PKG_GGI
#include <ggi/keyboard.h>

#define HK_BackSpace	GIIUC_BackSpace
#define HK_CtrlL	GIIK_CtrlL
#define HK_CtrlR	GIIK_CtrlR
#define HK_D		GIIUC_D
#define HK_d		GIIUC_d
#define HK_E		GIIUC_E
#define HK_F		GIIUC_F
#define HK_Delete	GIIK_Delete
#define HK_Down		GIIK_Down
#define HK_End		GIIK_End
#define HK_Enter	GIIUC_Return
#define HK_Escape	GIIUC_Escape
#define HK_F12		GIIK_F12
#define HK_Home		GIIK_Home
#define HK_Left		GIIK_Left
#define HK_NIL		GIIK_NIL
#define HK_PageDown	GIIK_PageDown
#define HK_PageUp	GIIK_PageUp
#define HK_Pause	GIIK_Pause
#define HK_PrintScreen	GIIK_PrintScreen
#define HK_Right	GIIK_Right
#define HK_S		GIIUC_S
#define HK_s		GIIUC_s
#define HK_ShiftL	GIIK_ShiftL
#define HK_ShiftR	GIIK_ShiftR
#define HK_SysRq	GIIK_SysRq
#define HK_Up		GIIK_Up

#endif /* HAVE_PKG_GGI */

#ifdef HAVE_SDL

#include <SDL_keysym.h>

#define HK_BackSpace	SDLK_BACKSPACE
#define HK_CtrlL	SDLK_LCTRL
#define HK_CtrlR	SDLK_RCTRL
#define HK_D		'D'
#define HK_d		'd'
#define HK_E		'E'
#define HK_F		'F'
#define HK_Delete	SDLK_DELETE
#define HK_Down		SDLK_DOWN
#define HK_End		SDLK_END
#define HK_Enter	SDLK_RETURN
#define HK_Escape	SDLK_ESCAPE
#define HK_F12		SDLK_F12
#define HK_Home		SDLK_HOME
#define HK_Left		SDLK_LEFT
#define HK_NIL		SDLK_UNKNOWN
#define HK_PageDown	SDLK_PAGEDOWN
#define HK_PageUp	SDLK_PAGEUP
#define HK_Pause	SDLK_PAUSE
#define HK_PrintScreen	SDLK_PRINT
#define HK_Right	SDLK_RIGHT
#define HK_S		'S'
#define HK_s		's'
#define HK_ShiftL	SDLK_LSHIFT
#define HK_ShiftR	SDLK_RSHIFT
#define HK_SysRq	SDLK_SYSREQ
#define HK_Up		SDLK_UP

#endif /* HAVE_SDL */

#endif /* __KEYS_HEROES_H__ */
