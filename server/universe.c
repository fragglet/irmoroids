#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <irmo.h>
#include <irmo/method.h>

#include "common/config.h"
#include "common/models.h"
#include "common/net.h"
#include "universe.h"

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


	misl->dx = player->avatar->dx + 512 * cos(angle);
	misl->dy = player->avatar->dy + 512 * sin(angle);
	
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

	irmo_object_set_int(irmoobj, "x", x);
	irmo_object_set_int(irmoobj, "y", y);
	irmo_object_set_int(irmoobj, "angle", angle);

	obj = g_new0(AstroObject, 1);
	obj->object = irmoobj;
	obj->x = x;
	obj->y = y;
	obj->angle = angle;

	universe_objects = g_slist_append(universe_objects, obj);

	return obj;
}

void universe_object_destroy(AstroObject *obj)
{
	universe_objects = g_slist_remove(universe_objects, obj);

	irmo_object_destroy(obj->object);

	free(obj);
}

static void universe_run_objects(AstroObject *obj, gpointer user_data)
{
	if (obj->dx || obj->dy) {
		obj->x += obj->dx;
		obj->y += obj->dy;
		obj->x &= 0xffff;
		obj->y &= 0xffff;
		
		irmo_object_set_int(obj->object, "x", obj->x);
		irmo_object_set_int(obj->object, "y", obj->y);
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

void universe_run()
{
	g_slist_foreach(universe_objects, 
			(GFunc) universe_run_objects, NULL);
	g_slist_foreach(universe_players,
			(GFunc) universe_run_players, NULL);
}

#define ROCK_SPEED 512

int misl = 0;

AstroObject *universe_new_rock()
{
	AstroObject *obj;
	int x, y;

	obj = universe_object_new(-1, -1, 0);

	obj->dx = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;
	obj->dy = (rand() % (ROCK_SPEED * 2)) - ROCK_SPEED;

	if (!misl) {
		irmo_object_set_int(obj->object, "model", MODEL_MISSILE1);
		misl=1;
	}else 
	
	irmo_object_set_int(obj->object, "model", MODEL_ROCK1);

	return obj;
}

// $Log: not supported by cvs2svn $
