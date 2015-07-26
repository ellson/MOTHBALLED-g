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
ofh=${of}.h
ofc=${of}.c

typeset -A POS NAME SPOS PROPS CHARMAP

namelist=()
state=""
statelist=()
next=""
nextlist=()
prop=""
proplist=()
indx=0
sindx=0

sm_node() {
    if test "${NAME[$1]}" = ""; then
        namelist=("${namelist[@]}" "$1")
        SPOS[$1]=$sindx
        charc=0
        for (( i=0; i<${#1}; i++ )); do
            if test $charc -ne 0; then
                NAME[$1]+=","
            fi
            ((charc++))
            NAME[$1]+="'${1:$i:1}'"
	    ((sindx++)) 
        done
        NAME[$1]+=",'\\0'"
        ((sindx++))
        if test $(( sindx % 2 )) -eq 1; then
            NAME[$1]+=",'\\0'"
            ((sindx++))
        fi
    fi
}

sm_state() {
    if test "$state" != ""; then
        nextlist=("${nextlist[@]}" "")
        proplist=("${proplist[@]}" "")
        ((indx++))
    fi
    state=$1
    statelist=("${statelist[@]}" "$1")
    POS[$1]=$indx
}

sm_leg1() {
    if test "$state" != "$1"; then
        sm_state $1
    fi
    sm_node $1
}

sm_leg2() {
    sm_node $1
}

sm_edge() {
    next=$2
}

sm_list_elem() {
    echo "lists are not expected in grammars" >&2
    exit 1
}

sm_prop() {
    prop="$*"
}

sm_cont() {
    for i in $*; do
        CHARMAP["$i"]="$state"
    done
}

sm_delete() {
    echo "deletes are not expected in grammars" >&2
    exit 1
}

sm_term() {
    if test "$next" != ""; then
        nextlist=("${nextlist[@]}" "$next")
	next=""

        nprop=""
        if test "$prop" != ""; then
            cnt=0
            for p in $prop; do
	        PROPS[$p]=""
                if test $cnt -ne 0;then
	            nprop+=" $p"
		else
	            nprop+="$p"
		fi
		((cnt++))
	    done
        fi
        proplist=("${proplist[@]}" "$nprop")
        prop=""

	((indx++))
    fi
}

#############################################
# fairly generic functions for g traversal
#    - expects input in the "shell friendly" formatting of g
#    -  ( hence the sed script below )

typeset -A NODE

leg1=""
leg2=""

g_node() {
    sm_term
    sm_state $1
    sm_node $1
}

g_edge() {
    sm_term
    if test "$1" != "="; then leg1="$1"; fi
    if test "$2" != "="; then leg2="$2"; fi
    sm_leg1 $leg1
    sm_leg2 $leg2
    sm_edge $leg1 $leg2
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

# add newline befor all interesting tokens, and a space after
sed '{s/</\n< /g;s/>/\n>/g;s/\[/\n \[ /g;s/\]/\n \]/g;s/{/\n { /g;s/}/\n }/g}' $ifn >${ifn}.s

while read op rest; do
    if test "$op" = ""; then continue; fi
    case "$op" in
    '>' | ')' | ']' | '}' ) ;;
    '<' ) g_edge $rest;;
    '(' ) g_list $rest;;
    '[' ) g_prop $rest;;
    '{' ) g_cont $rest;;
    '~' ) g_delete ;;
    ';' ) g_term ;;
    * ) g_node "$op";;
    esac
done <${ifn}.s

rm -f ${ifn}.s

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
for s in ${statelist[@]}; do
    if test $cnt -ne 0; then 
	printf ",\n"
    fi
    printf "    $s = ${POS[$s]}"
    ((cnt++))
done
printf "\n} state_t;\n\n"
) >>$ofh

cat >>$ofh <<EOF

extern char state_names[];
extern unsigned char char2state[];
extern char state_machine[];
extern unsigned char state_props[];

EOF
##############################################
#
# emit sm output: grammar.c


cat >$ofc <<EOF
/*
 * This is a generated file.  Do not edit.
 */

#include "$ofh"

EOF

####
(
printf "char state_names[] = {\n"
for n in ${namelist[@]}; do
    spos=${SPOS[$n]}
    printf "    /* %3d */  %s,\n" "$((spos/2))" "${NAME[$n]}"
done
printf "};\n\n"
) >>$ofc

####
(
printf "unsigned char char2state[] = {"
for msb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
    printf "\n    /* ${msb}0 */   "
    for lsb in 0 1 2 3 4 5 6 7; do
	printf "%3s," "${CHARMAP[${msb}${lsb}]}"
    done
    printf "\n    /* ${msb}8 */   "
    for lsb in 8 9 a b c d e f; do
	printf "%3s," "${CHARMAP[${msb}${lsb}]}"
    done
done
printf "\n};\n\n"
) >>$ofc

####
(
printf "/******************** eBNF ******************************\n *\n *"
for s in ${statelist[@]}; do
    indx=${POS[$s]}
    printf "%21s ::=" "$s"
    alts=0
    while true; do
        next=${nextlist[$indx]}
        prop=${proplist[$indx]}
        ((indx++))
        if test "$next" = ""; then break; fi
        ord=0
	for p in $prop; do
	    case $p in
            ALT ) if test $alts -ne 0; then
		      printf "\n *%25s" "|"
		  fi
		  (( alts++))
		  ;;
            REP | SREP) ((ord|=2));;
            OPT ) ((ord|=1));;
	    *) ;;
	    esac
        done
	case $ord in
	0 ) ordc="";; 
	1 ) ordc="?";; 
	2 ) ordc="+";; 
	3 ) ordc="*";; 
	esac
        printf " %s%s" "$next" "$ordc"
    done
    printf "\n *"
done
printf "\n ********************************************************/\n\n"
) >>$ofc

####
( printf "char state_machine[] = {\n"        )  >${ofc}.states
( printf "unsigned char state_props[] = {\n" )  >${ofc}.states
for s in ${statelist[@]}; do
    indx=${POS[$s]}
    ( printf "    /* %3d %12s */  " $indx $s ) >>${ofc}.states
    ( printf "    /* %3d %12s */  " $indx $s ) >>${ofc}.props
    while true; do
        next=${nextlist[$indx]}
	prop=${proplist[$indx]}
        ((indx++))
        if test "$next" = ""; then break; fi
        nxtindx=${POS[$next]}

        nprops=0
	for p in $prop; do
	    cnt=0
	    for q in ${!PROPS[@]}; do
		if test "$p" = "$q"; then
		    ((nprops += (1<<cnt) ))
		fi
	        ((cnt++))
	    done
	done

        ( printf " %4d," $((nxtindx-indx))   ) >>${ofc}.states
        ( printf " 0x%02x," $nprops          ) >>${ofc}.props
        ((indx++))
    done
    spos=${SPOS[$s]}
    ( printf " %4d,\n" 0                     ) >>${ofc}.states
    ( printf " %4d,\n" $((spos/2))           ) >>${ofc}.props
done
( printf "};\n\n"                            ) >>${ofc}.states
( printf "};\n\n"                            ) >>${ofc}.props

cat ${ofc}.states ${ofc}.props >>$ofc
rm -f ${ofc}.states ${ofc}.props

cat >>$ofh  <<EOF
#define sizeof_state_machine $indx

EOF
