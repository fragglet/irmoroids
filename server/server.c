
#include <stdio.h>
#include <stdlib.h>

#include "common/config.h"
#include "common/models.h"
#include "common/net.h"
#include "universe.h"

static IrmoSocket *sock;
static IrmoServer *server;

// destroy objects on client disconnect

static void destroy_player(IrmoClient *client, AstroPlayer *player)
{
	universe_players = g_slist_remove(universe_players, player);

	irmo_object_destroy(player->player_obj);

	universe_object_destroy(player->avatar);
}

static void new_player(IrmoObject *object, IrmoClient *client)
{
	IrmoUniverse *client_universe;
	AstroPlayer *player;
	IrmoObject *playerobj;
	AstroObject *avatar;

	printf("new player created\n");

	playerobj = irmo_object_new(universe, "Player");

	if (!playerobj) {
		fprintf(stderr, "No more objects for new player!\n");
		return;
	}

	avatar = universe_object_new(-1, -1, 4096);

	if (!avatar) {
		fprintf(stderr, "No more objects for player avatar!\n");
		return;
	}
	
	irmo_object_set_int(avatar->object, "model", MODEL_SHIP1);
	irmo_object_set_int(playerobj, "avatar",
			    irmo_object_get_id(avatar->object));
	
	player = g_new0(AstroPlayer, 1);

	player->client = client;
	player->client_obj = object;
	player->player_obj = playerobj;
	player->avatar = avatar;
	
	universe_players = g_slist_append(universe_players, player);

	// call client and tell them to associate the new player
	// with their own player object

	client_universe = irmo_client_get_universe(client);

	irmo_universe_method_call(client_universe, "assoc_player",
				  irmo_object_get_id(object),
				  irmo_object_get_id(playerobj));

	irmo_client_watch_disconnect(client, 
				(IrmoClientCallback) destroy_player, 
				player);
}

static void on_connect(IrmoClient *client, gpointer user_data)
{
	IrmoUniverse *client_universe 
		= irmo_client_get_universe(client);

	irmo_universe_watch_new(client_universe, "Player",
				(IrmoObjCallback) new_player, client);
}

void server_init()
{
	IrmoInterfaceSpec *client_spec;

	client_spec = irmo_interface_spec_new(CLIENT_INTERFACE_FILE);

	if (!client_spec) {
		fprintf(stderr, "server_init: Unable to load client spec!\n");
		exit(-1);
	}

	sock = irmo_socket_new(AF_INET, SERVER_PORT);

	if (!sock) {
		fprintf(stderr, "server_init: Unable to start server on port\n");
		exit(-1);
	}

	server = irmo_server_new(sock, NULL, universe, client_spec);

	if (!server) {
		fprintf(stderr, "server_init: Unable to create server\n");
		exit(-1);
	}

	irmo_server_watch_connect(server, on_connect, NULL);
}

void server_run()
{
	irmo_socket_run(sock);
}

// $Log: not supported by cvs2svn $
