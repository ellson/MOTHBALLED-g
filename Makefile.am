ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

.PHONY: rpm
rpm: dist
	rpmbuild -ta g-@VERSION@.tar.gz


EXTRA_DIST = g.spec g.spec.in autogen.sh

