ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

.PHONY: rpm
rpm: dist
	rpmbuild -ta g-@VERSION@.tar.gz

