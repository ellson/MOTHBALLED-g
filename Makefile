index.html: src/g index.html.sh
	./index.html.sh >index.html

.PHONY: g.tgz
g.tgz: src/g
	ln -s . g
	find -H g -type f | egrep -v '(.git|g.tgz|.md$$|.o$$|src/g$$)' | xargs tar cfz g.tgz
	rm -f g

src/g:
	( cd src; make )

clean:
	rm -f index.html
	( cd src; make clean )
