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

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <irmo.h>
#include <irmo/method.h>

#include "common/config.h"
#include "common/net.h"
#include "world.h"

#define EXPLOSION_TIME 10

static gboolean collision_occurred;

IrmoWorld *world;
GSList *world_objects = NULL;
GSList *world_players = NULL;

static gint find_player_foreach(AstroPlayer *p1, IrmoObject *needle)
{
	if (p1->player_obj == needle)
		return 0;

	return 1;
}

AstroPlayer *find_player(IrmoObject *needle)
{
	GSList *result;
	
	result = g_slist_find_custom(world_players,
				     needle,
				     (GCompareFunc) find_player_foreach);

	if (!result)
		return NULL;

	return (AstroPlayer *) g_slist_nth_data(result, 0);
}

static void fire_callback(IrmoMethodData *data, gpointer user_data)
{
	IrmoObjectID id;
	IrmoObject *fireplayer;
	AstroPlayer *player;
	AstroObject *misl;
	float angle;
	
	id = irmo_method_arg_int(data, "player");
	fireplayer = irmo_world_get_object_for_id(world, id);

	if (!fireplayer) {
		printf("fire_callback: player object not found!\n");
		return;
	}

	player = find_player(fireplayer);

	if (player->client->client != irmo_method_get_source(data)) {
		printf("fire_callback: fire from wrong client\n");
		return;
	}

	angle = (player->avatar->angle * M_PI * 2) / 0xffff;
	
	misl = world_object_new("Missile", 
				player->avatar->x, 
				player->avatar->y,
				player->avatar->angle);

	misl->type = OBJECT_MISSILE;
	misl->dx = player->avatar->dx + 512 * cos(angle);
	misl->dy = player->avatar->dy + 512 * sin(angle);
	misl->size = 512;
	misl->missile_life = 40;
}

void world_init()
{
	IrmoInterfaceSpec *spec;

	spec = irmo_interface_spec_new(SERVER_INTERFACE_FILE);

	if (!spec) {
		fprintf(stderr, "world_init: Cant load interface spec\n");
		return;
	}

	world = irmo_world_new(spec);

	irmo_interface_spec_unref(spec);

	irmo_world_method_watch(world, "fire", fire_callback, NULL);
}

AstroObject *world_object_new(char *classname, int x, int y, int angle)
{
	IrmoObject *irmoobj;
	AstroObject *obj;

	if (x < 0 || y < 0) {
		// random position
		
		x = rand() & 0xffff;
		y = rand() & 0xffff;
	}

	irmoobj = irmo_object_new(world, classname);

	if (!irmoobj)
		return NULL;

	obj = g_new0(AstroObject, 1);
	obj->object = irmoobj;
	obj->x = x;
	obj->y = y;
	obj->angle = angle;
	obj->scale = 1.0;
	obj->missile_life = 0;

	irmo_object_set_int(irmoobj, "x", obj->x);
	irmo_object_set_int(irmoobj, "y", obj->y);
	irmo_object_set_int(irmoobj, "angle", obj->angle);
	irmo_object_set_int(irmoobj, "scale", obj->scale * 256);
	
	world_objects = g_slist_append(world_objects, obj);

	return obj;
}

void world_object_destroy(AstroObject *obj)
{
	world_objects = g_slist_remove(world_objects, obj);

	irmo_object_destroy(obj->object);

	free(obj);
}

static void rock_collision(AstroObject *rock1, AstroObject *rock2)
{
	float momx1, momx2, momy1, momy2;

	momx1 = rock1->dx * rock1->size;
	momy1 = rock1->dy * rock1->size;
	momx2 = rock2->dx * rock2->size;
	momy2 = rock2->dy * rock2->size;

	rock1->dx = momx2 / rock1->size;
	rock1->dy = momy2 / rock1->size;
	rock2->dx = momx1 / rock2->size;
	rock2->dy = momy1 / rock2->size;
}

static float frand()
{
	int r = rand() % 65536;

	return r / 65536.0;
}

static float bellcurve(int iterations)
{
	int i;
	float f = 0;

	for (i=0; i<iterations; ++i)
		f += frand();

	return f / iterations;
}

static void missile_hit_rock(AstroObject *missile, AstroObject *target)
{
	int i;
	
	missile->destroyed = target->destroyed = TRUE;

	world_new_explosion(target);

	if (target->scale <= 0.5)
		return;
	
	for (i=0; i<4; ++i) {
		AstroObject *rock;
		int dx, dy;
		int x, y;
		float angle;
		float scale;
		int speed;
		
		// always start them off at the same place

		dx = 512 * cos(i * M_PI_2);
		dy = 512 * sin(i * M_PI_2);
		
		x = (int) (target->x + 4 * target->scale * dx) & 0xffff;
		y = (int) (target->y + 4 * target->scale * dy) & 0xffff;

		scale = target->scale * bellcurve(7);

		if (scale < 0.3)
			scale = 0.3;
		
		rock = world_new_rock(x, y, scale);

		// fly off at a random angle
		
		angle = M_PI_2 * (i + bellcurve(3) - 0.5);

		speed = bellcurve(5) * 512;

		dx = speed * cos(angle);
		dy = speed * sin(angle);
		
		rock->dx = target->dx + dx;
		rock->dy = target->dy + dy;
	}
}

static gboolean do_collision(AstroObject *obj1, AstroObject *obj2)
{
	if (obj1->type == OBJECT_ROCK && obj2->type == OBJECT_ROCK) {
		rock_collision(obj1, obj2);
	} else if (obj1->type == OBJECT_ROCK
		   && obj2->type == OBJECT_MISSILE) {
		missile_hit_rock(obj2, obj1);
	} else if (obj1->type == OBJECT_ROCK
		   && obj2->type == OBJECT_SHIP) {
		rock_collision(obj1, obj2);
	} else {
		return FALSE;
	}

	collision_occurred = TRUE;

	return TRUE;
}

static void run_collisions(AstroObject *obj1, AstroObject *obj2)
{
	double dx, dy;
	double d;

	// only 1 collision allowed
	
	if (collision_occurred)
		return;
	
	if (obj1->destroyed || obj2->destroyed)
		return;
	
	if (obj1 == obj2)
		return;

	dx = obj1->x - obj2->x;
	dy = obj1->y - obj2->y;

	d = sqrt(dx * dx + dy * dy);

	if (d < obj1->size + obj2->size) {
		if (!do_collision(obj1, obj2))
			do_collision(obj2, obj1);
	}
}

static void world_run_objects(AstroObject *obj, gpointer user_data)
{
	if (obj->missile_life) {
		--obj->missile_life;

		if (obj->missile_life <= 0) {
			obj->destroyed = TRUE;

			// if this is a missile, make it explode

			if (obj->type == OBJECT_MISSILE) {
				AstroObject *explosion = 
					world_new_explosion(obj);
			}
		}
		if (obj->type == OBJECT_EXPLOSION) {
			irmo_object_set_int(obj->object, "time", 
					    EXPLOSION_TIME-obj->missile_life);
		}
	}

	if (obj->dx || obj->dy) {
		int oldx=obj->x, oldy=obj->y;
	
		obj->x += obj->dx;
		obj->y += obj->dy;
		obj->x &= 0xffff;
		obj->y &= 0xffff;

		// check for collisions
		// if a collision occurs, we reset back to the old
		// position, as moving here would mean the objects
		// which collided would be inside each other.
		
		collision_occurred = FALSE;

		g_slist_foreach(world_objects,
				(GFunc) run_collisions, obj);

		if (collision_occurred) {
			obj->x = oldx;
			obj->y = oldy;
		} else {
			irmo_object_set_int(obj->object, "x", obj->x);
			irmo_object_set_int(obj->object, "y", obj->y);
		}
	}
}

#define SPEED 512

static void world_run_players(AstroPlayer *player, gpointer user_data)
{
	guint keystate;
	int turn = 0;
	
	keystate = irmo_object_get_int(player->client_obj, "keys");

	if (keystate & KEY_LEFT)
		++turn;
	if (keystate & KEY_RIGHT)
		--turn;

	if (turn) {
		player->avatar->angle += turn * 2048;
		player->avatar->angle &= 0xffff;
		irmo_object_set_int(player->avatar->object, "angle",
				    player->avatar->angle);
	}

	if (keystate & KEY_ACCEL) {
		float angle = (player->avatar->angle * 2 * M_PI) / 0xffff;
		int dx = SPEED * cos(angle);
		int dy = SPEED * sin(angle);

		player->avatar->dx = player->avatar->dx * 0.9 + dx * 0.1;
		player->avatar->dy = player->avatar->dy * 0.9 + dy * 0.1;
		
	}		
}

// garbage collect destroyed objects

static gint world_run_gc_find(AstroObject *a, AstroObject *b)
{
	if (a->destroyed)
		return 0;

	return 1;
}

static void world_run_gc()
{
	GSList *found;

	while ((found = g_slist_find_custom(world_objects, NULL,
					    (GCompareFunc)
					    world_run_gc_find))) {
		AstroObject *obj
			= (AstroObject *) g_slist_nth_data(found, 0);

		world_object_destroy(obj);
	}
}

void world_run()
{
	g_slist_foreach(world_objects, 
			(GFunc) world_run_objects, NULL);
	g_slist_foreach(world_players,
			(GFunc) world_run_players, NULL);

	world_run_gc();	
}

AstroObject *world_new_explosion(AstroObject *parent)
{
	AstroObject *obj;

	obj = world_object_new("Explosion", parent->x, parent->y, 0);
	obj->type = OBJECT_EXPLOSION;
	obj->dx = parent->dx;
	obj->dy = parent->dy;
	obj->size = 1200 * parent->scale;
	obj->scale = parent->scale * 0.6;
	obj->missile_life = EXPLOSION_TIME;

	irmo_object_set_int(obj->object, "scale", obj->scale * 256);

	return obj;
}

#define ROCK_SPEED 512

int misl = 0;

AstroObject *world_new_rock(int x, int y, float scale)
{
	AstroObject *obj;
	
	obj = world_object_new("Asteroid", x, y, 0);

	obj->type = OBJECT_ROCK;
	obj->dx = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;
	obj->dy = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;
	obj->size = 1200 * scale;
	obj->scale = scale;
	
	irmo_object_set_int(obj->object, "scale", scale * 256);

	return obj;
}

// $Log$
// Revision 1.9  2003/11/17 01:43:21  fraggle
// Rename irmo_objid_t to IrmoObjectID. Fix GL mode which was broken.
//
// Revision 1.8  2003/09/13 16:12:21  fraggle
// Fix int overflows
//
// Revision 1.7  2003/09/03 02:58:17  fraggle
// Make rock explosions a bit more random
//
// Revision 1.6  2003/09/02 20:59:36  fraggle
// Use subclassing in irmoroids: select the model to be used by the
// class, not a model number
//
// Revision 1.5  2003/09/02 16:54:32  fraggle
// Add explosions
//
// Revision 1.4  2003/09/02 15:49:30  fraggle
// Make objects smaller in scale (increase arena size)
//
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
// Revision 1.5  2003/06/09 21:14:09  sdh300
// Add Id tag and copyright notice
//
// Revision 1.4  2003/04/21 18:59:07  sdh300
// The models point in the wrong direction, requiring a 90 degree offset to
// angles. Fix this in the client side, not the server side.
//
// Revision 1.3  2003/03/22 23:47:01  sdh300
// Take mass into account in collisions
// Try and stop things getting stuck inside each other
//
// Revision 1.2  2003/03/22 23:17:53  sdh300
// Collisions between objects
// Rocks explode into smaller rocks
// Scaling of world objects
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
