CFLAGS = -g -O0 -Wall

g: parse.o grammar.o list.o g.o

parse.o: parse.c grammar.h list.h parse.h

grammar.o: grammar.c grammar.h

list.o: list.c list.h

g.o: g.c parse.h

grammar.png: g g2gv.sh
	./g -g | ./g2gv.sh | dot -Tpng -o grammar.png

# grammar.g can be built by g, but need to avoid the dependency loop
# and allow hand modifications to be made locally and stored in git
grammar.g:
	./g -d >grammar.g

grammar.c grammar.h: grammar.g gacc.sh
	./gacc.sh grammar.g

clean:
	rm -f *.o g grammar.png grammar.[ch]
