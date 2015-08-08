#!/bin/bash

echo "strict digraph { ordering=out"

sed '{s/</\n< /g;s/>/\n>/g;s/\[/\n \[ /g;s/\]/\n \]/g;s/{/\n { /g;s/}/\n }/g}' |
while read op t h x; do
    if test "$op" = "<"; then
	if test "$t" = "="; then
	    t=$se
        else
	    se=$t
        fi
        echo "  \"$t\" -> \"$h\""
    fi
done

echo "}"
