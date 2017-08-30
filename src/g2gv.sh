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

(echo "graph{node [shape=box]"; cat $1; echo "}") | sed -e \
    "s/_contenthash = \(.*\)$/label=<<table border=\"0\"><tr><td border=\"0\">\\\N<\/td><\/tr><tr><td border=\"0\" href=\"\1.svg\"><img src=\"\1.png\"\/><\/td><\/tr><\/table>>/" \
    >${1%.g}.gv

# FIXME
#    - convert edges:   <a b c>  =>   a -- x -- {b c}
#                           where x is a hash of <a b c>
#
#    - add mouseover to display attributes of nodes/edges
#
#    - convert this shell script to a c program that can read canonical g
#       -- save canonical g in g_snapshot
#       
#    - g and g2gv can be the same executable using $0
