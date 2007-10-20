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

#include <GL/gl.h>

#include "common/config.h"

#include "mesh.h"
#include "models.h"

static char *model_files[] = {
	NULL,
	"ship1.mesh",		// MODEL_SHIP1
	"rock1.mesh",		// MODEL_ROCK1
	"missile1.mesh",	// MODEL_MISSILE1
	NULL,			// MODEL_EXPLOSION
};

static AstroMesh *models[NUM_MODELS];

void models_init()
{
        char filename[128];
	int i;

	for (i=0; i<NUM_MODELS; ++i) {
		if (!model_files[i]) {
			models[i] = NULL;
			continue;
		}

                sprintf(filename, "%s/%s", MODELS_DIR, model_files[i]);
		
		models[i] = mesh_read_from_file(filename);

		if (!models[i]) {
			fprintf(stderr, "Unable to load %s\n", filename);
			exit(-1);
		}
	}
}

void model_draw(int modelnum)
{
	AstroMesh *mesh;
	int i, n;

	if (modelnum <= 0 || modelnum >= NUM_MODELS)
		return;

	mesh  = models[modelnum];

	for (i=0; i<mesh->num_polygons; ++i) {
		AstroPoly *poly;

		poly = (AstroPoly *) mesh->polygons[i];

		glColor3fv(poly->color->color);
		
		glBegin(GL_POLYGON);
		//glBegin(GL_LINE_LOOP);

		for (n=0; n<poly->num_vertices; ++n) {
			AstroVertex *v = (AstroVertex *) poly->vertices[n];

			glNormal3fv(v->normal);
//	printf("%f, %f, %f\n", v->normal[0], v->normal[1], v->normal[2]);
			glVertex3fv(v->pt);
		}

		glEnd();
	}
}

