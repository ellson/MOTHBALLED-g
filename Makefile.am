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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

.PHONY: rpm
rpm: dist
	rpmbuild -ta g-@VERSION@.tar.gz

.PHONY: doxygen
doxygen:
	doxygen

.PHONY: check
check:
	cppcheck --enable=all -I. src/*.[ch] >/dev/null


# codespell is a python script:
#      https://github.com/lucasdemarchi/codespell
# install on Fedora26 with:
#      pip install codespell
.PHONY: spellcheck
spellcheck:
	codespell -d -q 3 --skip="*.po,*.ts"

.PHONY: test
test:
	-for f in tests/*g; do ./testdiff.sh src/g $$f; done

.PHONY: testref
testref:
	-for f in tests/*g; do ./testref.sh src/g $$f; done

EXTRA_DIST = g.spec g.spec.in autogen.sh

DISTCLEANFILES = autom4te.cache/* doxygen/*/*/* doxygen/*/*
