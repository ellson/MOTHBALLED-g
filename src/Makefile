CFLAGS = -g -O0 -Wall

g: parse.o dumpg.o grammar.o inbuf.o list.o emit.o emit_t.o emit_g.o g.o

parse.o: parse.c grammar.h list.h parse.h emit.h

dumpg.o: dumpg.c dumpg.h grammar.h

grammar.o: grammar.c grammar.h

list.o: list.c list.h grammar.h

inbuf.o: inbuf.c inbuf.h

emit.o: emit.c emit.h list.h grammar.h

emit_t.o: emit_t.c emit.h list.h grammar.h

emit_g.o: emit_g.c emit.h list.h grammar.h

g.o: g.c parse.h dumpg.h emit.h

grammar.png: g g2gv.sh
	./g -g | ./g2gv.sh | dot -Tpng -o grammar.png

grammar.c: grammar.h

grammar.h grammar.ebnf: grammar.g gacc.sh
	./gacc.sh grammar.g

# grammar.g can be built by g, but need to avoid the dependency loop
# and allow hand modifications to be made locally and stored in git
grammar.g:
	./g -d >grammar.g

clean:
	rm -f *.o g grammar.png grammar.[ch] grammar.ebnf
