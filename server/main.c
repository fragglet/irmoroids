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


#include <irmo.h>
#include <sys/time.h>

#include "world.h"

#define SAMPLE_TIME 50

long long get_ms()
{
	struct timeval nowtime;

	gettimeofday(&nowtime, NULL);

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
			usleep(SAMPLE_TIME / 4);
		
		server_run(); 
	}
}

// $Log$
// Revision 1.4  2003/09/03 03:16:24  fraggle
// Start with several small rocks instead of one huge one
//
// Revision 1.3  2003/09/01 14:35:51  fraggle
// Rename Universe -> World
//
// Revision 1.2  2003/08/26 14:58:17  fraggle
// Stop using AF_* in irmoroids.
//
// Revision 1.1.1.1  2003/06/09 21:34:37  fraggle
// Initial sourceforge import
//
// Revision 1.3  2003/06/09 21:14:09  sdh300
// Add Id tag and copyright notice
//
// Revision 1.2  2003/03/22 23:17:52  sdh300
// Collisions between objects
// Rocks explode into smaller rocks
// Scaling of world objects
//
// Revision 1.1.1.1  2003/03/17 17:59:28  sdh300
// Initial import
//
