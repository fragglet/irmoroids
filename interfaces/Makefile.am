
EXTRA_DIST=interfaces.h

noinst_LTLIBRARIES=libinterfaces.la

libinterfaces_la_SOURCES=astroclient.if astro.if

.if.c:
	irmo-interface-compiler $^ -o $@
%.c : %.if
	irmo-interface-compiler $^ -o $@

