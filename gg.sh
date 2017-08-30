#!/bin/bash

g2gv() {
    (echo "graph{"; cat $1; echo "}") | sed -e \
    "s/_contenthash = \(.*\)$/label=<<table border=\"0\"><tr><td border=\"0\">\\\N<\/td><\/tr><tr><td border=\"0\" href=\"\1.svg\"><img src=\"\1.png\"\/><\/td><\/tr><\/table>>/" \
    >${1%.g}.gv
}

g $* >/dev/null
rm -rf rendering
mkdir rendering
cd rendering
zcat ../g_snapshot.tgz | tar xf -

for g in *; do
    i=${g%.g}
    echo -n "$i.svg $i.png: $i.gv" >>Makefile
    grep -h _contenthash $g | while read x x j; do
        echo -n " $j.svg $j.png" >>Makefile
    done
    echo "" >>Makefile
done

cat <<EOF >>Makefile

.SUFFIXES:
.SUFFIXES: .g .gv .png .svg

.gv.png:
 dot -Tpng \$< >\$@

.gv.svg:
 dot -Tsvg \$< >\$@

.g.gv:
 ../g2gv.sh \$< >\$@ 
EOF

sed 's/^ /\t/' <Makefile >Makefile_t && mv -f Makefile_t Makefile

make
