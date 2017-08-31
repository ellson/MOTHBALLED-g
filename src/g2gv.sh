#!/bin/bash

#########################################################################
# Copyright (c) 2017 AT&T Intellectual Property
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors: John Ellson <john.ellson@gmail.com>
#########################################################################

# this is prototype code to verify the conversion behavior
# eventually it will be in C as part of g

e_convert() {
    if test $# -eq 2; then
        newedge="$1--$2"
    else
        hash=$(echo "$*" | shasum -)
        hash=e${hash:5:5}
        newedge="$hash [shape=point label=\"\"];$1--$hash [dir=none]"
        if test $# -gt 2; then
            shift
            newedge+=";$hash--{$*}" 
        fi
    fi
    echo "$newedge"
}

outf=${1%.g}.gv

(echo "graph{node [shape=box]"; cat $1; echo "}") | sed -e \
    "s/_contenthash = \(.*\)$/label=<<table border=\"0\"><tr><td border=\"0\">\\\N<\/td><\/tr><tr><td border=\"0\" href=\"\1.svg\"><img src=\"\1.png\"\/><\/td><\/tr><\/table>>/" \
    >$outf

cp $outf ${outf}.t
while read e; do
    legs=${e#<}
    legs=${legs%>}
    sed -e "s/^$e/$( e_convert $legs )/g" <${outf}.t >${outf}.tt; mv ${outf}.tt ${outf}.t
done < <( grep -o '^<.*>' $outf )
mv -f ${outf}.t $outf

   
# FIXME
#    - add mouseover to display attributes of nodes/edges
#
#    - convert this shell script to a c program that can read canonical g
#       -- save canonical g in g_snapshot
#       
#    - g and g2gv can be the same executable using $0
