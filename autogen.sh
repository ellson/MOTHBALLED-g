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


G_GIT_DATE=$( git log -n 1 --format=%ci )

if test $? -eq 0; then
    G_VERSION_DATE=$( date -u +%Y%m%d.%H%M -d "$G_GIT_DATE" 2>/dev/null )
    if test $? -ne 0; then
        # try date with FreeBSD syntax
        G_VERSION_DATE=$( date -u -j -f "%Y-%m-%d %H:%M:%S %z" "$G_GIT_DATE" +%Y%m%d.%H%M )
        if test $? -ne 0; then
            echo "Warning: we do not know how to invoke date correctly." >$2
        else
            echo "Version date is based on time of last commit: $G_VERSION_DATE"
        fi
    else
        echo "Version date is based on time of last commit: $G_VERSION_DATE"
    fi
else
    G_VERSION_DATE="0"
    echo "Warning: we do not appear to be running in a git clone." >$2
fi

# initialize version for a "stable" build
cat >./version.m4 <<EOF
dnl g package version number, (as distinct from shared library version)
dnl For the minor number: odd => unstable series
dnl                       even => stable series
dnl For the micro number: 0 => in-progress development
dnl                       timestamp => tar-file snapshot or release
m4_define(g_version_major, 0)
m4_define(g_version_minor, 1)
dnl NB: the next line gets changed to a date/time string for development releases
m4_define(g_version_micro, $G_VERSION_DATE)
m4_define(g_version_date, $G_VERSION_DATE)
m4_define(g_collection, test)
m4_define(g_version_commit, unknown)
EOF

# attempt to suppress ar messages for 'u' when 'D' present.
# need to be set during libtoolize
export AR_FLAGS=crD 

# config/missing is created by autoreconf,  but apparently not recreated if already there.
# This breaks some builds from the graphviz.tar.gz sources.
# Arguably this is an autoconf bug.
rm -f config/missing

autoreconf -v --install --force || exit 1

# ensure config/depcomp exists even if still using automake-1.4
# otherwise "make dist" fails.
touch config/depcomp

# suppress automatic ./configure  is "./autogen.sh NOCONFIG"
if test "$1" != "NOCONFIG"; then
    # don't use any old cache, but create a new one
    rm -f config.cache
    ./configure -C "$@"
fi
