#ifndef ASTRO_TYPES_H
#define ASTRO_TYPES_H

typedef struct _AstroObject AstroObject;
typedef struct _AstroModel AstroModel;
typedef struct _AstroPlayer AstroPlayer;

#include <glib.h>
#include <irmo.h>

struct _AstroObject {
	IrmoObject *object;
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
AstroObject *universe_new_rock();

#endif /* #ifndef ASTRO_TYPES_H */

// $Log: not supported by cvs2svn $

