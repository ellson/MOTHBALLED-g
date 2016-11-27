# Process this file with automake to produce Makefile.in

LIBJE_VERSION="0:0:0"

pdfdir = $(pkgdatadir)/doc/pdf
pkgconfigdir = $(libdir)/pkgconfig

include_HEADERS = \
    libg_process.h

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
    iter.h \
    list.h \
    parse.h \
    pattern.h \
    print.h \
    reduce.h \
    sameas.h \
    process.h \
    thread.h \
    token.h \
    tree.h \
    types.h

noinst_LTLIBRARIES = libg_process_C.la
lib_LTLIBRARIES = libg_process.la
#pkgconfig_DATA = libg_process.pc

libg_process_C_la_SOURCES = \
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
    iter.c \
    list.c \
    parse.c \
    pattern.c \
    print.c \
    reduce.c \
    sameas.c \
    process.c \
    sslhash.c \
    thread.c \
    token.c \
    tree.c

libg_process_C_la_LIBADD =

libg_process_la_LDFLAGS = -version-info $(LIBJE_VERSION) -no-undefined
libg_process_la_SOURCES = $(libg_process_C_la_SOURCES)
libg_process_la_LIBADD = $(libg_process_C_la_LIBADD) 

#man_MANS = libg_process.3 g.1
#pdf_DATA = libg_process.3.pdf g.1.pdf

bin_PROGRAMS = g g_static

BUILT_SOURCES = grammar.h grammar.c

g_SOURCES = g.c
g_CPPFLAGS = 
g_LDADD = libg_process.la @LIBS@

g_static_SOURCES = g.c
g_static_CPPFLAGS = 
g_static_LDADD = libg_process_C.la @LIBS@

grammar.c: grammar.h

grammar.h grammar.ebnf grammar.gv: grammar.g gacc.sh
	./gacc.sh grammar.g

grammar.svg: grammar.gv
	dot -Tsvg grammar.gv -o grammar.svg

g.1.pdf: $(srcdir)/g.1
	    - $(GROFF) -Tps -man $(srcdir)/g.1 | $(PS2PDF) - - >g.1.pdf

libg_process.3.pdf: $(srcdir)/libg_process.3
	    - $(GROFF) -Tps -man $(srcdir)/libg_process.3 | $(PS2PDF) - - >libg_process.3.pdf

#EXTRA_DIST = g.1 g.1.pdf libg_process.3 libg_process.3.pdf gacc.sh grammar.g
EXTRA_DIST = gacc.sh grammar.g

DISTCLEANFILES = g.1.pdf libg_process.3.pdf grammar.h grammar.c grammar.ebnf grammar.gv grammar.svg
