#CFLAGS = -g -O0 -Wall
CFLAGS = -DNDEBUG
OBJS = grammar.o parse.o stats.o inbuf.o dumpg.o emit.o emit_t.o emit_g.o g.o
HDRS = grammar.h parse.h stats.h inbuf.h dumpg.h emit.h

all: test grammar.svg

g: ${OBJS}

${OBJS}: ${HDRS}

grammar.c: grammar.h

grammar.h grammar.ebnf: grammar.g gacc.sh
	./gacc.sh grammar.g

# grammar.g can be built by g, but we need to avoid the dependency loop
# and allow hand modifications to be made locally and stored in git
# So here we just check that the output grammar exactly matches the input
test_grammar.g: g
	./g -d >test_grammar.g
	diff grammar.g test_grammar.g

grammar.svg: g g2gv.sh
	./g -d | ./g2gv.sh | dot -Tsvg -o grammar.svg

# grammar.g (or test_grammar.g) provides a convinient test graph
# This test processes 1000 copies of the graph to generate some stats
.PHONY: test
test: g
	(for i in {1..1000}; do ./g -d; done) | ./g -s >/dev/null

clean:
	rm -f ${OBJS} g grammar.png grammar.[ch] grammar.ebnf test_grammar.g grammar.svg
