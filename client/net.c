#include <stdio.h>
#include <stdlib.h>
#include <irmo.h>
#include <GL/gl.h>
#include <math.h>

#include "common/config.h"
#include "common/models.h"
#include "common/net.h"

#include "net.h"

#define INTERFACES_DIR "../interfaces"

#define NUM_STARS 256

static GLfloat stars[NUM_STARS][3];

IrmoConnection *connection;
IrmoUniverse *universe;

IrmoUniverse *client_universe;
IrmoObject *client_player_obj;
guint client_keystate;

IrmoObject *player = NULL;
IrmoObject *player_avatar;

gboolean gfx_rotate = TRUE;
gboolean gfx_1stperson = FALSE;

static GLfloat avatar_x, avatar_y;
static GLfloat avatar_angle;
static GLfloat avatar_angle_cos, avatar_angle_sin;

static inline float angle_translate(int angle)
{
	return (angle * 360.0) / 0xffff;
}

static inline float coord_translate(int x)
{
	return x / 4096.0;
}

static void point_normalise(GLfloat *x, GLfloat *y)
{
	GLfloat max = coord_translate(0xffff);

	if (avatar_x - *x > max/2)
		*x += max;
	if (avatar_y - *y > max/2)
		*y += max;
	if (*x - avatar_x > max/2)
		*x -= max;
	if (*y - avatar_y > max/2)
		*y -= max;
}

void make_stars()
{
	int i;

	for (i=0; i<NUM_STARS; ++i) {
		stars[i][0] = coord_translate(rand() & 0xffff);
		stars[i][1] = coord_translate(rand() & 0xffff);
		stars[i][2] = 0.5 + (rand() % 512) / 256.0;

		if (rand() % 2) 
			stars[i][2] = -stars[i][2];
	}
}

void player_callback(IrmoMethodData *data, gpointer user_data)
{
	irmo_objid_t id = irmo_method_arg_int(data, "svobj");

	player = irmo_universe_get_object_for_id(universe, id);
}

void net_connect(char *host)
{
	IrmoInterfaceSpec *spec; 
	IrmoInterfaceSpec *client_spec;

	// build client universe

	client_spec = irmo_interface_spec_new(CLIENT_INTERFACE_FILE);

	if (!spec) {
		fprintf(stderr, "unable to load interface spec file!\n");
		exit(-1);
	}

	client_universe = irmo_universe_new(client_spec);
	irmo_interface_spec_unref(client_spec);

	// create an object for the player

	client_player_obj = irmo_object_new(client_universe, "Player");
	irmo_object_set_string(client_player_obj, "name", getenv("USER"));

	// load server spec

	spec = irmo_interface_spec_new(SERVER_INTERFACE_FILE);

	if (!spec) {
		fprintf(stderr, "unable to load interface spec file!\n");
		exit(-1);
	}


	connection = irmo_connect(AF_INET, 
				  host, SERVER_PORT,
				  spec, client_universe);

	if (!connection) {
		fprintf(stderr, "unable to connect to server\n");
		exit(-1);
	}

	irmo_interface_spec_unref(spec);

	universe = irmo_connection_get_universe(connection);

	irmo_universe_method_watch(client_universe, "assoc_player",
				   player_callback, NULL);

	make_stars();
}

void net_disconnect()
{
	irmo_disconnect(connection);
	irmo_connection_unref(connection);
}

void net_run()
{
	irmo_connection_run(connection);
}

int shown = 0;

long long get_ms()
{
	struct timeval nowtime;

	gettimeofday(&nowtime, NULL);

	return nowtime.tv_sec * 1000 + nowtime.tv_usec / 1000;
}

static long long get_object_time(IrmoObject *obj)
{
	long long t = get_ms() / 10;

	t += irmo_object_get_id(obj) * 4000;

	return t;
}

static void tumble_translation(IrmoObject *obj)
{
	long long t = get_object_time(obj) % (360 * 256);
	float size = irmo_object_get_int(obj, "scale") / 256.0;
	
	glRotatef(t / size, 0.7, 0.2, 1);
	glRotatef(t / size, 0.5, 1, -0.2);
}

static void twist_translation(IrmoObject *obj)
{
	long long t = get_object_time(obj);

	glRotatef(t, 0, 1, 0);
}

static void net_render_foreach(IrmoObject *obj, gpointer user_data)
{
	GLfloat x, y;
	GLfloat angle;
	GLfloat scale;
	int modelnum;

	//if (obj == player_avatar)
	//	return;
	
	x = coord_translate(irmo_object_get_int(obj, "x"));
	y = coord_translate(irmo_object_get_int(obj, "y"));

	point_normalise(&x, &y);
	
	angle = angle_translate(irmo_object_get_int(obj, "angle"));
	modelnum = irmo_object_get_int(obj, "model");

	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle-90, 0, 0, 1);

	if (modelnum == MODEL_ROCK1)
		tumble_translation(obj);
	else if (modelnum == MODEL_MISSILE1)
		twist_translation(obj);

	scale = irmo_object_get_int(obj, "scale") / 256.0;
	
	glScalef(scale, scale, scale);
	
	model_draw(modelnum);
	glPopMatrix();
}

void net_render_border()
{
	int max = coord_translate(0xffff);

	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex3f(0, 0, 0);
	glVertex3f(max, 0, 0);
	glVertex3f(max, max, 0);
	glVertex3f(0, max, 0);

	glEnd();	
}

void net_render_stars()
{
	int i;

	glColor3f(1, 1, 1);
	
	glBegin(GL_POINTS);

	for (i=0; i<NUM_STARS; ++i) {
		GLfloat x = stars[i][0], y = stars[i][1], z = stars[i][2];

		point_normalise(&x, &y);

		//glNormal3f(y - avatar_y, x - avatar_x, z);
		glNormal3f(0, 0, 1);
		glVertex3f(x, y, z);
	}
	
	glEnd();
}

void net_render()
{
	irmo_objid_t avatar_id;

	if (player) {
		avatar_id = irmo_object_get_int(player, "avatar");
		
		player_avatar = irmo_universe_get_object_for_id(universe, avatar_id);
//		printf("player: id %i\n", avatar_id);

		avatar_x = coord_translate(irmo_object_get_int(player_avatar, "x"));
		avatar_y = coord_translate(irmo_object_get_int(player_avatar, "y"));
		avatar_angle
			= angle_translate(irmo_object_get_int(player_avatar,
							      "angle"));
		//avatar_angle = 180;

		avatar_angle_cos = cos(M_PI * avatar_angle / 180.0);
		avatar_angle_sin = sin(M_PI * avatar_angle / 180.0);
		
		glScalef(0.5, 0.5, 0.5);

		if (gfx_1stperson) {
			glRotatef(90, -1, 0, 0);
			glTranslatef(0, 0, -0.2);
		}
		
		if (gfx_rotate || gfx_1stperson)
			glRotatef(90-avatar_angle, 0, 0, 1);
		glTranslatef(-avatar_x, -avatar_y, 0);
	}
//	glScalef(1.0/0xffff, 1.0/0xffff, 1);
//	net_render_border();
	net_render_stars();
	
	irmo_universe_foreach_object(universe, "Object",
				     net_render_foreach, NULL);
}

// $Log: not supported by cvs2svn $
// Revision 1.5  2003/04/21 18:59:04  sdh300
// The models point in the wrong direction, requiring a 90 degree offset to
// angles. Fix this in the client side, not the server side.
//
// Revision 1.4  2003/04/21 18:48:51  sdh300
// Fix rocks not rotating smoothly
//
// Revision 1.3  2003/03/22 23:46:21  sdh300
// Bigger objects spin slower
//
// Revision 1.2  2003/03/22 23:17:47  sdh300
// Collisions between objects
// Rocks explode into smaller rocks
// Scaling of world objects
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
