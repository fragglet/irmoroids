// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
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
//---------------------------------------------------------------------

#ifndef ASTRO_TYPES_H
#define ASTRO_TYPES_H

typedef struct _AstroObject AstroObject;
typedef struct _AstroModel AstroModel;
typedef struct _AstroPlayer AstroPlayer;
typedef struct _AstroClient AstroClient;

#include <glib.h>
#include <irmo.h>

typedef enum {
	OBJECT_ROCK,
	OBJECT_SHIP,
	OBJECT_MISSILE,
} AstroObjectType;

struct _AstroObject {
	AstroObjectType type;
	gboolean destroyed;
	IrmoObject *object;
	float scale;
	int size;
	int x, y;
	int angle;
	int dx, dy;
	int missile_life;
};

struct _AstroPlayer {
	AstroClient *client;
	IrmoObject *client_obj;		// client side player object
	IrmoObject *player_obj; 	// a Player object about player
	AstroObject *avatar;		// their ship
};

struct _AstroClient {
	IrmoClient *client;
	GSList *players;
};

extern IrmoWorld *world;
extern GSList *world_players;
extern GSList *world_objects;

void world_init();
AstroObject *world_object_new(int x, int y, int angle);
AstroObject *world_new_rock(int x, int y, float scale);

#endif /* #ifndef ASTRO_TYPES_H */

// $Log$
// Revision 1.3  2003/09/02 15:32:53  fraggle
// Limited life on missiles
//
// Revision 1.2  2003/09/02 15:12:00  fraggle
// Create an AstroClient object with a list of AstroPlayers for each object.
// To stop players being destroyed twice
//
// Revision 1.1  2003/09/01 14:35:51  fraggle
// Rename Universe -> World
//
// Revision 1.1.1.1  2003/06/09 21:34:37  fraggle
// Initial sourceforge import
//
// Revision 1.3  2003/06/09 21:14:09  sdh300
// Add Id tag and copyright notice
//
// Revision 1.2  2003/03/22 23:17:53  sdh300
// Collisions between objects
// Rocks explode into smaller rocks
// Scaling of world objects
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//

