index.html: g
	./index.html.sh >index.html

g.tgz: g
	tar -czf g.tgz \
		--exclude */.git* \
		--exclude *.tgz \
		--exclude *.md \
		--exclude *.o \
		--exclude src/g \
		../g

.PHONY: g
g:
	( cd src; make )

clean:
	rm -f index.html
	( cd src; make clean )
