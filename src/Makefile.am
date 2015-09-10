# Process this file with automake to produce Makefile.in

LIBJE_VERSION="0:0:0"

pdfdir = $(pkgdatadir)/doc/pdf
pkgconfigdir = $(libdir)/pkgconfig

LIBS = $(TAR_LIBS) $(Z_LIBS)

pkginclude_HEADERS =\
	context.h \
	dispatch.h \
	emit.h \
	grammar.h \
	hash.h \
	inbuf.h \
	libje.h \
	libje_private.h \
	list.h \
	pattern.h \
	sameas.h \
	token.h

noinst_HEADERS = \
	context.h \
	dispatch.h \
	emit.h \
	grammar.h \
	hash.h \
	inbuf.h \
	libje_private.h \
	list.h \
	pattern.h \
	sameas.h \
	token.h

lib_LTLIBRARIES = libje.la
#pkgconfig_DATA = libje.pc

libje_C_la_SOURCES = \
	grammar.c \
	libje.c \
	parse.c \
	persist.c \
	token.c \
	info.c \
	inbuf.c \
	dispatch.c \
	dumpg.c \
	list.c \
	hash.c \
	sameas.c \
	pattern.c \
	emit.c \
	emit_t.c \
	emit_g.c \
	emit_gv.c 

libje_C_la_LIBADD = $(LIBS)

libje_la_LDFLAGS = -version-info $(LIBJE_VERSION) -no-undefined
libje_la_SOURCES = $(libje_C_la_SOURCES)
libje_la_LIBADD = $(libje_C_la_LIBADD) 

#man_MANS = libje.3 g.1
#pdf_DATA = libje.3.pdf g.1.pdf

bin_PROGRAMS = g g_static

g_SOURCES = g.c
g_CPPFLAGS = 
g_LDADD = libje.la $(LIBS)

g_static_SOURCES = dot.c dot_builtins.c
g_static_CPPFLAGS = 
g_static_LDADD = libje_C.la $(LIBS)

grammar.c: grammar.h

grammar.h grammar.ebnf grammar.gv: grammar.g gacc.sh
	./gacc.sh grammar.g

g.1.pdf: $(srcdir)/g.1
	    - $(GROFF) -Tps -man $(srcdir)/g.1 | $(PS2PDF) - - >g.1.pdf

libje.3.pdf: $(srcdir)/libje.3
	    - $(GROFF) -Tps -man $(srcdir)/libje.3 | $(PS2PDF) - - >libje.3.pdf

EXTRA_DIST = g.1 g.1.pdf libje.3 libje.3.pdf gacc.sh grammar.g

DISTCLEANFILES = g.1.pdf libje.3.pdf
