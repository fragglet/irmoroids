INCLUDES=$(IRMO_CFLAGS) -I.. -DINSTALL_ROOT=\"$(pkgdatadir)\" 

bin_PROGRAMS = irmoroids-server

irmoroids_server_SOURCES=						\
	main.c  server.c  server.h  universe.c  universe.h
irmoroids_server_LDADD=$(IRMO_LIBS) -lm

