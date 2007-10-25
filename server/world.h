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

#ifndef ASTRO_TYPES_H
#define ASTRO_TYPES_H

typedef struct _AstroObject AstroObject;
typedef struct _AstroPlayer AstroPlayer;

#include <irmo.h>

typedef enum {
	OBJECT_ROCK,
	OBJECT_SHIP,
	OBJECT_MISSILE,
	OBJECT_EXPLOSION,
} AstroObjectType;

struct _AstroObject {
	AstroObjectType type;
	int destroyed;
	IrmoObject *object;
	float scale;
	int size;
	int x, y;
	int angle;
	int dx, dy;
	int missile_life;

        // Next object in linked list

        AstroObject *next;
};

struct _AstroPlayer {
        // Client this player is connected from

        IrmoClient *client;

        // Client side player object

	IrmoObject *client_obj;

 	// A Player object about player

	IrmoObject *player_obj;

        // Their ship

	AstroObject *avatar;

        // Next player in linked list.

        AstroPlayer *next;
};

extern IrmoWorld *world;
extern AstroPlayer *world_players;
extern AstroObject *world_objects;
extern int num_world_players;

void world_init();
AstroObject *world_object_new(char *classname, int x, int y, int angle);
AstroObject *world_new_explosion(AstroObject *parent);
AstroObject *world_new_rock(int x, int y, float scale);

#endif /* #ifndef ASTRO_TYPES_H */

