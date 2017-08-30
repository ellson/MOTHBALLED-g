#!/bin/bash

#########################################################################
# Copyright (c) 2017 AT&T Intellectual Property
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors: John Ellson <john.ellson@gmail.com>
#########################################################################

PROG=$( basename $0 )

if test $# -lt 1; then
    echo "No command given" >&2
    exit 1
fi

mkdir -p testref testdiff
MD5=$( echo "$*" | md5sum - | cut -d' ' -f1 )
OUT=testdiff/$MD5
REF=testref/$MD5
rm -f $OUT.*
echo $* >$OUT.cmd
CMD=$1
shift
$CMD $* >$OUT.out 2>$OUT.err
echo $? >$OUT.rc
cat $OUT.* | md5sum - | cut -d' ' -f1 >$OUT.res

if test "$PROG" = "testref.sh"; then
    mv -f $OUT.* testref/
    cat <<EOF
    COMMAND
    =======
$( cat $REF.cmd )

    STDOUT
    ======
$( cat $REF.out )

    STDERR
    ======
$( cat $REF.err )

    RC
    ==
$( cat $REF.rc )

EOF
else
    if ! cmp -s $REF.res $OUT.res; then
        cat <<EOF
    COMMAND
    =======
$( cat $REF.cmd )

    STDOUT
    ======
    $( diff $REF.out $OUT.out )

    STDERR
    ======
    $( diff $REF.err $OUT.err )

    RC
    ==
    $( diff $REF.rc $OUT.rc )

EOF
    fi
fi
