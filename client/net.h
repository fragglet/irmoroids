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

extern int net_limit;

#endif /* #ifndef ASTRO_NET_H */

// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
