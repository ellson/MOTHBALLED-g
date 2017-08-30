#!/bin/bash

g $* >/dev/null
rm -rf rendering
mkdir rendering
cd rendering
zcat ../g_snapshot.tgz | tar xf -

for i in *; do
    echo -n "$i.svg $i.png : $i.gv" >>Makefile
    grep -h _contenthash $i | while read x x j; do
        echo -n " $j.png" >>Makefile
    done
    echo "" >>Makefile
    (echo "graph{"; cat $i; echo "}") >$i.gv
done


#for i in *; do
#    (echo "graph {"; cat $i; echo "}") >${i}.gv
#    dot -Tsvg -Tpng -O ${i}.gv
#done
