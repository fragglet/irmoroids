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

#ifndef ASTEROIDS_MESH_H
#define ASTEROIDS_MESH_H

#include <glib.h>
#include <GL/gl.h>

typedef struct _AstroMesh AstroMesh;
typedef struct _AstroVertex AstroVertex;
typedef struct _AstroPoly AstroPoly;
typedef struct _AstroColor AstroColor;
	
struct _AstroVertex {
	gchar *name;
	GLfloat pt[3];
	GLfloat normal[3];
};

struct _AstroColor {
	gchar *name;
	GLfloat color[4];
};

struct _AstroPoly {
	AstroColor *color;
	GPtrArray *vertexes;
};

struct _AstroMesh {
	GHashTable *vertexes;		// hashed by name
	GHashTable *colors;		// ditto
	GPtrArray *polygons;
};

#endif /* #ifndef ASTEROIDS_MESH_H */

// $Log$
// Revision 1.1  2003/06/09 21:34:36  fraggle
// Initial revision
//
// Revision 1.2  2003/06/09 21:14:02  sdh300
// Add Id tag and copyright notice
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
