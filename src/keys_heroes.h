/*------------------------------------------------------------------------.
| Copyright 2000, 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
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


#ifndef HEROES__KEYS_HEROES__H
#define HEROES__KEYS_HEROES__H

#include "system.h"

/* Define keys hard coded into heroes code */

#ifdef HAVE_LIBGGI

#define	HK_MOD_None	0
#define	HK_MOD_Shift	GII_MOD_SHIFT
#define	HK_MOD_Ctrl	GII_MOD_CTRL
#define	HK_MOD_Alt	GII_MOD_ALT
#define	HK_MOD_Meta	GII_MOD_META

#define HK_0		GIIUC_0
#define HK_1		GIIUC_1
#define HK_2		GIIUC_2
#define HK_3		GIIUC_3
#define HK_4		GIIUC_4
#define HK_5		GIIUC_5
#define HK_6		GIIUC_6
#define HK_7		GIIUC_7
#define HK_8		GIIUC_8
#define HK_9		GIIUC_9
#define HK_10		GIIUC_10
#define HK_BackSpace	GIIUC_BackSpace
#define HK_CtrlL	GIIK_CtrlL
#define HK_CtrlR	GIIK_CtrlR
#define HK_D		GIIUC_D
#define HK_d		GIIUC_d
#define HK_E		GIIUC_E
#define HK_Delete	GIIK_Delete
#define HK_Down		GIIK_Down
#define HK_End		GIIK_End
#define HK_Enter	GIIUC_Return
#define HK_Escape	GIIUC_Escape
#define HK_F		GIIUC_F
#define HK_f		GIIUC_f
#define HK_F1		GIIK_F1
#define HK_F2		GIIK_F2
#define HK_F3		GIIK_F3
#define HK_F4		GIIK_F4
#define HK_F5		GIIK_F5
#define HK_F6		GIIK_F6
#define HK_F7		GIIK_F7
#define HK_F8		GIIK_F8
#define HK_F9		GIIK_F9
#define HK_F10		GIIK_F10
#define HK_F11		GIIK_F11
#define HK_F12		GIIK_F12
#define HK_Home		GIIK_Home
#define HK_i		GIIUC_i
#define HK_I		GIIUC_I
#define HK_Left		GIIK_Left
#define HK_NIL		GIIK_NIL
#define HK_o		GIIUC_o
#define HK_O		GIIUC_O
#define HK_p		GIIUC_p
#define HK_P		GIIUC_P
#define HK_PageDown	GIIK_PageDown
#define HK_PageUp	GIIK_PageUp
#define HK_Pause	GIIK_Pause
#define HK_PrintScreen	GIIK_PrintScreen
#define HK_Right	GIIK_Right
#define HK_S		GIIUC_S
#define HK_s		GIIUC_s
#define HK_ShiftL	GIIK_ShiftL
#define HK_ShiftR	GIIK_ShiftR
#define HK_Space	GIIUC_Space
#define HK_SysRq	GIIK_SysRq
#define HK_t		GIIUC_t
#define HK_T		GIIUC_T
#define HK_Up		GIIK_Up

#endif /* HAVE_LIBGGI */

#ifdef HAVE_LIBSDL

#define	HK_MOD_None	0
#define	HK_MOD_Shift	KMOD_SHIFT
#define	HK_MOD_Ctrl	KMOD_CTRL
#define	HK_MOD_Alt	KMOD_ALT
#define	HK_MOD_Meta	KMOD_META

#define HK_0		SDLK_0
#define HK_1		SDLK_1
#define HK_2		SDLK_2
#define HK_3		SDLK_3
#define HK_4		SDLK_4
#define HK_5		SDLK_5
#define HK_6		SDLK_6
#define HK_7		SDLK_7
#define HK_8		SDLK_8
#define HK_9		SDLK_9
#define HK_10		SDLK_10
#define HK_BackSpace	SDLK_BACKSPACE
#define HK_CtrlL	SDLK_LCTRL
#define HK_CtrlR	SDLK_RCTRL
#define HK_D		'D'
#define HK_d		'd'
#define HK_E		'E'
#define HK_Delete	SDLK_DELETE
#define HK_Down		SDLK_DOWN
#define HK_End		SDLK_END
#define HK_Enter	SDLK_RETURN
#define HK_Escape	SDLK_ESCAPE
#define HK_F		'F'
#define HK_f		'f'
#define HK_F1		SDLK_F1
#define HK_F2		SDLK_F2
#define HK_F3		SDLK_F3
#define HK_F4		SDLK_F4
#define HK_F5		SDLK_F5
#define HK_F6		SDLK_F6
#define HK_F7		SDLK_F7
#define HK_F8		SDLK_F8
#define HK_F9		SDLK_F9
#define HK_F10		SDLK_F10
#define HK_F11		SDLK_F11
#define HK_F12		SDLK_F12
#define HK_Home		SDLK_HOME
#define HK_i		'i'
#define HK_I		'I'
#define HK_Left		SDLK_LEFT
#define HK_NIL		SDLK_UNKNOWN
#define HK_o		'o'
#define HK_O		'O'
#define HK_p		'p'
#define HK_P		'P'
#define HK_PageDown	SDLK_PAGEDOWN
#define HK_PageUp	SDLK_PAGEUP
#define HK_Pause	SDLK_PAUSE
#define HK_PrintScreen	SDLK_PRINT
#define HK_Right	SDLK_RIGHT
#define HK_S		'S'
#define HK_s		's'
#define HK_ShiftL	SDLK_LSHIFT
#define HK_ShiftR	SDLK_RSHIFT
#define HK_Space	SDLK_SPACE
#define HK_SysRq	SDLK_SYSREQ
#define HK_t		't'
#define HK_T		'T'
#define HK_Up		SDLK_UP

#endif /* HAVE_LIBSDL */

#endif /* HEROES__KEYS_HEROES__H */
