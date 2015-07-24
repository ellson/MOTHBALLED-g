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

of=${ifn%.s}
# of=${ifn%.g}
ofh=${of}.h
ofc=${of}.c


typeset -A POS NAME SPOS PROPS CHARMAP

node=""
state=""
statelist=""
next=""
nextlist=""
prop=""
proplist=""
indx=0
sindx=0

sm_node() {
    node="$1"
    statelist+=" $1"

    NAME[$1]=""
    SPOS[$1]=$sindx
    charc=0
    for (( i=0; i<${#1}; i++ )); do
        if test $charc -ne 0; then
            NAME[$1]+=","
        fi
        ((charc++))
        NAME[$1]+="'${state:$i:1}'"
	((sindx++)) 
    done
    NAME[$1]+=",'\\0'"
    ((sindx++))
    if test $(( sindx % 2 )) -eq 1; then
        NAME[$1]+=",'\\0'"
        ((sindx++))
    fi
}

sm_end_next() {
    if test "$1" != ""; then
    	nextlist+=" 0"
    	proplist+=" ${POS[$1]}"
    	((indx++))
    fi
}

sm_edge() {
    if test "$1" != "$state"; then
	sm_end_next $state
	state="$1"
        POS[$state]=$indx
    fi
    next=$2
    prop=""
}

sm_list_elem() {
    echo "lists are not expected in grammars" >&2
    exit 1
}

sm_prop() {
    prop="$*"
}

sm_cont() {
    # FIXME - just because we don't use contents on edges...
    sm_end_next $node

    for i in $*; do
        CHARMAP["$i"]="$node"
    done
}

sm_delete() {
    echo "deletes are not expected in grammars" >&2
    exit 1
}

sm_term() {
    if test "$state" != ""; then
        if test "$next" != ""; then
	    nextlist+=" $next"

            if test "$prop" != ""; then
                cnt=0
                for p in $prop; do
	            PROPS[$p]=""
                    if test $cnt -eq 0;then
		        proplist+=" "
                    else
		        proplist+="|"
                    fi
	            proplist+="$prop"
	        done
                prop=""
            else
	        proplist+=" 0"
            fi
	    ((indx++))
	    next=""
        fi
    fi
}

#############################################
# fairly generic functions for g traversal
#    - expects input in the "shell friendly" format of g
#
# incomplete - but sufficient for grammar sm
#    - does not support recursive ACT

typeset -A NODE

g_node() {
    if test "${NODE[$1]}" = ""; then
        NODE[$1]="1"
	sm_node $1
    fi
}

g_edge() {
    g_node $1
    g_node $2
    sm_edge $1 $2
}

g_list() {
    sm_list $*
}

g_prop() {
    sm_prop $*
}
g_cont() {
    sm_cont $*
}
g_delete() {
    sm_delete
}
g_term() {
    sm_term
}

while read op toks; do
    if test "$op" = ""; then continue; fi
    case "$op" in
    '>' | ')' | ']' | '}' ) ;;
    '<' ) g_edge $toks;;
    '(' ) g_list $toks;;
    '[' ) g_prop $toks;;
    '{' ) g_cont $toks;;
    '~' ) g_delete ;;
    ';' ) g_term ;;
    'NUL' ) g_node "NUL";;
    default ) g_node "$op";;
    esac
done <$ifn

##############################################
#
# emit sm output: grammar.h

cat >$ofh <<EOF
/*
 * This is a generated file.  Do not edit.
 */

EOF

####

(
cnt=0
printf "typedef enum {\n"
for p in ${!PROPS[@]}; do
    if test $cnt -ne 0; then 
	printf ",\n"
    fi
    printf "    $p = 1<<$cnt"
    ((cnt++))
done
printf "\n} props_t;\n\n"
) >>$ofh

####
(
cnt=0
printf "typedef enum {\n"
for s in $statelist; do
    if test $cnt -ne 0; then 
	printf ",\n"
    fi
    printf "    $s ${POS[$s]}"
    ((cnt++))
done
printf "\n} state_t;\n\n"
) >>$ofh

##############################################
#
# emit sm output: grammar.c


cat >$ofc <<EOF
/*
 * This is a generated file.  Do not edit.
 */

EOF

####
(
printf "char state_names[] = {\n"
for s in $statelist; do
    printf "    /* %3s */  %s,\n" "${SPOS[$s]}" "${NAME[$s]}"
done
printf "};\n\n"
) >>$ofc

####
(
printf "char char2state[] = {"
for msb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
    printf "\n /* ${msb}0 */  "
    for lsb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
	ch="${msb}${lsb}"
	printf " ${CHARMAP[$ch}]}"
    done
done
printf "\n};\n\n"
) >>$ofc

####
(
printf "/* EBNF (omitting terminals)\n"
for s in $statelist; do
    fieldc=0
#    for i in ${NEXT[$s]}; do
#        if test $fieldc -eq 0; then
#            printf "    %15s ::= " "$s"
#        fi
#        (( fieldc++  ))
#	printf " %s" "$i"
#        printf "%s" "${PROPS[$i]}";
#	if test "${POS[$i]}" != ""; then
#	    printf "|%s" "$i"
#	else
#    	    if test $fieldc -eq 0; then
#                printf "    %15s ::= " "$s"
#	    else
#	        printf " "
#            fi
#            ((fieldc++))
#	    printf "%s" "$i" 
#       fi
#    done
    if test $fieldc -ne 0; then
        printf "\n"
    fi
done
printf "*/\n\n"
) >>$ofc

####
(
printf "char state_machine[] = {\n"
for s in $statelist; do
    tpos=${POS[$s]}
    printf "    /* %3s %15s */  " "$tpos" "$s"
    fieldc=0
    propc=0
#    for i in ${NEXT[$s]}; do
#	if [ -z ${POS[$i]} ]; then
#            if test $propc -eq 0; then
#                printf "%s" "$i"
#	    else
#	        printf "|%s" "$i"
#            fi
#            ((propc++))
#	else
#    	    if test $fieldc -ne 0; then
#                if test $propc -eq 0; then
#                    printf "0"
#                fi
#	        printf ", "
#            fi
#            propc=0
#            hpos=${POS[$i]}
#            ((tpos++))
#	    printf "%d," $((hpos-tpos)) 
#            ((fieldc++))
#        fi
#    done
    if test $fieldc -ne 0; then
        if test $propc -eq 0; then
            printf "0"
        fi
        printf ", "
    fi
    spos=${SPOS[$s]}
    printf "0,$((spos/2)),\n"
done
printf "};\n\n"
) >>$ofc
