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

#include <GL/gl.h>

#include "common/config.h"
#include "common/models.h"

#include "mesh.h"
#include "models.h"

static char *model_files[] = {
	NULL,
	"ship1.mesh",		// MODEL_SHIP1
	"rock1.mesh",		// MODEL_ROCK1
	"missile1.mesh",	// MODEL_MISSILE1
};

static AstroMesh *models[NUM_MODELS];

void models_init()
{
	int i;

	for (i=0; i<NUM_MODELS; ++i) {
		gchar *filename;
		
		if (!model_files[i]) {
			models[i] = NULL;
			continue;
		}

		filename = g_strdup_printf("%s/%s",
					   MODELS_DIR, model_files[i]);
		
		models[i] = mesh_read_from_file(filename);

		if (!models[i]) {
			fprintf(stderr, "Unable to load %s\n", filename);
			exit(-1);
		}
		free(filename);
	}
}

void model_draw(int modelnum)
{
	AstroMesh *mesh;
	int i, n;

	if (modelnum <= 0 || modelnum >= NUM_MODELS)
		return;

	mesh  = models[modelnum];

	for (i=0; i<mesh->polygons->len; ++i) {
		AstroPoly *poly;

		poly = (AstroPoly *) mesh->polygons->pdata[i];

		glColor3fv(poly->color->color);
		
		glBegin(GL_POLYGON);
		//glBegin(GL_LINE_LOOP);

		for (n=0; n<poly->vertexes->len; ++n) {
			AstroVertex *v 
				= (AstroVertex *) poly->vertexes->pdata[n];

			glNormal3fv(v->normal);
//	printf("%f, %f, %f\n", v->normal[0], v->normal[1], v->normal[2]);
			glVertex3fv(v->pt);
		}

		glEnd();
	}
}

// $Log$
// Revision 1.1  2003/06/09 21:34:36  fraggle
// Initial revision
//
// Revision 1.4  2003/06/09 21:14:03  sdh300
// Add Id tag and copyright notice
//
// Revision 1.3  2003/05/21 01:52:00  sdh300
// Add command line options for demo
//
// Revision 1.2  2003/03/17 18:21:39  sdh300
// Use config.h for MODELS_DIR
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
