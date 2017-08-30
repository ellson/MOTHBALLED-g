#!/bin/bash

(echo "graph{"; cat $1; echo "}") | sed -e \
    "s/_contenthash = \(.*\)$/label=<<table border=\"0\"><tr><td border=\"0\">\\\N<\/td><\/tr><tr><td border=\"0\" href=\"\1.svg\"><img src=\"\1.png\"\/><\/td><\/tr><\/table>>/" \
    >${1%.g}.gv

