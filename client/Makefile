
PACKAGES=irmo glib-2.0
CFLAGS = `pkg-config --cflags $(PACKAGES)` `sdl-config --cflags` -I..
LDFLAGS = `pkg-config --libs $(PACKAGES)` `sdl-config --libs` -lGL -lGLU -lfl
OBJS=main.o mesh.o net.o gfx.o models.o

astro : $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.c : %.lex
	flex -o$@ $<

# $Log: not supported by cvs2svn $

