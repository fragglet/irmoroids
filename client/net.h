#ifndef ASTRO_NET_H
#define ASTRO_NET_H

#include <irmo.h>

void net_connect(char *host);

extern IrmoConnection *connection;
extern IrmoUniverse *universe;

extern IrmoUniverse *client_universe;
extern IrmoObject *client_player_obj;
extern guint client_keystate;

extern IrmoObject *player;
extern IrmoObject *player_avatar;

extern gboolean gfx_rotate, gfx_1stperson;

#endif /* #ifndef ASTRO_NET_H */

// $Log: not supported by cvs2svn $
