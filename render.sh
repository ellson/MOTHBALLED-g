#!/bin/bash

rm -rf rendering
mkdir rendering
cd rendering
zcat ../g_snapshot.tgz | tar xf -
for i in *; do
    (echo "graph {"; cat $i; echo "}") >${i}.gv
    dot -Tsvg -Tpng -O ${i}.gv
done
