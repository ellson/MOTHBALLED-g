ACLOCAL_AMFLAGS = -I m4
CFLAGS = -O0 -Wall -g

SUBDIRS = src

.PHONY: rpm
rpm: dist
	rpmbuild -ta g-@VERSION@.tar.gz

.PHONY: doxygen
doxygen:
	doxygen

.PHONY: test
test:
	-for f in tests/*g; do ./testdiff.sh src/g $$f; done

.PHONY: testref
testref:
	-for f in tests/*g; do ./testref.sh src/g $$f; done

EXTRA_DIST = g.spec g.spec.in autogen.sh

