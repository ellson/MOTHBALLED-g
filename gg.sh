#!/bin/bash

g $* >/dev/null
rm -rf rendering
mkdir rendering
cd rendering
zcat ../g_snapshot.tgz | tar xf -

echo "top.svg:" >Makefile
for g in *.g; do
    i=${g%.g}
    echo -n "$i.svg $i.png: $i.gv" >>Makefile
    echo "$i" >>is_content
    grep -h _contenthash $g | while read x x j; do
        echo -n " $j.svg $j.png" >>Makefile
        echo "$j" >>is_contained
    done
    echo "" >>Makefile
done

sort -u is_content >is_content_t
sort -u is_contained >is_contained_t
diff is_content_t is_contained_t | while read dif top; do 
    if test "$dif" == '<'; then
	cat <<EOF >>Makefile
top.svg: $top.svg
 ln $top.svg top.svg
EOF
	break
    fi
done
rm -f is_cont*

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
