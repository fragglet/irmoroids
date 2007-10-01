// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// Copyright (C) 2002-3 Simon Howard
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

#include <glib.h>
#include <irmo.h>

#include "world.h"

#define SAMPLE_TIME 50

long long get_ms()
{
	GTimeVal nowtime;
	
	g_get_current_time(&nowtime);

	return nowtime.tv_sec * 1000 + nowtime.tv_usec / 1000;
}

int main(int argc, char *argv[])
{
	long long oldmovetime;
	int i;

	srand(time(NULL));
	
	world_init();
	server_init();

	for (i=0; i<4; ++i)
		world_new_rock(-1, -1, 1.5);
	
	for (oldmovetime = get_ms();; ) {
		long long nowtime = get_ms();
		
		if (nowtime - oldmovetime > SAMPLE_TIME) {
			world_run();
			oldmovetime += SAMPLE_TIME;
		} else
			g_usleep(SAMPLE_TIME / 4);
		
		server_run(); 
	}
}

