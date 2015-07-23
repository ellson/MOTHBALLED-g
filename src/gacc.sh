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
	if [ -z $NODE[$t] ]; then
	    nodelist+=" $t"
            NODE[$t]="$h"
            NAME[$t]=""
            if test "$prev" != ""; then
                emit_node
            fi
	    POS[$t]=$indx
            SPOS[$t]=$sindx
            prev=$t
        fi
        prev=$t
        for p in $props; do
            if test "$p" != "]"; then
		PROPS[$p]=""
	        NODE[$t]+=" $p"
            fi
        done
	((indx++))
        NODE[$t]+=" "
    fi
done <$ifn
if test "$prev" != ""; then
    emit_node
fi

####

cat >$ofh <<EOF
/*
 * This is a generated file.  Do not edit.
 */

EOF

####

(
cnt=0
echo "typedef enum {"
for p in ${!PROPS[@]}; do
    echo "    $p = 1<<$cnt"
    ((cnt++))
done
echo "} props_t;"
echo "" 
) >>$ofh

cat >$ofc <<EOF
/*
 * This is a generated file.  Do not edit.
 */

EOF

####
(
printf "char state_names[] = {"
for n in $nodelist; do
    printf "    /* %3s */  %s,\n" "${SPOS[$n]}" "${NAME[$n]}"
done
printf "};\n\n"
) >>$ofc

####
(
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
) >>$ofc

####
(
printf "char state_machine[] = {\n"
for n in $nodelist; do
    tpos=${POS[$n]}
    printf "    /* %3s %15s */  " "$tpos" "$n"
    fieldc=0
    propc=0
    for i in ${NODE[$n]}; do
	if [ -z ${POS[$i]} ]; then
            if test $propc -eq 0; then
                printf "%s" "$i"
	    else
	        printf "|%s" "$i"
            fi
            ((propc++))
	else
    	    if test $fieldc -ne 0; then
                if test $propc -eq 0; then
                    printf "0"
                fi
	        printf ", "
            fi
            propc=0
            hpos=${POS[$i]}
            ((tpos++))
	    printf "%d," $((hpos-tpos)) 
            ((fieldc++))
        fi
    done
    if test $fieldc -ne 0; then
        if test $propc -eq 0; then
            printf "0"
        fi
        printf ", "
    fi
    spos=${SPOS[$n]}
    printf "0,$((spos/2)),\n"
done
printf "};\n\n"
) >>$ofc

####
(
printf "#define state_machine_start %s\n\n" "${POS[ACT]}"
) >>$ofc

####
(
printf "typedef enum {\n"
for n in $nodelist; do
    printf "    $n ${POS[$n]},\n"
done
printf "} state_t;\n\n"
) >>$ofh

