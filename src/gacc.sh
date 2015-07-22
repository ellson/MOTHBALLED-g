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
nodelist=""
while read op t h x x props; do
    if test "$op" != "<"; then
	nodelist+=" $op"
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
	        NODE[$t]+=" $p"
            fi
        done
	((indx++))
        NODE[$t]+=" "
    fi
done
if test "$prev" != ""; then
    emit_node
fi

####

cat <<EOF
/*
 * This is a generated file.  Do not edit.
 */

typedef enum {
    ALT    = 1<<6,      // alternive - one must be satisfied
    OPT    = 1<<5,      // optional
    REP    = 1<<4,      // repeatable   ( REP|OPT means 0 or more )
    SREP   = 1<<3,      // ... with SPACE separators
    REC    = 1<<2       // recursion
} props_t;

EOF

####
printf "char state_names[] = {"
for n in $nodelist; do
    printf "    /* %3s */  %s,\n" "${SPOS[$n]}" "${NAME[$n]}"
done
printf "};\n\n"

####
printf "/* EBNF (omitting terminals)\n"
for n in $nodelist; do
    fieldc=0
    for i in ${NODE[$n]}; do
	if [ -z ${POS[$i]} ]; then
	    printf "|%s" "$i"
	else
    	    if test $fieldc -eq 0; then
                printf "    %15s ::= " "$n"
	    else
	        printf " "
            fi
            ((fieldc++))
	    printf "%s" "$i" 
        fi
    done
    if test $fieldc -ne 0; then
        printf "\n"
    fi
done
printf "*/\n\n"

####
printf "char state_machine[] = {\n"
for n in $nodelist; do
    printf "    /* %3s %15s */  " "${POS[$n]}" "$n"
    fieldc=0
    for i in ${NODE[$n]}; do
	if [ -z ${POS[$i]} ]; then
	    printf "|%s" "$i"
	else
    	    if test $fieldc -ne 0; then
	        printf ","
            fi
	    printf "%s" "${POS[$i]},0" 
        fi
        ((fieldc++))
    done
    if test $fieldc -ne 0; then
        printf ","
    fi
    spos=${SPOS[$n]}
    printf "0,$((spos/2)),\n"
done
printf "};\n\n"

####
printf "#define state_machine_start %s\n\n" "${POS[ACT]}"


####
printf "typedef enum {\n"
for n in $nodelist; do
    printf "    %15s = %s\n" "$n" "${POS[$n]},"
done
printf "} state_t;\n\n"

