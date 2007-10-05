
noinst_LTLIBRARIES=libirmoroids-server.la

libirmoroids_server_la_CFLAGS=                             \
        -I..                                               \
        $(IRMO_CFLAGS)                                     \
        -DINSTALL_ROOT=\"$(pkgdatadir)\"

libirmoroids_server_la_SOURCES=                            \
        main.c                                             \
        server.c         server.h                          \
        world.c          world.h
libirmoroids_server_la_LIBADD=$(IRMO_LIBS) -lm

