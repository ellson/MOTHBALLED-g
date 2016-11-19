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

.PHONY: test
test:
	-for f in tests/*g; do ./testdiff.sh src/g $$f; done

.PHONY: testref
testref:
	-for f in tests/*g; do ./testref.sh src/g $$f; done

EXTRA_DIST = g.spec g.spec.in autogen.sh

DISTCLEANFILES = autom4te.cache/* doxygen/*/*/* doxygen/*/*
