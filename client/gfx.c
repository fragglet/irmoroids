
#include <SDL.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Surface *screen;

void gfx_init()
{
	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
				  SDL_OPENGL|SDL_GL_DOUBLEBUFFER);

	glClearDepth( 200.0f );

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBegin(GL_POLYGON);

	glVertex2f(-0.5, -0.5);
	glVertex2f(0.5, -0.5);
	glVertex2f(0, 0.5);

	glEnd();
}

void gfx_clear()
{
	glClearDepth( 200.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
}

void gfx_update()
{
	SDL_GL_SwapBuffers();
}

// $Log: not supported by cvs2svn $
