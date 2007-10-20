
if HAVE_CLIENT

bin_PROGRAMS=irmoroids irmoroids-server
SUBDIRS=server interfaces client models

else

bin_PROGRAMS=irmoroids-server
SUBDIRS=server interfaces models

endif

irmoroids_SOURCES=
irmoroids_LDADD=                                           \
        client/libirmoroids-client.la                      \
        interfaces/libinterfaces.la                        \
        $(IRMO_LIBS) $(SDL_LIBS) -lGLU @LEXLIB@

irmoroids_server_SOURCES=
irmoroids_server_LDADD=                                    \
        server/libirmoroids-server.la                      \
        interfaces/libinterfaces.la                        \
        $(IRMO_LIBS) $(SDL_LIBS) -lm

