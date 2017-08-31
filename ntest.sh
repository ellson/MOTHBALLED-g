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

src/g $1 >ntest_out 2>ntest_err
echo "$?" >ntest_rc

grep '^#rc:' $1 >ntest_t
if test $? -eq 0; then
    sed -e 's/^#rc://' <ntest_t >ntest_expected_rc
else
    echo "0" >ntest_expected_rc
fi
cmp ntest_expected_rc ntest_rc
if test $? -ne 0; then
    echo "$1 - rc differs from expected"
fi

grep '^#out:' $1 >ntest_t
if test $? -eq 0; then
    sed -e 's/^#out://' <ntest_t >ntest_expected_out
else
    rm -f ntest_expected_out
    touch ntest_expected_out
fi
cmp ntest_expected_out ntest_out
if test $? -ne 0; then
    echo "$1 - stdout differs from expected"
fi

grep '^#err:' $1 >ntest_t
if test $? -eq 0; then
    sed -e 's/^#err://' <ntest_t >ntest_expected_err
else
    rm -f ntest_expected_err
    touch ntest_expected_err
fi
cmp ntest_expected_err ntest_err
if test $? -ne 0; then
    echo "$1 - stderr differs from expected"
fi
