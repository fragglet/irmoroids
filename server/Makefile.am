INCLUDES=$(IRMO_CFLAGS) -I.. -DINSTALL_ROOT=\"$(pkgdatadir)\" 

bin_PROGRAMS = irmoroids-server

irmoroids_server_SOURCES=						\
	main.c  server.c  server.h  world.c  world.h
irmoroids_server_LDADD=$(IRMO_LIBS) -lm

