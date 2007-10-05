
noinst_LTLIBRARIES= libirmoroids-client.la

libirmoroids_client_la_SOURCES=                            \
        gfx.c                                              \
        main.c                                             \
        net.c           net.h                              \
        mesh.l          mesh.h                             \
        models.c        models.h  
libirmoroids_client_la_CFLAGS=                             \
        -I..                                               \
        $(SDL_CFLAGS)                                      \
        $(IRMO_CFLAGS)                                     \
        -DINSTALL_ROOT=\"$(pkgdatadir)\"

libirmoroids_client_la_LIBADD=$(IRMO_LIBS) $(SDL_LIBS) -lGLU @LEXLIB@

