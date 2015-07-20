#!/bin/bash

typeset -A NODE

idx=0
nxt_cnt=0
while read op t h x x; do
    if test "$op" != "<"; then
	NODE($op)="$idx $nxt_cnt"
        nxt_cnt=0
    else
        ((nxt_cnt++))
        ((idx++))
    fi
done
