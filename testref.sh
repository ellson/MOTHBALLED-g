#!/bin/bash

if test $# -lt 1; then
    echo "No command given" >&2
    exit 1
fi

mkdir -p testref
MD5=$( echo "$*" | md5sum - | cut -d' ' -f1 )
OUT=testref/$MD5
rm -f $OUT.*
echo $* >$OUT.cmd
CMD=$1
shift
$CMD $* >$OUT.out 2>$OUT.err
echo $? >$OUT.rc

echo "cmd=$(cat $OUT.cmd)"
echo "out=$(cat $OUT.out)"
echo "err=$(cat $OUT.err)"
echo "rc=$(cat $OUT.rc)"
