#!/bin/bash

# render a .g graph to ~/public_html/g/top.svg
#     visible at http://localhost/~<user>/g/top.svg

# run g on the input graph,  result is in g_snapshot.tgz
g $* >/dev/null

# prepare diretory for the rendering
mkdir -p ~/public_html/g
rm -rf ~/public_html/g/*
DIR=`pwd`
cd ~/public_html/g
zcat $DIR/g_snapshot.tgz | tar xf -

# primary target of make
echo "top.svg:" >Makefile

# extract containment dependencies
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

# extract the only node that isn't contained and make it top.svg
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

# add suffix rules to Makefile
cat <<EOF >>Makefile

.SUFFIXES:
.SUFFIXES: .g .gv .png .svg

.gv.png:
 dot -Tpng \$< >\$@

.gv.svg:
 dot -Tsvg \$< >\$@

.g.gv:
 $DIR/g2gv.sh \$< >\$@ 
EOF

# why is it so hard to put required tabs in the Makefile ?
sed 's/^ /\t/' <Makefile >Makefile_t && mv -f Makefile_t Makefile

# make does all the work
make

# ensure result is readable from the web
chmod -R 755 .
