#!/bin/bash

g $* >/dev/null
rm -rf rendering
mkdir rendering
cd rendering
zcat ../g_snapshot.tgz | tar xf -

for i in *; do
    echo -n "$i.svg $i.png: $i.gv" >>Makefile
    grep -h _contenthash $i | while read x x j; do
        echo -n " $j.svg $j.png" >>Makefile
    done
    echo "" >>Makefile
    (echo "graph{"; cat $i; echo "}") | sed -e \
    "s/_contenthash = \(.*\)$/label=<<table border=\"0\"><tr><td border=\"0\">\\N<\/td><\/tr><tr><td border=\"0\" href=\"\1.svg\"><img src=\"\1.png\"\/><\/td><\/tr><\/table>>/" >$i.gv
done

echo "" >>Makefile
echo ".SUFFIXES: .gv .png .svg" >>Makefile
echo "" >>Makefile
echo ".gv.png:" >>Makefile
echo " dot -Tpng \$< >\$@" | sed 's/ /\t/' >>Makefile
echo "" >>Makefile
echo ".gv.svg:" >>Makefile
echo " dot -Tsvg \$< >\$@" | sed 's/ /\t/' >>Makefile

make

#for i in *; do
#    (echo "graph {"; cat $i; echo "}") >${i}.gv
#    dot -Tsvg -Tpng -O ${i}.gv
#done
