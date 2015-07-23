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


typeset -A STATE_NEXT POS STATE_NAME SPOS PROPS STATE_PROPS CHARMAP

state=""
statelist=""
indx=0
sindx=0

sm_node() {
    if test "$state" != ""; then   # if there was a state still unterminated
        # add terminator
        STATE_NEXT[$state]+=" ,0"
        ((indx++))
        # add index to states's name (always on even addres)
        STATE_NEXT[$state]+=",$(( ${SPOS[$state]} / 2))"
        ((indx++))
    fi

    state=$1
    statelist+=" $state"
    STATE_NEXT[$state]="    "
        
    POS[$state]=$indx
    ((indx++))
    STATE_PROPS[$indx]="0"

    STATE_NAME[$state]=""
    SPOS[$1]=$sindx
    charc=0
    for (( i=0; i<${#1}; i++ )); do
        if test $charc -ne 0; then
            STATE_NAME[$state]+=","
        fi
        ((charc++))
        STATE_NAME[$state]+="'${state:$i:1}'"
	((sindx++)) 
   done
   STATE_NAME[$state]+=",'\\0'"
   ((sindx++))
   if test $(( sindx % 2 )) -eq 1; then
       STATE_NAME[$state]+=",'\\0'"
       ((sindx++))
   fi
}

sm_edge() {
    STATE_NEXT[$1]+=" $2"
    ((indx++))
}

sm_list_elem() {
    echo "lists are not supported" >&2
    exit 1
}

sm_prop() {
        PROPS[$1]=""
        if test "${STATE_PROPS[$indx]}" = "0"; then
            STATE_PROPS[$indx]="$1"
        else
            STATE_PROPS[$indx]+="|$1"
        fi
}

sm_cont() {
    for i in $*; do
        CHARMAP["$i"]=$state
    done
}

#############################################
# fairly generic functions for g traversal
#
# incomplete - but sufficient for sm

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
    for i in $*; do
	sm_list_elem $i
    done
}

g_prop() {
    for i in $*; do
	sm_prop $i
    done
}
g_cont() {
    sm_cont $*
}

while read op toks; do
    case "$op" in
    '' | '>' | ')' | ']' | '}' ) ;;
    '<' ) g_edge $toks;;
    '(' ) g_list $toks;;
    '[' ) g_prop $toks;;
    '{' ) g_cont $toks;;
    default ) g_node $op
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
echo "typedef enum {"
for p in ${!PROPS[@]}; do
    echo "    $p = 1<<$cnt"
    ((cnt++))
done
echo "} props_t;"
echo "" 
) >>$ofh

####
(
printf "typedef enum {\n"
for s in $statelist; do
    printf "    $s ${POS[$s]},\n"
done
printf "} state_t;\n\n"
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
printf "char state_names[] = {"
for s in $statelist; do
    printf "    /* %3s */  %s,\n" "${SPOS[$s]}" "${NAME[$s]}"
done
printf "};\n\n"
) >>$ofc

####
(
printf "/* EBNF (omitting terminals)\n"
for s in $statelist; do
    fieldc=0
    for i in ${STATE_NEXT[$s]}; do
	if [ -z ${POS[$i]} ]; then
	    printf "|%s" "$i"
	else
    	    if test $fieldc -eq 0; then
                printf "    %15s ::= " "$s"
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
for s in $statelist; do
    tpos=${POS[$s]}
    printf "    /* %3s %15s */  " "$tpos" "$s"
    fieldc=0
    propc=0
    for i in ${STATE_NEXT[$s]}; do
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
    spos=${SPOS[$s]}
    printf "0,$((spos/2)),\n"
done
printf "};\n\n"
) >>$ofc

####
(
printf "#define state_machine_start %s\n\n" "${POS[ACT]}"
) >>$ofc

