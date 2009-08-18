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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <math.h>

#include <irmo.h>

#include "common/config.h"
#include "common/net.h"
#include "common/math-constants.h"
#include "interfaces/interfaces.h"
#include "world.h"

#define EXPLOSION_TIME 10

IrmoWorld *world;
AstroObject *world_objects = NULL;
AstroPlayer *world_players = NULL;
int num_world_players = 0;

AstroPlayer *find_player(IrmoObject *needle)
{
        AstroPlayer *rover;
	
        for (rover=world_players; rover != NULL; rover = rover->next) {
                if (rover->player_obj == needle) {
                        return rover;
                }
        }

        return NULL;
}

static void fire_callback(IrmoMethodData *data, void *user_data)
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

	if (player->client != irmo_method_get_source(data)) {
		printf("fire_callback: fire from wrong client\n");
		return;
	}

	angle = (player->avatar->angle * PI * 2) / 0xffff;
	
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
	IrmoInterface *iface;

	iface = irmo_interface_load(interface_astro,
                                    interface_astro_length);

	if (!iface) {
		fprintf(stderr, "world_init: Cant load interface file\n");
		return;
	}

	world = irmo_world_new(iface);

	irmo_interface_unref(iface);

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

	obj = malloc(sizeof(AstroObject));
        memset(obj, 0, sizeof(AstroObject));
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
	
        // Hook into linked list
      
        obj->next = world_objects;
        world_objects = obj;

	return obj;
}

void world_object_destroy(AstroObject *obj)
{
        AstroObject **rover;

        for (rover=&world_objects; *rover != NULL; rover = &((*rover)->next)) {
                if (*rover == obj) {
                        *rover = obj->next;
                        irmo_object_destroy(obj->object);
                        free(obj);
                        return;
                }
        }

        assert(0);
}

static void matrix_mult2(float *matrix, float *x, float *result)
{
	result[0] = x[0] * matrix[0] + x[1] * matrix[1];
	result[1] = x[0] * matrix[2] + x[1] * matrix[3];
}

static void rock_collision(AstroObject *rock1, AstroObject *rock2)
{
	float dx, dy;
	float penetrate_dist;
	float transform[4];
	float x[2], r[2];
	float c, s;
	float d;

	// Distance between rocks:

	dx = rock2->x - rock1->x;
	dy = rock2->y - rock1->y;
	d = sqrt(dx*dx + dy*dy);

	// The distance they have penetrated "into" each other:

	penetrate_dist = rock1->size + rock2->size - d;

	// Push the rocks apart so that they are just touching:

	c = dx / d;
	s = dy / d;

	rock2->x += penetrate_dist * c / 2;
	rock2->y += penetrate_dist * s / 2;
	rock1->x -= penetrate_dist * c / 2;
	rock1->y -= penetrate_dist * s / 2;

	// Force vector between the two balls:

	x[0] = rock1->dx * rock1->size - rock2->dx * rock2->size;
	x[1] = rock1->dy * rock1->size - rock2->dy * rock2->size;

	// Rotate the force vector to get the force in the direction
	// between the two balls. Build a transformation matrix to
	// rotate the direction vector:

	transform[0] = c;  transform[1] = s;
	transform[2] = -s; transform[3] = c;

	matrix_mult2(transform, x, r);

	// r[0] gives the force between the balls.  Apply this force
	// to both balls in opposite directions:

	rock1->dx -= c * r[0] / rock1->size;
	rock1->dy -= s * r[0] / rock1->size;
	rock2->dx += c * r[0] / rock2->size;
	rock2->dy += s * r[0] / rock2->size;
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
	
	missile->destroyed = 1;
        target->destroyed = 1;

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

		dx = 512 * cos(i * PI_2);
		dy = 512 * sin(i * PI_2);
		
		x = (int) (target->x + 4 * target->scale * dx) & 0xffff;
		y = (int) (target->y + 4 * target->scale * dy) & 0xffff;

		scale = target->scale * bellcurve(7);

		if (scale < 0.3)
			scale = 0.3;
		
		rock = world_new_rock(x, y, scale);

		// fly off at a random angle
		
		angle = PI_2 * (i + bellcurve(3) - 0.5);

		speed = bellcurve(5) * 512;

		dx = speed * cos(angle);
		dy = speed * sin(angle);
		
		rock->dx = target->dx + dx;
		rock->dy = target->dy + dy;
	}
}

static int do_collision(AstroObject *obj1, AstroObject *obj2)
{
	if (obj1->type == OBJECT_ROCK && obj2->type == OBJECT_ROCK) {
		rock_collision(obj1, obj2);
	} else if (obj1->type == OBJECT_ROCK && obj2->type == OBJECT_MISSILE) {
		missile_hit_rock(obj2, obj1);
	} else if (obj1->type == OBJECT_ROCK && obj2->type == OBJECT_SHIP) {
		rock_collision(obj1, obj2);
	} else {
		return 0;
	}

	return 1;
}

// Check for collisions with the given object.  Returns true if a 
// collision occurred.

static int run_collisions(AstroObject *obj)
{
        AstroObject *rover;
	double dx, dy;
	double d;

        if (obj->destroyed) {
                return 0;
        }

        for (rover=world_objects; rover != NULL; rover = rover->next) {
                if (rover->destroyed) {
                        continue;
                }

                // Don't collide with self

                if (obj == rover) {
                        continue;
                }

                dx = rover->x - obj->x;
                dy = rover->y - obj->y;

                d = sqrt(dx * dx + dy * dy);

                if (d < rover->size + obj->size) {
                        if (do_collision(rover, obj) 
                         || do_collision(obj, rover)) {
                                return 1;
                        }
                }
        }

        return 0;
}

static void world_run_object(AstroObject *obj)
{
        int collision_occurred;

	if (obj->missile_life) {
		--obj->missile_life;

		if (obj->missile_life <= 0) {
			obj->destroyed = 1;

			// if this is a missile, make it explode

			if (obj->type == OBJECT_MISSILE) {
                                world_new_explosion(obj);
			}
		}
		if (obj->type == OBJECT_EXPLOSION) {
			irmo_object_set_int(obj->object, "time", 
					    EXPLOSION_TIME-obj->missile_life);
		}
	}

	if (obj->dx || obj->dy) {
		int oldx, oldy;

                oldx = obj->x;
                oldy = obj->y;
	
		obj->x += obj->dx;
		obj->y += obj->dy;
		obj->x &= 0xffff;
		obj->y &= 0xffff;

		// check for collisions
		// if a collision occurs, we reset back to the old
		// position, as moving here would mean the objects
		// which collided would be inside each other.
		
		collision_occurred = run_collisions(obj);

		if (collision_occurred) {
			obj->x = oldx;
			obj->y = oldy;
		} else {
			irmo_object_set_int(obj->object, "x", obj->x);
			irmo_object_set_int(obj->object, "y", obj->y);
		}
	}
}

static void world_run_objects(void)
{
        AstroObject *rover;

        for (rover=world_objects; rover != NULL; rover = rover->next) {
                world_run_object(rover);
        }
}

#define SPEED 512

static void world_run_player(AstroPlayer *player)
{
	unsigned int keystate;
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
		float angle = (player->avatar->angle * 2 * PI) / 0xffff;
		int dx = SPEED * cos(angle);
		int dy = SPEED * sin(angle);

		player->avatar->dx = player->avatar->dx * 0.9 + dx * 0.1;
		player->avatar->dy = player->avatar->dy * 0.9 + dy * 0.1;
		
	}		
}

static void world_run_players(void)
{
        AstroPlayer *rover;

        for (rover=world_players; rover != NULL; rover = rover->next) {
                world_run_player(rover);
        }
}

// garbage collect destroyed objects

static void world_run_gc()
{
        AstroObject *rover;
        AstroObject *next;

        rover = world_objects;

        while (rover != NULL) {
                
                next = rover->next;

                if (rover->destroyed) {
                        world_object_destroy(rover);
                }

                rover = next;
        }
}

void world_run()
{
        world_run_objects();
        world_run_players();
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

