// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: gfx.c,v 1.2 2003-06-09 21:14:02 sdh300 Exp $
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


#include <SDL.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Surface *screen;

void gfx_init()
{
	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
				  SDL_OPENGL|SDL_GL_DOUBLEBUFFER);

	glClearDepth( 200.0f );

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBegin(GL_POLYGON);

	glVertex2f(-0.5, -0.5);
	glVertex2f(0.5, -0.5);
	glVertex2f(0, 0.5);

	glEnd();
}

void gfx_clear()
{
	glClearDepth( 200.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void gfx_update()
{
	SDL_GL_SwapBuffers();
}

// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
