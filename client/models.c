
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

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/03/17 18:21:39  sdh300
// Use config.h for MODELS_DIR
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
