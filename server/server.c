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

#include "common/config.h"
#include "common/net.h"
#include "world.h"

static IrmoSocket *sock;
static IrmoServer *server;

// destroy objects on client disconnect

static void destroy_player(IrmoClient *client, AstroPlayer *player)
{
	world_players = g_slist_remove(world_players, player);

	irmo_object_destroy(player->player_obj);

	world_object_destroy(player->avatar);

	player->client->players = g_slist_remove(player->client->players,
						 player);
	
	free(player);
}

static void new_player(IrmoObject *object, AstroClient *client)
{
	IrmoWorld *client_world;
	AstroPlayer *player;
	IrmoObject *playerobj;
	AstroObject *avatar;

	printf("new player created\n");

	playerobj = irmo_object_new(world, "Player");

	if (!playerobj) {
		fprintf(stderr, "No more objects for new player!\n");
		return;
	}

	avatar = world_object_new("PlayerObject", -1, -1, 4096);

	if (!avatar) {
		fprintf(stderr, "No more objects for player avatar!\n");
		return;
	}

	avatar->type = OBJECT_SHIP;
	avatar->size = 1200;
	
	irmo_object_set_int(playerobj, "avatar",
			    irmo_object_get_id(avatar->object));
	irmo_object_set_int(avatar->object, "player", 
			    irmo_object_get_id(playerobj));
	
	player = g_new0(AstroPlayer, 1);

	player->client = client;
	player->client_obj = object;
	player->player_obj = playerobj;
	player->avatar = avatar;

	world_players = g_slist_append(world_players, player);

	// call client and tell them to associate the new player
	// with their own player object

	client_world = irmo_client_get_world(client->client);

	irmo_world_method_call(client_world, "assoc_player",
				  irmo_object_get_id(object),
				  irmo_object_get_id(playerobj));

	irmo_object_watch_destroy(object, (IrmoObjCallback) destroy_player,
				  player);

	client->players = g_slist_append(client->players, player);
}

static void on_disconnect(IrmoClient *client, AstroClient *as_client)
{
	while (as_client->players) {
		destroy_player(client, 
			       (AstroPlayer *) g_slist_nth_data(as_client->players, 0));
	}
	
	free(as_client);
}

static void on_connect(IrmoClient *client, gpointer user_data)
{
	AstroClient *as_client;
	IrmoWorld *client_world 
		= irmo_client_get_world(client);

	as_client = g_new0(AstroClient, 1);
	as_client->client = client;
	
	irmo_client_watch_disconnect(client, 
				     (IrmoClientCallback) on_disconnect,
				     as_client);
	irmo_world_watch_new(client_world, "Player",
			     (IrmoObjCallback) new_player, as_client);
}

void server_init()
{
	IrmoInterfaceSpec *client_spec;

	client_spec = irmo_interface_spec_new(CLIENT_INTERFACE_FILE);

	if (!client_spec) {
		fprintf(stderr, "server_init: Unable to load client spec!\n");
		exit(-1);
	}

	sock = irmo_socket_new(IRMO_SOCKET_IPV6, SERVER_PORT);

	if (sock) {
		printf("server_init: Using IPv6\n");
	} else {
		sock = irmo_socket_new(IRMO_SOCKET_IPV4, SERVER_PORT);

		if (sock) {
			printf("server_init: Using IPv4\n");
		} else {
		
			fprintf(stderr, "server_init: Unable to start server on port\n");
			exit(-1);

		}
	}

	server = irmo_server_new(sock, NULL, world, client_spec);

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

// $Log$
// Revision 1.8  2003/09/02 20:59:36  fraggle
// Use subclassing in irmoroids: select the model to be used by the
// class, not a model number
//
// Revision 1.7  2003/09/02 15:49:30  fraggle
// Make objects smaller in scale (increase arena size)
//
// Revision 1.6  2003/09/02 15:12:00  fraggle
// Create an AstroClient object with a list of AstroPlayers for each object.
// To stop players being destroyed twice
//
// Revision 1.5  2003/09/01 17:04:32  fraggle
// destroy players when the client player object is destroyed as well
// as when the client disconnects
//
// Revision 1.4  2003/09/01 14:35:51  fraggle
// Rename Universe -> World
//
// Revision 1.3  2003/08/26 14:58:17  fraggle
// Stop using AF_* in irmoroids.
//
// Revision 1.2  2003/08/06 16:16:03  fraggle
// IPv6 support
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
