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

cat <<EOF

<html>
<head>
<title>The "g" graph language</title>
</head>
<body>
<h1>Introduction (from README.md)</h1>
<pre>
EOF

cat README.md

cat <<EOF
</pre>
<h1>Table of Contents</h1>

( Note. Some of these pages are generated automatically during the build process.   Please  build
before viewing. )


<ol>
<li>How to build "g"
<ul>
<li> Type "make" in the top-directory (i.e. in the same directory as this index.html).
</ul>
<li>The Grammar of "g":
<ul>
<li><a type="text/plain" href="doc/annotated_grammar.ebnf">Annotated eBNF (not definitive)</a>
<li><a type="text/plain" href="src/grammar.g">The definitive source in "g"</a>
<li><a type="text/plain" href="src/grammar.ebnf">Rendered in eBNF from definitive source</a>
<li><a href="src/grammar.svg">Rendered by dot from definitive source</a>
</ul>
<li>Some hand drawn examples:
<ul>
<li><a href="doc/sketches/hello_world.svg">hello_world</a>
<li><a href="doc/sketches/edge_into_container_using_nodepath.svg">edge_into_container_using_nodepath</a>
<li><a href="doc/sketches/edge_into_container_via_port.svg">edge_into_container_via_port</a>
<li><a href="doc/sketches/netlist.svg">netlist</a>
<li><a href="doc/sketches/implement_netlist_with_edge_container.svg">implement_netlist_with_edge_container</a>
<li><a href="doc/sketches/multi_node_multi_edge.svg">multi_node_multi_edge</a>
</ul>
<li><a href="src/">Sources</a>
<ul>
EOF

shopt -s nullglob
for i in src/Makefile src/*.c src/*.sh;do
   echo "<li><a type=\"text/plain\" href=\"$i\">${i#src/}</a>"
   for j in ${i%.c}.[hg]; do
       echo "<a type=\"text/plain\" href=\"$j\">${j#src/}</a>"
   done
done

cat <<EOF
</ul>
<li><a href="src/">Example Graphs</a>
<ul>
EOF

for i in graphs/*.g;do
   echo "<li><a type=\"text/plain\" href=\"$i\">${i#graphs/}</a>"
done

cat <<EOF
</ul>
</ol>

</body>
</html>
EOF

