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

#ifndef ASTRO_NET_H
#define ASTRO_NET_H

#include <irmo.h>

void net_connect(char *host);

extern IrmoConnection *connection;
extern IrmoWorld *world;

extern IrmoWorld *client_world;
extern IrmoObject *client_player_obj;
extern guint client_keystate;

extern IrmoObject *player;
extern IrmoObject *player_avatar;

extern gboolean gfx_rotate, gfx_1stperson;

extern int net_limit;

#endif /* #ifndef ASTRO_NET_H */

// $Log$
// Revision 1.2  2003/09/01 14:35:51  fraggle
// Rename Universe -> World
//
// Revision 1.1.1.1  2003/06/09 21:34:36  fraggle
// Initial sourceforge import
//
// Revision 1.3  2003/06/09 21:14:03  sdh300
// Add Id tag and copyright notice
//
// Revision 1.2  2003/05/21 01:52:00  sdh300
// Add command line options for demo
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
