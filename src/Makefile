CFLAGS = -g -O0 -Wall

main: parse.o list.o main.o

parse.o: grammar.c parse.c grammar.h parse.h list.h

list.o: list.c list.h

main.o: main.c parse.h

grammar.png: main g2gv.sh
	./main -g | ./g2gv.sh | dot -Tpng -o grammar.png

grammar.g: main 
	./main -d >grammar.g

grammar.c.in: grammar.g jacc.sh
	./jacc.sh <grammar.g >grammar.c.in

clean:
	rm -f *.o main grammar.png grammar.g grammar.c.in
