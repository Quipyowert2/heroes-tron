/*------------------------------------------------------------------.
| Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License version 2 as    |
| published by the Free Software Foundation.                        |
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

#ifndef HEROES__OPPONENTS__H
#define HEROES__OPPONENTS__H

#include "state.h"

typedef enum an_opponent_type an_opponent_type;
enum an_opponent_type {
  OT_NONE   = 0,		/* cannot play */
  OT_QUEST  = 1,		/* can play quest mode */
  OT_DEATHM = 2,		/* can play death-math */
  OT_KILLEM = 4,		/* can play kill'em all */
  OT_TCASH  = 8,		/* can play time-ca$h */
  OT_COLOR  = 16,		/* can play color */
};

typedef enum a_throttle a_throttle;
enum a_throttle {
  TH_BRAKE = 0,
  TH_NORMAL = 1,
  TH_SPEEDUP = 2,
};


typedef struct an_opponent_action an_opponent_action;
struct an_opponent_action {
  a_dir dir;
  a_throttle throttle;
};

typedef struct an_opponent_sig an_opponent_sig;
struct an_opponent_sig {

  /* A name for the opponent plug-in.  This is not yet used today,
     except for debugging messages, bug it might be used later to
     present to the user a list of plug-ins to chose from.  */
  const char *name;

  /* A mask indicating which mode this opponent can be used for.  */
  an_opponent_type type;

  /* This is called after the state have been initialized, so that the
     opponent can prepare itself to play.  This function is called
     once for each player that will be driven by the current opponent
     module.  The returned void* data, are callback data which will be
     passed back to the opponent in following function calls.  */
  void *(*initialize_player)(const a_level_state *state, int player);

  /* This is called after the level is over, so the opponent can
     do some cleanup.  For instance callback_data should be
     free here if it was allocated by initialize_player.  */
  void (*finalize_player)(const a_level_state *state, int player,
			  void *callback_data);

  /* Unless NULL, this functions is called every time the player is
     introduced into the game (after a death, or at the beginning of
     the game.  The returned void* data will replace any previous
     callback data setup by initialize_player, in following function
     calls.  */
  void *(*start_player)(const a_level_state *state, int player,
			void *callback_data);

  /* Unless NULL, this functions is called every time the player is
     removed from the game (i.e., after a death).  */
  void *(*stop_player)(const a_level_state *state, int player,
		       void *callback_data);


  /* Unless NULL, this function is called each time the player enter a
     new square.  This is usually where the player should decide
     what's the next direction it will take.  */
  void (*square_update)(const a_level_state *state, int player,
			an_opponent_action *action,
			void *callback_data);

  /* Unless NULL, this functions is called on each frame (i.e. 70
     times per second).  This is called frequently, so please, keep
     it fast!.  */
  void (*frame_update)(const a_level_state *state, int player,
		       an_opponent_action *action,
		       void *callback_data);
};



#endif /* HEROES__OPPONENTS__H */
