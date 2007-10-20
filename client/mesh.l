/*
 * Copyright (C) 2002-3 Simon Howard
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details. You should have
 * received a copy of the GNU General Public License along with this
 * program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
%{

#include <stdlib.h>
#include <stdarg.h>

#include "mesh.h"

typedef enum {
	TOKEN_NONE,
	TOKEN_DEFINE,
	TOKEN_VERTEX,
	TOKEN_POLY,
	TOKEN_COLOR,
	TOKEN_COLON,
	TOKEN_NAME, 
	TOKEN_NUMBER,
	TOKEN_MINUS,
	TOKEN_COMMA,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
} token_t;

#define YY_NO_UNPUT 
#define YY_DECL static token_t yylex()

#define YY_INPUT(buf,result,max_size)                                   \
        {                                                               \
		int c = fgetc(yyin);                                    \
		if (c == '\n') ++line_num;                              \
		result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);        \
	}

typedef struct {
	char *name;
	GLfloat val;
} AstroDefine;

static AstroDefine **defines;
static int num_defines;

static AstroMesh *mesh;
static char *yyin_filename;
static int line_num;

%}

COMMENT		"#".*\n
WHITESPACE	[[:space:]]+
NUMBER		[[:digit:]]*(\.[[:digit:]]+)?
NAME		[[:alnum:]_]+

%%

{WHITESPACE}	;
{COMMENT}	;
","		return TOKEN_COMMA;
":"		return TOKEN_COLON;
"{"		return TOKEN_LPAREN;
"}"		return TOKEN_RPAREN;
"-"		return TOKEN_MINUS;
"define"	return TOKEN_DEFINE;
"vertex"	return TOKEN_VERTEX;
"poly"		return TOKEN_POLY;
"color"		return TOKEN_COLOR;
{NUMBER}	return TOKEN_NUMBER;
{NAME}		return TOKEN_NAME;
.		printf("what is this: %s\n", yytext);

%%

static void parse_assert(int expr, char *msg, ...)
{
	va_list l;

	if (!expr) {
		va_start(l, msg);
		fprintf(stderr, "%s: %i: ", yyin_filename, line_num);
		vfprintf(stderr, msg, l);
		fprintf(stderr, "\n");
		va_end(l);
		exit(-1);
	}
}

static AstroDefine *get_define_by_name(char *name)
{
        int i;

        for (i=0; i<num_defines; ++i) {
                if (!strcmp(defines[i]->name, name)) {
                        return defines[i];
                }
        }

        return NULL;
}

static AstroColor *get_color_by_name(AstroMesh *mesh, char *name)
{
        int i;

        for (i=0; i<mesh->num_colors; ++i) {
                if (!strcmp(mesh->colors[i]->name, name)) {
                        return mesh->colors[i];
                }
        }

        return NULL;
}

static AstroVertex *get_vertex_by_name(AstroMesh *mesh, char *name)
{
        int i;

        for (i=0; i<mesh->num_vertices; ++i) {
                if (!strcmp(mesh->vertices[i]->name, name)) {
                        return mesh->vertices[i];
                }
        }

        return NULL;
}

static GLfloat eat_ordinate()
{
	token_t token;

	token = yylex();

	if (token == TOKEN_NAME) {
		AstroDefine *def = get_define_by_name(yytext);

		parse_assert(def != NULL,
			     "unknown define name '%s'", yytext);

		return def->val;
	}

	if (token == TOKEN_MINUS) {
		return -eat_ordinate();
	}

	parse_assert(token == TOKEN_NUMBER,
		     "expecting a valid number");

	return atof(yytext);
}

static void eat_define()
{
	AstroDefine *def;
	token_t token;

	def = malloc(sizeof(AstroDefine));
	
	token = yylex();
	parse_assert(token == TOKEN_NAME, "expected a valid define name");
	parse_assert(get_define_by_name(yytext) == NULL,
		     "multiple defines named '%s'", yytext);

	def->name = strdup(yytext);
 
 	token = yylex();
	parse_assert(token == TOKEN_COLON, "expecting a colon");

	def->val = eat_ordinate();

	// add to list of defines

        defines = realloc(defines,
                          (num_defines + 1) * sizeof(AstroDefine *));

        defines[num_defines] = def;
        ++num_defines;
}

static void eat_vertex()
{
	AstroVertex *vertex;
	token_t token;
	int i;

	vertex = malloc(sizeof(AstroVertex));

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expected a valid vertex name");
	parse_assert(get_vertex_by_name(mesh, yytext) == NULL,
		     "multiple vertices named '%s'", yytext);
	
	vertex->name = strdup(yytext);

	token = yylex();
	parse_assert(token == TOKEN_COLON, "expecting a colon");

	token = yylex();
	parse_assert(token == TOKEN_LPAREN, "expecting '{'");

	for (i=0; i<3; ++i) {
		vertex->pt[i] = eat_ordinate();

		token = yylex();
		if (i == 2) {
			parse_assert(token == TOKEN_RPAREN,
				     "expecting '}' for end of coordinate");
		} else {
			parse_assert(token == TOKEN_COMMA,
				     "expecting a comma between ordinates");
		}
	}
	
        mesh->vertices = realloc(mesh->vertices,
                                 (mesh->num_vertices + 1) * sizeof(AstroVertex *));

        mesh->vertices[mesh->num_vertices] = vertex;
        ++mesh->num_vertices;
}

static void vector_cross(GLfloat *u, GLfloat *v, GLfloat *result)
{
	result[0] = u[1] * v[2] - u[2] * v[1];
	result[1] = u[2] * v[0] - u[0] * v[2];
	result[2] = u[0] * v[1] - u[1] * v[0];
}

static inline void vector_add(GLfloat *u, GLfloat *v, GLfloat *result)
{
	result[0] = u[0] + v[0];
	result[1] = u[1] + v[1];
	result[2] = u[2] + v[2];
}

static inline void vector_sub(GLfloat *u, GLfloat *v, GLfloat *result)
{
	result[0] = u[0] - v[0];
	result[1] = u[1] - v[1];
	result[2] = u[2] - v[2];
}

static void eat_poly()
{
	AstroPoly *poly;
	token_t token;
	AstroVertex **verts;
	GLfloat u[3], v[3], norm[3]; 
	int i;

	poly = malloc(sizeof(AstroPoly));
        memset(poly, 0, sizeof(AstroPoly));

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expecting a valid color name");
	
	poly->color = (AstroColor *) get_color_by_name(mesh, yytext);
	parse_assert(poly->color != NULL, "unknown color '%s'", yytext);

	token = yylex();
	parse_assert(token == TOKEN_LPAREN, "expecting '{'");

	for (i=0; ; ++i) {
		AstroVertex *vertex;
		
		token = yylex();
		parse_assert(token == TOKEN_NAME,
			     "expecting a vertex name");

		vertex = (AstroVertex *) get_vertex_by_name(mesh, yytext);
		parse_assert(vertex != NULL,
			     "vertex '%s' not found", yytext);

                poly->vertices = realloc(poly->vertices,
                                         (poly->num_vertices + 1) 
                                            * sizeof(AstroVertex *));
                poly->vertices[poly->num_vertices] = vertex;
                ++poly->num_vertices;

		token = yylex();
		
		if (token == TOKEN_RPAREN)
			break;

		parse_assert(token == TOKEN_COMMA,
			     "expecting a comma between vertices");
	}

        mesh->polygons = realloc(mesh->polygons,
                                 (mesh->num_polygons + 1) * sizeof(AstroPoly *));
        mesh->polygons[mesh->num_polygons] = poly;
        ++mesh->num_polygons;

	verts = (AstroVertex **) poly->vertices;

	vector_sub(verts[2]->pt, verts[1]->pt, u);
	vector_sub(verts[2]->pt, verts[0]->pt, v);
	vector_cross(u, v, norm);

	for (i=0; i<poly->num_vertices; ++i) {
		AstroVertex *v = (AstroVertex *) poly->vertices[i];
//		printf("norm: %f, %f, %f\n", norm[0], norm[1], norm[2]);
		vector_add(v->normal, norm, v->normal);
	}

	//printf("new polygon (%i vertexes)\n", poly->vertexes->len);
}

static void eat_color()
{
	AstroColor *color;
	token_t token;
	int i;
	
	color = malloc(sizeof(AstroColor));

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expecting a valid color name");
	parse_assert(get_color_by_name(mesh, yytext) == NULL,
		     "multiple colors named '%s'", yytext);
	
	color->name = strdup(yytext);

	token = yylex();
	parse_assert(token == TOKEN_COLON, "expecting a colon");

	token = yylex();
	parse_assert(token == TOKEN_LPAREN, "expecting '{'");

	for (i=0; i<3; ++i) {
		color->color[i] = eat_ordinate();

		token = yylex();

		if (i == 2) {
			parse_assert(token == TOKEN_RPAREN,
				     "expecting '}'");
		} else {
			parse_assert(token == TOKEN_COMMA,
				     "expecting comma between rgb values");
		}
	}

        mesh->colors = realloc(mesh->colors,
                               (mesh->num_colors + 1) * sizeof(AstroColor *));
        mesh->colors[mesh->num_colors] = color;
        ++mesh->num_colors;
}

static void destroy_define(AstroDefine *def)
{
	free(def->name);
	free(def);
}

static void normalise_vector(AstroVertex *vertex)
{
	GLfloat mag;

	mag = vertex->normal[0] * vertex->normal[0]
	    + vertex->normal[1] * vertex->normal[1]
	    + vertex->normal[2] * vertex->normal[2];

	mag = sqrt(mag);

	vertex->normal[0] /= mag;
	vertex->normal[1] /= mag;
	vertex->normal[2] /= mag;
}

AstroMesh *mesh_read_from_file(char *filename)
{
	token_t tok;
        int i;

	yyin_filename = filename;
	line_num = 1;

	mesh = malloc(sizeof(AstroMesh));
        memset(mesh, 0, sizeof(AstroMesh));

	defines = NULL;
        num_defines = 0;
	
	yyin = fopen(filename, "r");

	if (!yyin) {
		fprintf(stderr,
			"mesh_read_from_file: %s not found!\n", filename);
		return NULL;
	}

	while (tok = yylex()) {
		switch (tok) {
			case TOKEN_DEFINE:
				eat_define();
				break;
			case TOKEN_VERTEX:
				eat_vertex();
				break;
			case TOKEN_POLY:
				eat_poly();
				break;
			case TOKEN_COLOR:
				eat_color();
				break;
			default:
				parse_assert(0, "expecting a new definition");
				break;
		}
	}

	fclose(yyin);

        // Normalise all vertices

        for (i=0; i<mesh->num_vertices; ++i) {
                normalise_vector(mesh->vertices[i]);
        }

        for (i=0; i<num_defines; ++i) {
                destroy_define(defines[i]);
        }
        free(defines);

	return mesh;
}

#if 0

int main()
{
	mesh_read_from_file("ship1.mesh");
}

#endif

