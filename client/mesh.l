%{

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

// names for ordinates, hashed by name

typedef struct {
	gchar *name;
	GLfloat val;
} AstroDefine;

static GHashTable *defines;

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

static GLfloat eat_ordinate()
{
	token_t token;

	token = yylex();

	if (token == TOKEN_NAME) {
		AstroDefine *def = (AstroDefine *) 
				g_hash_table_lookup(defines, yytext);

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

	def = g_new0(AstroDefine, 1);
	
	token = yylex();
	parse_assert(token == TOKEN_NAME, "expected a valid define name");
	parse_assert(g_hash_table_lookup(defines, yytext) == NULL,
		     "multiple defines named '%s'", yytext);

	def->name = g_strdup(yytext);
 
 	token = yylex();
	parse_assert(token == TOKEN_COLON, "expecting a colon");

	def->val = eat_ordinate();

	// add to list of defines

	g_hash_table_insert(defines, def->name, def);
}

static void eat_vertex()
{
	AstroVertex *vertex;
	token_t token;
	int i;

	vertex = g_new0(AstroVertex, 1);

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expected a valid vertex name");
	parse_assert(g_hash_table_lookup(mesh->vertexes, yytext) == NULL,
		     "multiple vertexes named '%s'", yytext);
	
	vertex->name = g_strdup(yytext);

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
	
	g_hash_table_insert(mesh->vertexes, vertex->name, vertex);
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

	poly = g_new0(AstroPoly, 1);
	poly->vertexes = g_ptr_array_new();

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expecting a valid color name");
	
	poly->color = (AstroColor *) 
		g_hash_table_lookup(mesh->colors, yytext);
	parse_assert(poly->color != NULL,
		     "unknown color '%s'", yytext);

	token = yylex();
	parse_assert(token == TOKEN_LPAREN, "expecting '{'");

	for (i=0; ; ++i) {
		AstroVertex *vertex;
		
		token = yylex();
		parse_assert(token == TOKEN_NAME,
			     "expecting a vertex name");

		vertex = (AstroVertex *)
			g_hash_table_lookup(mesh->vertexes, yytext);
		parse_assert(vertex != NULL,
			     "vertex '%s' not found", yytext);

		g_ptr_array_add(poly->vertexes, vertex);

		token = yylex();
		
		if (token == TOKEN_RPAREN)
			break;

		parse_assert(token == TOKEN_COMMA,
			     "expecting a comma between vertexes");
	}

	g_ptr_array_add(mesh->polygons, poly);

	verts = (AstroVertex **) poly->vertexes->pdata;

	vector_sub(verts[2]->pt, verts[1]->pt, u);
	vector_sub(verts[2]->pt, verts[0]->pt, u);
	vector_cross(u, v, norm);

	for (i=0; i<poly->vertexes->len; ++i) {
		AstroVertex *v = (AstroVertex *) poly->vertexes->pdata[i];
		vector_add(v->normal, norm, v->normal);
	}

	//printf("new polygon (%i vertexes)\n", poly->vertexes->len);
}

static void eat_color()
{
	AstroColor *color;
	token_t token;
	int i;
	
	color = g_new0(AstroColor, 1);

	token = yylex();
	parse_assert(token == TOKEN_NAME, "expecting a valid color name");
	parse_assert(g_hash_table_lookup(mesh->colors, yytext) == NULL,
		     "multiple colors named '%s'", yytext);
	
	color->name = g_strdup(yytext);

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

	g_hash_table_insert(mesh->colors, color->name, color);
}

static gboolean destroy_defines(gchar *name, AstroDefine *def,
				gpointer user_data)
{
	free(def->name);
	free(def);

	return TRUE;
}

AstroMesh *mesh_read_from_file(gchar *filename)
{
	token_t tok;

	yyin_filename = filename;
	line_num = 1;

	mesh = g_new0(AstroMesh, 1);
	mesh->vertexes = g_hash_table_new(g_str_hash, g_str_equal);
	mesh->polygons = g_ptr_array_new();
	mesh->colors = g_hash_table_new(g_str_hash, g_str_equal);

	defines = g_hash_table_new(g_str_hash, g_str_equal);
	
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

	g_hash_table_foreach_remove(defines, 
				    (GHRFunc) destroy_defines, NULL);
	g_hash_table_destroy(defines);

	return mesh;
}

#if 0

int main()
{
	mesh_read_from_file("ship1.mesh");
}

#endif

// $Log: not supported by cvs2svn $
