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
ofgv=${of}.gv
ofebnf=${of}.ebnf

typeset -A POS NAME SPOS PROPS CHARMAP CONTENT

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
        NAME[$1]="${#1},"
        ((sindx++))
        for (( i=0; i<${#1}; i++ )); do
            NAME[$1]+="'${1:$i:1}',"
            ((sindx++)) 
        done
        if test $(( sindx % 2 )) -eq 1; then
            NAME[$1]+="0,"
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
    CONTENT[$state]="$*"
    for i in $*; do
        CHARMAP["$i"]="$state"
    done
}

sm_delete() {
    echo "deletes are not expected in grammars" >&2
    exit 1
}

sm_term() {
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
    if test "$next" != ""; then
        nextlist=("${nextlist[@]}" "$next")
        next=""
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

#############################################
# iterate over statelist and generate temporary files for inclusion in output files

cat >${ifn}.ebnf <<EOF

Meta grammar:	'|' separates alternates, otherwise the tokens are sequential
		'_' imdicates that a non-ABC character must separate elements (e.g. WS)
		'?' indicates that the token is optional
		'+' indicates that the token is to be repeated 1 or more times
		'*' indicates that the token is to be repeated 0 or more times

EOF

( printf "strict digraph { ordering=out\n"       )  >${ifn}.gv
( printf "typedef enum {\n"                      )  >${ifn}.enum
( printf "char state_machine[] = {\n"            )  >${ifn}.states
( printf "unsigned char state_props[] = {\n"     )  >${ifn}.props
( printf "char state_token[] = {\n"              )  >${ifn}.token
( printf "char state_agaws[] = {\n"              )  >${ifn}.agaws

cnt=0
for s in ${statelist[@]}; do
    ((cnt++))
    indx=${POS[$s]}
    if test $cnt -ne ${#statelist[@]}; then
        comma=","
    else
        comma=""
    fi
    alts=0
    ( printf "%13s ::=" "$s"                     ) >>${ifn}.ebnf
    ( printf "%13s = %s%s" $s $indx $comma       ) >>${ifn}.enum
    ( printf "    /* %3d %12s */  " $indx $s     ) >>${ifn}.states
    ( printf "    /* %3d %12s */  " $indx $s     ) >>${ifn}.props
    ( printf "    /* %3d %12s */  " $indx $s     ) >>${ifn}.token
    ( printf "    /* %3d %12s */  " $indx $s     ) >>${ifn}.agaws
    while true; do
        next=${nextlist[$indx]}
        prop=${proplist[$indx]}
        if test "$next" = ""; then break; fi
        nxtindx=${POS[$next]}

        ord=0
        ws=""
        agaws=0
        nprops=0
        for p in $prop; do
            case $p in
            ALT ) if test $alts -ne 0; then
                      ( printf "\n%17s" "|"      ) >>${ifn}.ebnf
                  fi
                  (( alts++))
                  ;;
            OPT)  ((ord|=1));;
            REP)  ((ord|=2));;
            SREP) ((ord|=2)); ws='_';;
            *) ;;
            esac

            pcnt=0
            for q in ${!PROPS[@]}; do
	        if test "$p" = "$q"; then
	            ((nprops += (1 << pcnt) ))
	        fi
	        ((pcnt++))
            done
        done
        case $ord in
        0 ) ordc="";; 
        1 ) ordc="?";; 
        2 ) ordc="+";; 
        3 ) ordc="*";; 
        esac
        ( printf " %s%s%s" "$ws" "$next" "$ordc" ) >>${ifn}.ebnf
        ( printf "    \"%s\" -> \"%s\" [label=\"%s%s\"]\n" "$s" "$next" "$ws" "$ordc" ) >>${ifn}.gv
        ( printf " %4d," $((nxtindx-indx))       ) >>${ifn}.states
        ( printf " 0x%02x," $nprops              ) >>${ifn}.props
        ( printf " %4d," 0                       ) >>${ifn}.token
        ( printf " %4d," 0                       ) >>${ifn}.agaws
        ((indx++))
    done

    class="${CONTENT[$s]}"
    tokchar=""
    for tokchar in $class; do break; done
    if test "$tokchar" = ""; then
	tokchar=0
    else
	tokchar=0x$tokchar
    fi
    if test "$s" = "BIN" -o "$s" = "NLL" -o "$s" = "WS"; then
        printable=0
    else
        printable=1
    fi
#    ( printf "prop=$prop"                        ) >>${ifn}.agaws
    if test "$class" != ""; then
        altc=0
        ( printf " "                             ) >>${ifn}.ebnf
        for c in $class; do
	    cc=$(( 0x$c ))
            if test $altc -gt 0; then
                if test $(( altc % 8 )) -eq 0; then
                    ( printf "\n%18s" "| "       ) >>${ifn}.ebnf
                else
                    ( printf "|"                 ) >>${ifn}.ebnf
                fi
            fi
            ((altc++))
            if test $printable -eq 1 -a $cc -lt 128 ; then
                ( printf "'\x$c'"                ) >>${ifn}.ebnf
            else
                ( printf "'0x$c'"                ) >>${ifn}.ebnf
            fi
        done
    fi
    spos=${SPOS[$s]}
    ( printf "\n"                                ) >>${ifn}.ebnf
    ( printf "\n"                                ) >>${ifn}.enum
    ( printf " %4d,\n" 0                         ) >>${ifn}.states
    ( printf " %4d,\n" $((spos/2))               ) >>${ifn}.props
    ( printf " %4s,\n" $tokchar                  ) >>${ifn}.token
    ( printf " %4s,\n" $agaws                    ) >>${ifn}.agaws
done
( printf "\n\n"                                  ) >>${ifn}.ebnf
( printf "}\n\n"                                 ) >>${ifn}.gv
( printf "} state_t;\n\n"                        ) >>${ifn}.enum
( printf "};\n\n"                                ) >>${ifn}.states
( printf "};\n\n"                                ) >>${ifn}.props
( printf "};\n\n"                                ) >>${ifn}.token
( printf "};\n\n"                                ) >>${ifn}.agaws

##############################################
# assemble output files
#     grammar.h

cat >$ofh <<EOF
/*
 * This is a generated file.  Do not edit.
 */

EOF

(
    cnt=0
    printf "typedef enum {\n"
    for p in ${!PROPS[@]}; do
        if test $cnt -ne 0; then 
            printf ",\n"
        fi
        printf "%13s = %s" "$p" "1<<$cnt"
        ((cnt++))
    done
    printf "\n} props_t;\n\n"
) >>$ofh

cat ${ifn}.enum >>$ofh

cat >>$ofh  <<EOF
extern unsigned char state_names[];
extern unsigned char char2state[];
extern char state_machine[];
extern unsigned char state_props[];
extern char state_token[];

typedef enum {
	SUCCESS,
	FAIL
} success_t;

#define sizeof_state_machine $((++indx))

extern unsigned char *NAMEP(int si);

EOF

##############################################
# assemble output files
#     grammar.c

cat >$ofc <<EOF
/*
 * This is a generated file.  Do not edit.
 *
 *
 ******************** eBNF - start ***********************

EOF

cat ${ifn}.ebnf >>$ofc

cat >>$ofc <<EOF

 ******************** eBNF - end ************************/

#include "$ofh"

EOF

(
    printf "unsigned char state_names[] = {\n"
    for n in ${namelist[@]}; do
        spos=${SPOS[$n]}
        printf "    /* %3d */  %s\n" "$((spos/2))" "${NAME[$n]}"
    done
    printf "};\n\n"
) >>$ofc

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

cat ${ifn}.states >>$ofc
cat ${ifn}.props  >>$ofc
cat ${ifn}.token  >>$ofc
cat ${ifn}.agaws >>$ofc

cat >>$ofc  <<EOF
unsigned char *NAMEP(int si) {
    while (state_machine[si]) si++;
    return state_names + 2*state_props[si];
}

EOF

##############################################
# assemble output files
#     grammar.ebnf

cat ${ifn}.ebnf   >$ofebnf


##############################################
# assemble output files
#     grammar.gv

cat ${ifn}.gv     >$ofgv

#############################################
# clean up temporary files

rm -f ${ifn}.ebnf ${ifn}.gv ${ifn}.enum ${ifn}.states ${ifn}.props ${ifn}.token ${ifn}.agaws

