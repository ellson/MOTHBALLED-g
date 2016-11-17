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

typeset -A POS NAME SPOS PROPS CHARMAP CONTENT IDABC VALABC

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

# add newline before all interesting tokens, and a space after
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

Meta grammar:

    '|' separates alternates, otherwise the tokens are sequential
    '_' indicates that a non-ABC character must separate elements (e.g. WS)
    '?' indicates that the token is optional
    '+' indicates that the token is to be repeated 1 or more times
    '*' indicates that the token is to be repeated 0 or more times

Grammar:

EOF

( printf "strict digraph { ordering=out\n"       )  >${ifn}.gv
( printf "typedef enum {\n"                      )  >${ifn}.enum
( printf "char state_machine[] = {\n"            )  >${ifn}.states
( printf "unsigned char state_props[] = {\n"     )  >${ifn}.props
( printf "char state_token[] = {\n"              )  >${ifn}.token
#( printf "char state_agaws[] = {\n"              )  >${ifn}.agaws

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
#    ( printf "    /* %3d %12s */  " $indx $s     ) >>${ifn}.agaws
    while true; do
        next=${nextlist[$indx]}
        prop=${proplist[$indx]}
        if test "$next" = ""; then break; fi
        nxtindx=${POS[$next]}

        if test $alts -ne 0; then
            ( printf "\n%17s" "|"      ) >>${ifn}.ebnf
        fi
        ord=0
        ws=""
        agaws=0
        nprops=0
        for p in $prop; do
            case $p in
            ALT)  ((alts++));;
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
#        ( printf " %4d," 0                       ) >>${ifn}.agaws
        ((indx++))
    done

    case $s in
    IDENTIFIER)
        class="${CONTENT[$s]}"
        if test "$class" != "ABC"; then
            echo "The IDENTIFIER class must contain only ABC" >&2
            exit 1
        fi
        IDABC[$class]=ABC
        ( printf " "                             ) >>${ifn}.ebnf
        ( printf "ABC"                           ) >>${ifn}.ebnf
	;;
    VSTRING)
        class="${CONTENT[$s]}"
#        ( printf "prop=$prop"                    ) >>${ifn}.agaws
        ( printf " "                             ) >>${ifn}.ebnf
        altc=0
	    for c in $class; do
            VALABC[$c]=ABC
	        if test $altc -gt 0; then
		        if test $(( altc % 16 )) -eq 0; then
                    ( printf "\n%18s" "| "       ) >>${ifn}.ebnf
	            else
		            ( printf "|"                 ) >>${ifn}.ebnf
		        fi
            fi
	        ((altc++))
	        ( printf "$c"                        ) >>${ifn}.ebnf
	    done
        ;;
    *)
        class="${CONTENT[$s]}"
        tokchar=""
        for tokchar in $class; do break; done
	    if test "$tokchar" = "" -o "$s" = "IDENTIFIER" -o "$s" = "VSTRING"; then
            tokchar=0
        else
            tokchar=0x$tokchar
        fi
        if test "$s" = "BIN" -o "$s" = "NLL" -o "$s" = "WS"; then
            printable=0
        else
            printable=1
        fi
#        ( printf "prop=$prop"                    ) >>${ifn}.agaws
        if test "$class" != ""; then
            altc=0
            ( printf " "                         ) >>${ifn}.ebnf
            for c in $class; do
                cc=$(( 0x$c ))
                if test $altc -gt 0; then
                    if test $(( altc % 10 )) -eq 0; then
                        ( printf "\n%18s" "| "   ) >>${ifn}.ebnf
                    else
                        ( printf "|"             ) >>${ifn}.ebnf
                    fi
                fi
                ((altc++))
                if test $printable -eq 1 -a $cc -lt 128 ; then
                    ( printf "'\x$c'"            ) >>${ifn}.ebnf
                else
                    ( printf "'0x$c'"            ) >>${ifn}.ebnf
                fi
            done
        fi
	;;
    esac
    spos=${SPOS[$s]}
    ( printf "\n"                                ) >>${ifn}.ebnf
    ( printf "\n"                                ) >>${ifn}.enum
    ( printf " %4d,\n" 0                         ) >>${ifn}.states
    ( printf " %4d,\n" $((spos/2))               ) >>${ifn}.props
    ( printf " %4s,\n" $tokchar                  ) >>${ifn}.token
#    ( printf " %4s,\n" $agaws                    ) >>${ifn}.agaws
done
#( printf "\n\n"                                  ) >>${ifn}.ebnf
( printf "}\n\n"                                 ) >>${ifn}.gv
( printf "} state_t;\n\n"                        ) >>${ifn}.enum
( printf "};\n\n"                                ) >>${ifn}.states
( printf "};\n\n"                                ) >>${ifn}.props
( printf "};\n\n"                                ) >>${ifn}.token
#( printf "};\n\n"                                ) >>${ifn}.agaws

cat >>${ifn}.ebnf <<EOF

Extra grammar:

    Comments in the form of "# ... EOL" are skipped over by the parser.

    IDENTIFIER is a name used for nodes, ports, attributes, disambiguators.

    Whitespace (WS) has no significance in the grammar except in quoted
    IDENTIFIERS or VSTRINGs and as a separator of last resort between
    IDENTIFIERs.

    It is strongly recommended that quoting not be used in IDENTIFIERS.
    C/C++ do not permit quoting or special character in identifiers, 
    but DOT and JSON do, and we may want to input graphs with their conventions.

    That being said, IDENTIFIERS and VSTRINGs can be concatenations of
    quoted and unquoted character sequences.

        for example:       abc"d e f"ghi"j\\\\k"
        is equivalent to:  "abcd e fghij\\\\k"

    Characters that may be unquoted are listed by class in the VSTRING terminal
    above, and also '*' (see "Patterns" below).

    Quoted character sequences are bounded by DQT characters (i.e. '"')
    and may include any characters, except that NLL, DQT, and BSL need
    to be escaped with a BSL (i.e. '\\')

    VSTRINGs are for use in VALUEs.  The unquoted character set allowed
    in VSTRINGs is more relaxed than in IDENTIFIERs.  This permits:
    file paths (DOS and Unix), URLS,  email addresses, signed or unsigned numbers,
    all to be used as values without quoting.

    Examples of valid unquoted VALUEs:

        abcABC123
        user@host.domain
        /unix/file/path
        C:\dos\file\path
        http://some.host:80/some/path?foo=bar
        1
        +2
        -0.5
        1.5E-9
        128*2**32
        95.5%
        $200.00
        1-800-555-1212

    NB. The single-quote character ''' is not special, and has no quoting
    behavior in this grammar.

Patterns:

    If there is an unquoted '*' in any string in a SUBJECT, then the entire
    SUBJECT is considered a pattern, and its ACT a pattern_act.  Patterns
    are used to add ATTRIBUTES and CONTAINER to any future ACT whose SUBJECT
    matches the pattern. The AST matches any character sequence.
    
    Additionally, in queries and deletessd (VERB = QRY or TLD), an
    unquoted '*' in a VSTRING will further constrain the operation
    to NODES or EDGES with VALUES that match.

EOF
##############################################
# assemble output files
#     grammar.h

cat >$ofh <<EOF
/*
 * This is a generated file.  Do not edit.
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

#ifdef __cplusplus
extern "C" {
#endif

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
#define sizeof_state_machine $((++indx))

extern unsigned char state_names[];
extern unsigned char char2state[];
extern unsigned char char2vstate[];
extern char state_machine[];
extern unsigned char state_props[];
extern char state_token[];
extern unsigned char *NAMEP(int si);

#ifdef __cplusplus
}
#endif

#endif
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

(
    printf "unsigned char char2vstate[] = {"
    for msb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
        printf "\n    /* ${msb}0 */   "
        for lsb in 0 1 2 3 4 5 6 7; do
            valabc=${VALABC[${CHARMAP[${msb}${lsb}]}]}
            if test "$valabc" != ""; then
	            printf "%3s," "$valabc"
	        else
                printf "%3s," "${CHARMAP[${msb}${lsb}]}"
	        fi
        done
        printf "\n    /* ${msb}8 */   "
        for lsb in 8 9 a b c d e f; do
            valabc=${VALABC[${CHARMAP[${msb}${lsb}]}]}
            if test "$valabc" != ""; then
	            printf "%3s," "$valabc"
	        else
                printf "%3s," "${CHARMAP[${msb}${lsb}]}"
	        fi
        done
    done
    printf "\n};\n\n"
) >>$ofc

(
    typeset -A NOTVALABC
    printf "/**\n"
    printf " * The character classes that are allowed unquoted in VSTRINGs are\n"
    printf " * perhaps more easily understood by what character classes are *not* allowed,\n"
    printf " * and which *must* be quoted by one of the available quoting mechanisms:\n"
    printf " *\n"
    for msb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
        for lsb in 0 1 2 3 4 5 6 7 8 9 a b c d e f; do
            valabc=${VALABC[${CHARMAP[${msb}${lsb}]}]}
            if test "$valabc" = ""; then
                NOTVALABC[${CHARMAP[${msb}${lsb}]}]=${msb}${lsb}
	        fi
        done
    done
    printf " *       "
    for i in "${!NOTVALABC[@]}"; do
        printf " $i"
    done
    printf "\n *\n"
    printf " * NB. AST is allowed in unquoted VSTRINGS, but retains its string\n"
    printf " * match properties when used in queries or deletes (VERB = QRY or TLD).\n"
    printf " * To use an AST literally in queries or deletes, it must be quoted.\n"
    printf " */\n\n"
) >>$ofc

cat ${ifn}.states >>$ofc
cat ${ifn}.props  >>$ofc
cat ${ifn}.token  >>$ofc
#cat ${ifn}.agaws >>$ofc

cat >>$ofc  <<EOF
/** 
 * obtain the string name of the state, for introspection
 *
 * @param si state index
 * @return pointer to len+chars array from state_names
 */
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
