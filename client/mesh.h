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

// $Log: not supported by cvs2svn $
