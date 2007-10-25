//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef ASTRO_NET_H
#define ASTRO_NET_H

#include <irmo.h>

void net_connect(char *host);
void net_run(void);
void net_block(void);

extern IrmoConnection *connection;
extern IrmoWorld *world;

extern IrmoWorld *client_world;
extern IrmoObject *client_player_obj;
extern unsigned int client_keystate;

extern IrmoObject *player;
extern IrmoObject *player_avatar;

extern int gfx_rotate, gfx_1stperson;

extern int net_limit;

#endif /* #ifndef ASTRO_NET_H */

