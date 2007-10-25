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
#include <stdarg.h>

#include "common/config.h"
#include "common/net.h"
#include "interfaces/interfaces.h"
#include "world.h"

static IrmoServer *server;

// send a message to a client

void player_write_message(AstroPlayer *player, char *message, ...)
{
        char buf[128];
	va_list args;

	va_start(args, message);

        vsprintf(buf, message, args);

	va_end(args);

	irmo_world_method_call(irmo_object_get_world(player->client_obj),
			       "display_message", 
			       irmo_object_get_id(player->client_obj),
			       buf);
}

// destroy objects on client disconnect

static void destroy_player(IrmoClient *client, AstroPlayer *player)
{
        AstroPlayer **rover;

        // Unhook from linked list

        for (rover=&world_players; *rover != NULL; rover=&(*rover)->next) {
                if (*rover == player) {
                        *rover = (*rover)->next;
                        break;
                }
        }

        // Destroy the player object and avatar

	irmo_object_destroy(player->player_obj);
	world_object_destroy(player->avatar);

	free(player);

        --num_world_players;
}

static void new_player(IrmoObject *object, IrmoClient *client)
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
	
	player = malloc(sizeof(AstroPlayer));
        player->client = client;
	player->client_obj = object;
	player->player_obj = playerobj;
	player->avatar = avatar;

        // Hook into players list

        player->next = world_players;
        world_players = player;

	// call client and tell them to associate the new player
	// with their own player object

	client_world = irmo_client_get_world(client);

	irmo_world_method_call(client_world, "assoc_player",
				  irmo_object_get_id(object),
				  irmo_object_get_id(playerobj));

	irmo_object_watch_destroy(object, (IrmoObjCallback) destroy_player,
				  player);

        ++num_world_players;

	player_write_message(player, "welcome to this server.");
	player_write_message(player, "there are currently %i players in the game",
                             num_world_players);

        // When the client disconnects, destroy the player

        irmo_client_watch_disconnect(client, (IrmoClientCallback) destroy_player,
                                     player);
}

static void on_connect(IrmoClient *client, void *user_data)
{
	IrmoWorld *client_world 
		= irmo_client_get_world(client);

	irmo_world_watch_new(client_world, "Player",
			     (IrmoObjCallback) new_player, client);
}

void server_init()
{
	IrmoInterface *client_interface;

	client_interface = irmo_interface_load(interface_astroclient,
                                               interface_astroclient_length);

	if (!client_interface) {
		fprintf(stderr, "server_init: Unable to load "
                                "client interface file!\n");
		exit(-1);
	}

	server = irmo_server_new(IRMO_SOCKET_IPV6, SERVER_PORT, world,
                                 client_interface);

	if (server) {
		printf("server_init: Using IPv6\n");
	} else {
		server = irmo_server_new(IRMO_SOCKET_IPV4, SERVER_PORT, world, client_interface);

		if (server) {
			printf("server_init: Using IPv4\n");
		} else {
		
			fprintf(stderr, "server_init: Unable to start server on port\n");
			exit(-1);

		}
	}

	irmo_server_watch_connect(server, on_connect, NULL);
}

void server_run()
{
	irmo_server_run(server);
}

