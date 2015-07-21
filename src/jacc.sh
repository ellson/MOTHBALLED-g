#!/bin/bash

typeset -A NODE POS SPOS NAME

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

indx=0
sindx=0
prev=""
while read op t h x x props; do
    if test "$op" != "<"; then
        NODE[$op]=""
        NAME[$op]=""
        if test "$prev" != ""; then
 	    emit_node
        fi
	POS[$op]=$indx
	SPOS[$op]=$sindx
        prev=$op
    else
        NODE[$t]+="$h"
        for p in $props; do
            if test "$p" != "]"; then
	        NODE[$t]+=" |$p"
            fi
        done
	((indx++))
        NODE[$t]+=" "
    fi
done
if test "$prev" != ""; then
    emit_node
fi

cat <<EOF
#define ALT 1<<15
#define REP 1<<14
#define SREP 1<<13
#define OPT 1<<12
#define REC 1<<11

EOF

echo "char state_names[] = {"
(
for n in ${!NODE[@]}; do
    printf "  /* %3s %-15s */ %s,\n" "${SPOS[$n]}" "$n" "${NAME[$n]}"
done
) | sort -n -k2
echo "};"
echo ""

echo "unsigned short state_machine[] = {"
(
for n in ${!NODE[@]}; do
    printf "/* %3s %-15s */ " "${POS[$n]}" "$n"
    fieldc=0
    for i in ${NODE[$n]}; do
	if [ -z ${POS[$i]} ]; then
	    echo -n "$i"
	else
    	    if test $fieldc -ne 0; then
	        echo -n ","
            fi
            ((fieldc++))
	    echo -n "${POS[$i]}" 
        fi
    done
    if test $fieldc -ne 0; then
        echo -n ","
    fi
    echo "${SPOS[$n]}<<7,"
done
) | sort -n -k2
echo "};"
echo ""
echo "#define state_machine_start ${POS[ACT]}"
