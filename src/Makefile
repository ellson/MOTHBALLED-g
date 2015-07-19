CFLAGS = -g -O0 -Wall

main: parse.c list.c main.c parse.h list.h

grammar.png: main g2gv.sh
	./main -g | ./g2gv.sh | dot -Tpng -o grammar.png

clean:
	rm -f main grammar.png
