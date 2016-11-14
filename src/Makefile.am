# Process this file with automake to produce Makefile.in

LIBJE_VERSION="0:0:0"

pdfdir = $(pkgdatadir)/doc/pdf
pkgconfigdir = $(libdir)/pkgconfig

include_HEADERS = \
    libg_session.h

noinst_HEADERS = \
    attribute.h \
    compare.h \
    container.h \
    dispatch.h \
    doact.h \
    expand.h \
    fatal.h \
    grammar.h \
    hash.h \
    ikea.h \
    inbuf.h \
    info.h \
    list.h \
    merge.h \
    parse.h \
    pattern.h \
    print.h \
    reduce.h \
    sameas.h \
    session.h \
    thread.h \
    token.h \
    tree.h \
    types.h

#    emit.h \
#    emit_act.h \
#

noinst_LTLIBRARIES = libg_session_C.la
lib_LTLIBRARIES = libg_session.la
#pkgconfig_DATA = libg_session.pc

libg_session_C_la_SOURCES = \
    attribute.c \
    compare.c \
    container.c \
    dispatch.c \
    doact.c \
    dumpg.c \
    expand.c \
    fatal.c \
    grammar.c \
    hash.c \
    ikea.c \
    inbuf.c \
    info.c \
    list.c \
    merge.c \
    parse.c \
    pattern.c \
    print.c \
    reduce.c \
    sameas.c \
    session.c \
    sslhash.c \
    thread.c \
    token.c \
    tree.c

#    emit.c \
#    emit_act.c \
#    emit_g.c \
#    emit_gv.c \
#    emit_ikea.c \
#    emit_t.c \
#	

libg_session_C_la_LIBADD =

libg_session_la_LDFLAGS = -version-info $(LIBJE_VERSION) -no-undefined
libg_session_la_SOURCES = $(libg_session_C_la_SOURCES)
libg_session_la_LIBADD = $(libg_session_C_la_LIBADD) 

#man_MANS = libg_session.3 g.1
#pdf_DATA = libg_session.3.pdf g.1.pdf

bin_PROGRAMS = g g_static

BUILT_SOURCES = grammar.h grammar.c

g_SOURCES = g.c
g_CPPFLAGS = 
g_LDADD = libg_session.la @LIBS@

g_static_SOURCES = g.c
g_static_CPPFLAGS = 
g_static_LDADD = libg_session_C.la @LIBS@

grammar.c: grammar.h

grammar.h grammar.ebnf grammar.gv: grammar.g gacc.sh
	./gacc.sh grammar.g

grammar.svg: grammar.gv
	dot -Tsvg grammar.gv -o grammar.svg

g.1.pdf: $(srcdir)/g.1
	    - $(GROFF) -Tps -man $(srcdir)/g.1 | $(PS2PDF) - - >g.1.pdf

libg_session.3.pdf: $(srcdir)/libg_session.3
	    - $(GROFF) -Tps -man $(srcdir)/libg_session.3 | $(PS2PDF) - - >libg_session.3.pdf

#EXTRA_DIST = g.1 g.1.pdf libg_session.3 libg_session.3.pdf gacc.sh grammar.g
EXTRA_DIST = gacc.sh grammar.g

DISTCLEANFILES = g.1.pdf libg_session.3.pdf grammar.h grammar.c grammar.ebnf grammar.gv grammar.svg
