# Process this file with automake to produce Makefile.in

LIBJE_VERSION="0:0:0"

pdfdir = $(pkgdatadir)/doc/pdf
pkgconfigdir = $(libdir)/pkgconfig

include_HEADERS = \
	libje.h

noinst_HEADERS = \
    attribute.h \
    compare.h \
    container.h \
    dispatch.h \
    doact.h \
    emit.h \
    emit_act.h \
    expand.h \
    fatal.h \
    grammar.h \
    graph.h \
    hash.h \
    ikea.h \
    inbuf.h \
    info.h \
    list.h \
    match.h \
    merge.h \
    pattern.h \
    print.h \
    reduce.h \
    sameas.h \
    session.h \
    success.h \
    thread.h \
    token.h \
    tree.h

noinst_LTLIBRARIES = libje_C.la
lib_LTLIBRARIES = libje.la
#pkgconfig_DATA = libje.pc

libje_C_la_SOURCES = \
    attribute.c \
    compare.c \
    container.c \
    dispatch.c \
    doact.c \
    dumpg.c \
    emit.c \
    emit_act.c \
    emit_g.c \
    emit_gv.c \
    emit_ikea.c \
    emit_t.c \
    expand.c \
    fatal.c \
    grammar.c \
    graph.c \
    hash.c \
    ikea.c \
    inbuf.c \
    info.c \
    libje.c \
    list.c \
    match.c \
    merge.c \
    pattern.c \
    print.c \
    reduce.c \
    sameas.c \
    sslhash.c \
    token.c \
    tree.c

libje_C_la_LIBADD =

libje_la_LDFLAGS = -version-info $(LIBJE_VERSION) -no-undefined
libje_la_SOURCES = $(libje_C_la_SOURCES)
libje_la_LIBADD = $(libje_C_la_LIBADD) 

#man_MANS = libje.3 g.1
#pdf_DATA = libje.3.pdf g.1.pdf

bin_PROGRAMS = g g_static

BUILT_SOURCES = grammar.h

g_SOURCES = g.c
g_CPPFLAGS = 
g_LDADD = libje.la @LIBS@

g_static_SOURCES = g.c
g_static_CPPFLAGS = 
g_static_LDADD = libje_C.la @LIBS@

grammar.h: grammar.c

grammar.c grammar.ebnf grammar.gv: grammar.g gacc.sh
	./gacc.sh grammar.g

grammar.svg: grammar.gv
	dot -Tsvg grammar.gv -o grammar.svg

g.1.pdf: $(srcdir)/g.1
	    - $(GROFF) -Tps -man $(srcdir)/g.1 | $(PS2PDF) - - >g.1.pdf

libje.3.pdf: $(srcdir)/libje.3
	    - $(GROFF) -Tps -man $(srcdir)/libje.3 | $(PS2PDF) - - >libje.3.pdf

#EXTRA_DIST = g.1 g.1.pdf libje.3 libje.3.pdf gacc.sh grammar.g
EXTRA_DIST = gacc.sh grammar.g

DISTCLEANFILES = g.1.pdf libje.3.pdf grammar.h grammar.c grammar.ebnf grammar.gv grammar.svg
