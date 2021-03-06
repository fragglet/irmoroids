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

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "SDL.h"

#include <irmo.h>

#include "common/net.h"

#include "models.h"
#include "net.h"

GLfloat main_light_position[]    = {5, 0.1, 0, 1};
//{2.0, 0.0, -10.0, 1};
GLfloat main_light_ambient[]     = {0.1, 0.1, 0.1, 1};
GLfloat main_light_diffuse[]     = {0.5, 0.5, 0.5, 1};

GLfloat material_ambient[]  = {0.15, 0.15, 0.15, 1};
GLfloat material_diffuse[]  = {0.60, 0.60, 0.60, 1};

static void run_sdl_keyup(SDL_Event *ev)
{
	switch (ev->key.keysym.sym) {
	case SDLK_LEFT:
		client_keystate &= ~KEY_LEFT;
		break;
	case SDLK_RIGHT:
		client_keystate &= ~KEY_RIGHT;
		break;
	case SDLK_UP:
		client_keystate &= ~KEY_ACCEL;
		break;
	default:
		return;
	}

	irmo_object_set_int(client_player_obj, "keys", client_keystate);
}

static void run_sdl_keydown(SDL_Event *ev)
{
	switch (ev->key.keysym.sym) {
	case SDLK_r:
		gfx_rotate = !gfx_rotate;
		return;
	case SDLK_q:
		net_disconnect();
		exit(-1);
		return;
	case SDLK_1:
		gfx_1stperson = !gfx_1stperson;
		return;
	case SDLK_LEFT:
		client_keystate |= KEY_LEFT;
		break;
	case SDLK_RIGHT:
		client_keystate |= KEY_RIGHT;
		break;
	case SDLK_UP:
		client_keystate |= KEY_ACCEL;
		break;
	case SDLK_SPACE:
		irmo_world_method_call(world, "fire", 
					  irmo_object_get_id(player));
		return;
	default:
		return;
	}

	irmo_object_set_int(client_player_obj, "keys", client_keystate);
}

static void run_sdl_events()
{
	SDL_Event ev;
	
	while (SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYUP:
			run_sdl_keyup(&ev);
			break;
		case SDL_KEYDOWN:
			run_sdl_keydown(&ev);
			break;
		}
	}
}

extern int irmo_proto_use_preexec;
static char *hostname;

void parse_cmd_line(int argc, char *argv[])
{
	int i;

	hostname = "localhost";

	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "--limit")) {
			++i;
			net_limit = atoi(argv[i]);
		} else if (!strcmp(argv[i], "--no-preexec")) {
			irmo_proto_use_preexec = 0;
		} else {
			hostname = argv[i];
		}
	}
}

int main(int argc, char *argv[])
{
	GLfloat angle=0.01;
	int i;

	parse_cmd_line(argc, argv);

	gfx_init();
	models_init();
	net_connect(hostname);

	printf("initted gfx\n");

	/*
	usleep(100);
	net_run();
	for(;;);
	*/

	while (1) {
		
		gfx_clear();

		if (gfx_1stperson) {
			glMatrixMode(GL_MODELVIEW);
			//glFrustum(-0.4, 0.4, -0.3, 0.3, 0.4, 200);
			gluPerspective(90.0, 1.3333, 0.01, 2000.0);
			glScalef(2.0, 2.0, 2.0);
		} else
			glMatrixMode(GL_PROJECTION);

		glLightfv(GL_LIGHT0, GL_AMBIENT, main_light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, main_light_diffuse);

		glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

		glShadeModel(GL_SMOOTH);
 
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

		glScalef(0.3, 0.3, 0.3);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, main_light_position);
		
//		glScalef(1, 1, 1);
//			gluLookAt(32767, 32767, 10, 18397, 9848, 0, 0, 0, 1);
		
		net_render();
		
		gfx_update();

		run_sdl_events();
		
		net_run();
		net_block();
	}

}

