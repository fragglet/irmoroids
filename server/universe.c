#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <irmo.h>
#include <irmo/method.h>

#include "common/config.h"
#include "common/models.h"
#include "common/net.h"
#include "universe.h"

static gboolean collision_occurred;

IrmoUniverse *universe;
GSList *universe_objects = NULL;
GSList *universe_players = NULL;

static gint find_player_foreach(AstroPlayer *p1, IrmoObject *needle)
{
	if (p1->player_obj == needle)
		return 0;

	return 1;
}

AstroPlayer *find_player(IrmoObject *needle)
{
	GSList *result;
	
	result = g_slist_find_custom(universe_players,
				     needle,
				     (GCompareFunc) find_player_foreach);

	if (!result)
		return NULL;

	return (AstroPlayer *) g_slist_nth_data(result, 0);
}

static void fire_callback(IrmoMethodData *data, gpointer user_data)
{
	irmo_objid_t id;
	IrmoObject *fireplayer;
	AstroPlayer *player;
	AstroObject *misl;
	float angle;
	
	id = irmo_method_arg_int(data, "player");
	fireplayer = irmo_universe_get_object_for_id(universe, id);

	if (!fireplayer) {
		printf("fire_callback: player object not found!\n");
		return;
	}

	player = find_player(fireplayer);

	if (player->client != irmo_method_get_source(data)) {
		printf("fire_callback: fire from wrong client\n");
		return;
	}

	angle = (player->avatar->angle * M_PI * 2) / 0xffff;
	angle += M_PI / 2;
	
	misl = universe_object_new(player->avatar->x, player->avatar->y,
				   player->avatar->angle);

	misl->type = OBJECT_MISSILE;
	misl->dx = player->avatar->dx + 512 * cos(angle);
	misl->dy = player->avatar->dy + 512 * sin(angle);
	misl->size = 1024;
	
	irmo_object_set_int(misl->object, "model", MODEL_MISSILE1);
}

void universe_init()
{
	IrmoInterfaceSpec *spec;

	spec = irmo_interface_spec_new(SERVER_INTERFACE_FILE);

	if (!spec) {
		fprintf(stderr, "universe_init: Cant load interface spec\n");
		return;
	}

	universe = irmo_universe_new(spec);

	irmo_interface_spec_unref(spec);

	irmo_universe_method_watch(universe, "fire", fire_callback, NULL);
}

AstroObject *universe_object_new(int x, int y, int angle)
{
	IrmoObject *irmoobj;
	AstroObject *obj;

	if (x < 0 || y < 0) {
		// random position
		
		x = rand() & 0xffff;
		y = rand() & 0xffff;
	}

	irmoobj = irmo_object_new(universe, "Object");

	if (!irmoobj)
		return NULL;

	obj = g_new0(AstroObject, 1);
	obj->object = irmoobj;
	obj->x = x;
	obj->y = y;
	obj->angle = angle;
	obj->scale = 1.0;

	irmo_object_set_int(irmoobj, "x", obj->x);
	irmo_object_set_int(irmoobj, "y", obj->y);
	irmo_object_set_int(irmoobj, "angle", obj->angle);
	irmo_object_set_int(irmoobj, "scale", obj->scale * 256);
	
	universe_objects = g_slist_append(universe_objects, obj);

	return obj;
}

void universe_object_destroy(AstroObject *obj)
{
	universe_objects = g_slist_remove(universe_objects, obj);

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

static void missile_hit_rock(AstroObject *missile, AstroObject *target)
{
	int i;
	
	missile->destroyed = target->destroyed = TRUE;

	if (target->scale <= 0.5)
		return;
	
	for (i=0; i<4; ++i) {
		AstroObject *rock;
		int dx, dy;
		int x, y;
		
		dx = 512 * cos(i * M_PI / 2);
		dy = 512 * sin(i * M_PI / 2);
		
		x = target->x + 4 * target->scale * dx;
		y = target->y + 4 * target->scale * dy;
		
		rock = universe_new_rock(x, y, 
					 target->scale * 0.5);
		
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

static void universe_run_objects(AstroObject *obj, gpointer user_data)
{
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

		g_slist_foreach(universe_objects,
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

#define SPEED 1024

static void universe_run_players(AstroPlayer *player, gpointer user_data)
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
		irmo_object_set_int(player->avatar->object, "angle",
				    player->avatar->angle);
	}

	if (keystate & KEY_ACCEL) {
		float angle = (player->avatar->angle * 2 * M_PI) / 0xffff
			+ M_PI / 2;
		int dx = SPEED * cos(angle);
		int dy = SPEED * sin(angle);

		player->avatar->dx = player->avatar->dx * 0.9 + dx * 0.1;
		player->avatar->dy = player->avatar->dy * 0.9 + dy * 0.1;
		
	}		
}

// garbage collect destroyed objects

static gint universe_run_gc_find(AstroObject *a, AstroObject *b)
{
	if (a->destroyed)
		return 0;

	return 1;
}

static void universe_run_gc()
{
	GSList *found;

	while ((found = g_slist_find_custom(universe_objects, NULL,
					    (GCompareFunc)
					    universe_run_gc_find))) {
		AstroObject *obj
			= (AstroObject *) g_slist_nth_data(found, 0);

		universe_object_destroy(obj);
	}
}

void universe_run()
{
	g_slist_foreach(universe_objects, 
			(GFunc) universe_run_objects, NULL);
	g_slist_foreach(universe_players,
			(GFunc) universe_run_players, NULL);

	universe_run_gc();	
}

#define ROCK_SPEED 512

int misl = 0;

AstroObject *universe_new_rock(int x, int y, float scale)
{
	AstroObject *obj;
	
	obj = universe_object_new(x, y, 0);

	obj->type = OBJECT_ROCK;
	obj->dx = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;
	obj->dy = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;
	obj->size = 2500 * scale;
	obj->scale = scale;
	
	irmo_object_set_int(obj->object, "scale", scale * 256);
	irmo_object_set_int(obj->object, "model", MODEL_ROCK1);

	return obj;
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/03/22 23:17:53  sdh300
// Collisions between objects
// Rocks explode into smaller rocks
// Scaling of world objects
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
