CFLAGS = -g -O0 -Wall

main: parse.o list.o main.o

parse.o: grammar.c parse.c grammar.h parse.h list.h

list.o: list.c list.h

main.o: main.c parse.h

grammar.png: main g2gv.sh
	./main -g | ./g2gv.sh | dot -Tpng -o grammar.png

# grammar.g can be built by main, but need to avoid the dependency loop
# and allow hand modifications to be made locally and stored in git
grammar.g:
	./main -d >grammar.g

ngrammar.c: grammar.g gacc.sh
	./gacc.sh <grammar.g >ngrammar.c

clean:
	rm -f *.o main grammar.png ngrammar.c
