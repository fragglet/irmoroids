// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
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
//---------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <irmo.h>
#include <GL/gl.h>
#include <math.h>

#include "common/config.h"
#include "common/math-constants.h"
#include "common/net.h"

#include "interfaces/interfaces.h"

#include "models.h"
#include "net.h"

#define NUM_STARS 1024

static GLfloat stars[NUM_STARS][3];

IrmoConnection *connection;
IrmoWorld *world;

IrmoWorld *client_world;
IrmoObject *client_player_obj;
unsigned int client_keystate;

IrmoObject *player = NULL;
IrmoObject *player_avatar;

int gfx_rotate = 0;
int gfx_1stperson = 0;

int net_limit = 1000;

static GLfloat avatar_x, avatar_y;
static GLfloat avatar_angle;
static GLfloat avatar_angle_cos, avatar_angle_sin;

static inline float angle_translate(int angle)
{
	return (angle * 360.0) / 0xffff;
}

static inline float coord_translate(int x)
{
	return x / 4096.0;
}

static void point_normalise(GLfloat *x, GLfloat *y)
{
	GLfloat max = coord_translate(0xffff);

	if (avatar_x - *x > max/2)
		*x += max;
	if (avatar_y - *y > max/2)
		*y += max;
	if (*x - avatar_x > max/2)
		*x -= max;
	if (*y - avatar_y > max/2)
		*y -= max;
}

void make_stars()
{
	int i;

	for (i=0; i<NUM_STARS; ++i) {
		stars[i][0] = coord_translate(rand() & 0xffff);
		stars[i][1] = coord_translate(rand() & 0xffff);
		stars[i][2] = 0.5 + (rand() % 512) / 256.0;

		if (rand() % 2) 
			stars[i][2] = -stars[i][2];
	}
}

void player_callback(IrmoMethodData *data, void *user_data)
{
	IrmoObjectID id = irmo_method_arg_int(data, "svobj");

	player = irmo_world_get_object_for_id(world, id);
}

void all_object_callback(IrmoObject *object, char *varname, void *user_data)
{
	printf("object %i(%s)::%s changed\n", 
	       irmo_object_get_id(object),
	       irmo_object_get_class(object),
	       varname);
}

static void display_message(IrmoMethodData *data, void *user_data)
{
	puts(irmo_method_arg_string(data, "message"));
}

static void net_disconnected(IrmoConnection *conn, void *user_data)
{
	puts("disconnected from server");
	exit(0);
}

void net_connect(char *host)
{
	IrmoInterface *iface; 
	IrmoInterface *client_interface;

	// build client world

        client_interface = irmo_interface_load(interface_astroclient,
                                               interface_astroclient_length);

	if (!client_interface) {
		fprintf(stderr, "unable to load interface file!\n");
		exit(-1);
	}

	client_world = irmo_world_new(client_interface);
	irmo_interface_unref(client_interface);

	// callbacks
	
	irmo_world_method_watch(client_world, "display_message",
				display_message, NULL);

	// create an object for the player

	client_player_obj = irmo_object_new(client_world, "Player");
	irmo_object_set_string(client_player_obj, "name", getenv("USER"));

	// load server interface

        iface = irmo_interface_load(interface_astro, interface_astro_length);

	if (!iface) {
		fprintf(stderr, "unable to load interface file!\n");
		exit(-1);
	}

	connection = irmo_connect(IRMO_SOCKET_IPV4, 
				  host, SERVER_PORT,
				  iface, client_world);

	if (!connection) {
		fprintf(stderr, "unable to connect to server\n");
		exit(-1);
	}

	irmo_client_set_max_sendwindow(connection, net_limit);

	irmo_client_watch_disconnect(connection, net_disconnected, NULL);

	irmo_interface_unref(iface);

	world = irmo_connection_get_world(connection);

	//	irmo_world_watch_class(world, NULL, NULL, 
	//			all_object_callback, NULL);
	irmo_world_method_watch(client_world, "assoc_player",
				   player_callback, NULL);

	make_stars();
}

void net_disconnect()
{
	irmo_disconnect(connection);
	irmo_connection_unref(connection);
}

void net_run(void)
{
	irmo_connection_run(connection);
}

void net_block(void)
{
	irmo_socket_block(irmo_connection_get_socket(connection), 20);
}

static long long get_object_time(IrmoObject *obj)
{
	long long t = SDL_GetTicks() / 10;

	t += irmo_object_get_id(obj) * 4000;

	return t;
}

static void tumble_translation(IrmoObject *obj)
{
	long long t = get_object_time(obj) % (360 * 256);
	float size = irmo_object_get_int(obj, "scale") / 256.0;
	
	glRotatef(t / size, 0.7, 0.2, 1);
	glRotatef(t / size, 0.5, 1, -0.2);
}

static void twist_translation(IrmoObject *obj)
{
	long long t = get_object_time(obj);

	glRotatef(t, 0, 1, 0);
}

static int get_object_modelnum(IrmoObject *obj)
{
	char *classname = irmo_object_get_class(obj);

	if (!strcmp(classname, "PlayerObject"))
		return MODEL_SHIP1;
	else if (!strcmp(classname, "Asteroid"))
		return MODEL_ROCK1;
	else if (!strcmp(classname, "Missile"))
		return MODEL_MISSILE1;
	else if (!strcmp(classname, "Explosion"))
		return MODEL_EXPLOSION;
	else
		return MODEL_NONE;
}

static void net_render_object(IrmoObject *obj)
{
	GLfloat x, y;
	GLfloat angle;
	GLfloat scale;
	int modelnum;

	//if (obj == player_avatar)
	//	return;
	
	x = coord_translate(irmo_object_get_int(obj, "x"));
	y = coord_translate(irmo_object_get_int(obj, "y"));

	point_normalise(&x, &y);
	
	angle = angle_translate(irmo_object_get_int(obj, "angle"));

	modelnum = get_object_modelnum(obj);

	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle-90, 0, 0, 1);

	if (modelnum == MODEL_ROCK1)
		tumble_translation(obj);
	else if (modelnum == MODEL_MISSILE1)
		twist_translation(obj);

	scale = irmo_object_get_int(obj, "scale") / 512.0;
	
	glScalef(scale, scale, scale);
	
	if (modelnum == MODEL_EXPLOSION) {
		int points;
		GLfloat scale2;

		scale2 = exp(irmo_object_get_int(obj, "time") / 5.0);

		glScalef(scale2, scale2, scale2);
		points = 40 * scale * scale2;

		glColor4f(0.7, 0, 0, 1 /scale2);
		gfx_draw_circle(points);
	} else {
		model_draw(modelnum);
	}

	glPopMatrix();
}

void net_render_objects(IrmoWorld *world)
{
        IrmoIterator *iter;
        IrmoObject *obj;

        iter = irmo_world_iterate_objects(world, "Object");

        while (irmo_iterator_has_more(iter)) {
                obj = irmo_iterator_next(iter);

                net_render_object(obj);
        }

        irmo_iterator_free(iter);
}

void net_render_border()
{
	int max = coord_translate(0xffff);

	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex3f(0, 0, 0);
	glVertex3f(max, 0, 0);
	glVertex3f(max, max, 0);
	glVertex3f(0, max, 0);

	glEnd();	
}

void net_render_stars()
{
	int i;

	glColor3f(1, 1, 1);
	
	glBegin(GL_POINTS);

	for (i=0; i<NUM_STARS; ++i) {
		GLfloat x = stars[i][0], y = stars[i][1], z = stars[i][2];

		point_normalise(&x, &y);

		//glNormal3f(y - avatar_y, x - avatar_x, z);
		glNormal3f(0, 0, 1);
		glVertex3f(x, y, z);
	}
	
	glEnd();
}

void net_render()
{
	IrmoObjectID avatar_id;

	if (player) {
		avatar_id = irmo_object_get_int(player, "avatar");
		
		player_avatar = irmo_world_get_object_for_id(world, avatar_id);
//		printf("player: id %i\n", avatar_id);

		avatar_x = coord_translate(irmo_object_get_int(player_avatar, "x"));
		avatar_y = coord_translate(irmo_object_get_int(player_avatar, "y"));
		avatar_angle
			= angle_translate(irmo_object_get_int(player_avatar,
							      "angle"));
		//avatar_angle = 180;

		avatar_angle_cos = cos(PI * avatar_angle / 180.0);
		avatar_angle_sin = sin(PI * avatar_angle / 180.0);
		
		if (gfx_1stperson) {
			glRotatef(90, -1, 0, 0);
			glTranslatef(0, 0, -0.2);
		}
		
		if (gfx_rotate || gfx_1stperson)
			glRotatef(90-avatar_angle, 0, 0, 1);
		glTranslatef(-avatar_x, -avatar_y, 0);
	}
//	glScalef(1.0/0xffff, 1.0/0xffff, 1);
//	net_render_border();
	net_render_stars();
	
        net_render_objects(world);
}

