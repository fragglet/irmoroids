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

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <math.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Surface *screen;

void gfx_shutdown(void)
{
	SDL_Quit();
}

void gfx_init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "cant init SDL: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
				  SDL_OPENGL);

	if (!screen) {
		fprintf(stderr, "Unable to initialise display: %s\n",
				SDL_GetError());
		exit(-1);
	}

	SDL_WM_SetCaption("Irmoroids", NULL);

	glShadeModel(GL_SMOOTH);

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	glClearDepth(200.0);
	glClearColor(0, 0, 0, 0);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	atexit(gfx_shutdown);
}

void gfx_draw_circle(int res)
{
	int i;

	if (res < 4)
		res = 4;

	glBegin(GL_POLYGON);

	for (i=0; i<res; ++i) {
		GLfloat angle = (2 * M_PI * i) / res;

		glVertex2f(cos(angle), sin(angle));
	}

	glEnd();
}

void gfx_clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void gfx_update()
{
	SDL_GL_SwapBuffers();
}

// $Log$
// Revision 1.5  2003/11/17 01:43:21  fraggle
// Rename irmo_objid_t to IrmoObjectID. Fix GL mode which was broken.
//
// Revision 1.4  2003/09/03 03:13:09  fraggle
// Shut down graphics at exit
//
// Revision 1.3  2003/09/02 18:53:56  fraggle
// Use translucency for explosions.
//
// Revision 1.2  2003/09/02 16:54:31  fraggle
// Add explosions
//
// Revision 1.1.1.1  2003/06/09 21:34:35  fraggle
// Initial sourceforge import
//
// Revision 1.2  2003/06/09 21:14:02  sdh300
// Add Id tag and copyright notice
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
