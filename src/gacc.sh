#!/bin/bash

if test $# -lt 1; then
   echo "Usage: $0 <grammar spec in g>" >&2
   exit 1
fi

ifn="$1"
if test ! -r "$ifn"; then
   echo "$0 : \"$ifn\" is not readable"
   exit 1
fi

of=${ifn%.g}
of=${ifn%.s}
ofh=${of}.h
ofc=${of}.c

typeset -A NODE POS SPOS NAME PROPS
nodelist=""

emit_node() {
    ((indx++))  # account for terminator
    charc=0
    for (( i=0; i<${#prev}; i++ )); do
        if test $charc -ne 0; then
            NAME[$prev]+=","
        fi
        ((charc++))
        NAME[$prev]+="'${prev:$i:1}'"
	((sindx++)) 
    done
    NAME[$prev]+=",'\\0'"
    ((sindx++))
    if test $(( sindx % 2 )) -eq 1; then
	NAME[$prev]+=",'\\0'"
	((sindx++))
    fi
}

add_node() {
    if test "{NODE[$1]}" = ""; then
        NODE[$i]=" "
	nodelist+="$i "
    fi
}
add_edge() {
    add_node $1
    add_node $2
    NODE[$i]+="$2 "
}
add_list() {
    for i in $*; do
        echo "lists are not supported" >&2
        exit 1
    done
}
add_prop() {
    for i in $*; do
        PROP[$i]=""
    done
}
add_cont() {
    for i in $*; do
        CHAR[$i]=$current_node
    done
}

indx=0
sindx=0
prev=""
nodelist=""
while read op toks; do
    case "$op" in
    '' | '>' | ')' | ']' | '}' ) ;;
    '<' ) add_edge $toks;;
    '(' ) add_list $toks;;
    '[' ) add_prop $toks;;
    '{' ) add_cont $toks;;
    default ) add_node $op
    esac
done <$ifn
