#ifndef ASTRO_TYPES_H
#define ASTRO_TYPES_H

typedef struct _AstroObject AstroObject;
typedef struct _AstroModel AstroModel;
typedef struct _AstroPlayer AstroPlayer;

#include <glib.h>
#include <irmo.h>

typedef enum {
	OBJECT_ROCK,
	OBJECT_SHIP,
	OBJECT_MISSILE,
} AstroObjectType;

struct _AstroObject {
	AstroObjectType type;
	gboolean destroyed;
	IrmoObject *object;
	float scale;
	int size;
	int x, y;
	int angle;
	int dx, dy;
};

struct _AstroPlayer {
	IrmoClient *client;
	IrmoObject *client_obj;		// client side player object
	IrmoObject *player_obj; 	// a Player object about player
	AstroObject *avatar;		// their ship
};

extern IrmoUniverse *universe;
extern GSList *universe_players;
extern GSList *universe_objects;

void universe_init();
AstroObject *universe_object_new(int x, int y, int angle);
AstroObject *universe_new_rock(int x, int y, float scale);

#endif /* #ifndef ASTRO_TYPES_H */

// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//

