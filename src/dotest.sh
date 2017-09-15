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


# Run a test of g with a given test file.  The test file
#  indicated the expected output in special comments:

grep '^#opt:' $1 >test_t
if test $? -eq 0; then
    OPT=$( sed -e 's/^#opt://' <test_t )
else
    OPT=""
fi
grep '^#rc:' $1 >test_t
if test $? -eq 0; then
    RC=$( sed -e 's/^#rc://' <test_t )
else
    RC=0
fi

src/g $OPT $1 >test_out 2>test_err

if test $? -ne $RC; then
    echo "$1 - rc differs from expected"
fi

grep '^#out:' $1 >test_t
if test $? -eq 0; then
    sed -e 's/^#out://' <test_t >test_expected_out
else
    rm -f test_expected_out
    touch test_expected_out
fi
cmp test_expected_out test_out
if test $? -ne 0; then
    echo "$1 - stdout differs from expected"
fi

grep '^#err:' $1 >test_t
if test $? -eq 0; then
    sed -e 's/^#err://' <test_t >test_expected_err
else
    rm -f test_expected_err
    touch test_expected_err
fi
cmp test_expected_err test_err
if test $? -ne 0; then
    echo "$1 - stderr differs from expected"
fi
