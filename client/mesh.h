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

#ifndef ASTEROIDS_MESH_H
#define ASTEROIDS_MESH_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

typedef struct _AstroMesh AstroMesh;
typedef struct _AstroVertex AstroVertex;
typedef struct _AstroPoly AstroPoly;
typedef struct _AstroColor AstroColor;
	
struct _AstroVertex {
	char *name;
	GLfloat pt[3];
	GLfloat normal[3];
};

struct _AstroColor {
	char *name;
	GLfloat color[3];
};

struct _AstroPoly {
	AstroColor *color;
        AstroVertex **vertices;
        int num_vertices;
};

struct _AstroMesh {
        AstroVertex **vertices;
        int num_vertices;
        AstroColor **colors;
        int num_colors;
        AstroPoly **polygons;
        int num_polygons;
};

AstroMesh *mesh_read_from_file(char *filename);

#endif /* #ifndef ASTEROIDS_MESH_H */

