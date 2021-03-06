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

#ifndef ASTRO_MODELS_H
#define ASTRO_MODELS_H

enum {
	MODEL_NONE,
	MODEL_SHIP1,
	MODEL_ROCK1,
	MODEL_MISSILE1,
	MODEL_EXPLOSION,                // "virtual" model
	NUM_MODELS
};
 
void models_init();

#endif /* #ifndef ASTRO_MODELS_H */

