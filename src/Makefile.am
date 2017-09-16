# Process this file with automake to produce Makefile.in

#########################################################################
# Copyright (c) 2017 AT&T Intellectual Property
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors: John Ellson <john.ellson@gmail.com>
#########################################################################

LIBG_PROCESS_VERSION="0:0:0"

pdfdir = $(pkgdatadir)/doc/pdf
pkgconfigdir = $(libdir)/pkgconfig

include_HEADERS = \
    libg.h

noinst_HEADERS = \
    container.h \
    dispatch.h \
    doact.h \
    expand.h \
    fatal.h \
    grammar.h \
    identifier.h \
    ikea.h \
    inbuf.h \
    info.h \
    io.h \
    iter.h \
    list.h \
    merge.h \
    parse.h \
    print.h \
    reduce.h \
    rewrite.h \
    sameas.h \
    process.h \
    thread.h \
    token.h \
    tree.h \
    tuple.h \
    types.h \
    vstring.h

noinst_LTLIBRARIES = libg_C.la
lib_LTLIBRARIES = libg.la
#pkgconfig_DATA = libg.pc

libg_C_la_SOURCES = \
    container.c \
    dispatch.c \
    doact.c \
    dumpg.c \
    expand.c \
    fatal.c \
    grammar.c \
    identifier.c \
    ikea.c \
    inbuf.c \
    info.c \
    io.c \
    iter.c \
    list.c \
    merge.c \
    parse.c \
    print.c \
    reduce.c \
	rewrite.c \
    sameas.c \
    process.c \
    thread.c \
    token.c \
    tree.c \
    tuple.c \
    vstring.c

libg_C_la_LIBADD =

libg_la_LDFLAGS = -version-info $(LIBG_PROCESS_VERSION) -no-undefined
libg_la_SOURCES = $(libg_C_la_SOURCES)
libg_la_LIBADD = $(libg_C_la_LIBADD) 

#man_MANS = libg.3 g.1
#pdf_DATA = libg.3.pdf g.1.pdf

bin_SCRIPTS = gg.sh g2gv.sh

bin_PROGRAMS = g g_static

BUILT_SOURCES = grammar.h grammar.c

g_SOURCES = g.c
g_CPPFLAGS = 
g_LDADD = libg.la @LIBS@

g_static_SOURCES = g.c
g_static_CPPFLAGS = 
g_static_LDADD = libg_C.la @LIBS@

grammar.c: grammar.h

grammar.h grammar.ebnf grammar.gv: grammar.g gacc.sh
	./gacc.sh grammar.g

grammar.svg: grammar.gv
	dot -Tsvg grammar.gv -o grammar.svg

g.1.pdf: $(srcdir)/g.1
	    - $(GROFF) -Tps -man $(srcdir)/g.1 | $(PS2PDF) - - >g.1.pdf

libg.3.pdf: $(srcdir)/libg.3
	    - $(GROFF) -Tps -man $(srcdir)/libg.3 | $(PS2PDF) - - >libg.3.pdf

#EXTRA_DIST = g.1 g.1.pdf libg.3 libg.3.pdf gacc.sh grammar.g
EXTRA_DIST = gacc.sh grammar.g gg.sh g2gv.sh

DISTCLEANFILES = *.la g.1.pdf libg.3.pdf grammar.h grammar.c grammar.ebnf grammar.gv grammar.svg
