INCLUDES=$(SDL_CFLAGS) $(IRMO_CFLAGS) -I.. -DINSTALL_ROOT=\"$(pkgdatadir)\"

bin_PROGRAMS = irmoroids

irmoroids_SOURCES=							\
	gfx.c  		main.c		mesh.h		models.c	\
	models.h	net.c		net.h		mesh.l
irmoroids_LDADD=$(IRMO_LIBS) -lGL -lGLU $(SDL_LIBS) 

