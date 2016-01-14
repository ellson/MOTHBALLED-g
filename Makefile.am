ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

.PHONY: rpm
rpm: dist
	rpmbuild -ta g-@VERSION@.tar.gz

.PHONY: doxygen
doxygen:
	doxygen

EXTRA_DIST = g.spec g.spec.in autogen.sh

