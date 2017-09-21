#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#########################################################################
# Copyright (c) 2017 AT&T Intellectual Property
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors: John Ellson <john.ellson@gmail.com>
#########################################################################

AC_PREREQ([2.69])

# set:
#   g_version_major
#   g_version_minor
#   g_version_micro
#   g_collection
#   g_version_date
#   g_version_commit
m4_include(./version.m4)

AC_INIT([g], [g_version_major().g_version_minor().g_version_micro()], [john.ellson@gmail.com])

G_VERSION_MAJOR=g_version_major()
G_VERSION_MINOR=g_version_minor()
G_VERSION_MICRO=g_version_micro()

#NB: "stable" or "development"
G_COLLECTION=g_collection()

#NB: date/time of last commit - or "0"
G_VERSION_DATE=g_version_date()

#NB: git commit hash of last commit
G_VERSION_COMMIT=g_version_commit()

G_SOURCES=g/$G_COLLECTION/SOURCES

AC_SUBST([G_VERSION_MAJOR])
AC_SUBST([G_VERSION_MINOR])
AC_SUBST([G_VERSION_MICRO])
AC_SUBST([G_VERSION_DATE])
AC_SUBST([G_VERSION_COMMIT])
AC_SUBST([G_COLLECTION])
AC_SUBST([G_SOURCES])

dnl ===========================================================================

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for header files.
AC_CHECK_HEADERS([ \
    stdio.h \
    stdlib.h \
    stdint.h \
    string.h \
    time.h \
    sys/utsname.h \
    sys/types.h \
    sys/sysinfo.h \
    unistd.h \
    pwd.h \
    errno.h \
    assert.h \
    fcntl.h \
    libtar.h \
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([no-define])
AC_PROG_LIBTOOL
AC_CHECK_PROG(GROFF,groff,groff,false)

# check for pkgconfig
PKG_PROG_PKG_CONFIG

AC_PREFIX_DEFAULT([/usr/local])
if test "x${prefix}" = "xNONE"; then
    prefix=${ac_default_prefix}
    AC_SUBST([prefix])
fi

# automatically check for other dependencies in $prefix first
    if test "x${prefix}" != "x/usr"; then
        CPPFLAGS="$CPPFLAGS -I${prefix}/include"
        LDFLAGS="$LDFLAGS -L${prefix}/lib"
        PKG_CONFIG_PATH="$prefix/lib/pkgconfig$PATHSEPARATOR$PKG_CONFIG_PATH"
        export PKG_CONFIG_PATH
fi

# Checks for libraries.
AC_CHECK_LIB([crypto], [main])
AC_CHECK_LIB([openssl], [main])
AC_CHECK_LIB([tar], [main])
AC_CHECK_LIB([z], [main])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([ \
    clock_gettime \
    sysinfo \
    mkstemp \
    strerror \
    uname \
])

# detect effect of api change in openssl-1.1.0 (fedora-26)
AC_CHECK_FUNCS([EVP_MD_CTX_new])


# debug
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug],[Compile the debug version (default: disabled)]),
        [enable_debug=$enableval],
        [enable_debug=no])
AM_CONDITIONAL([DEBUG], [test $enable_debug = "yes"])
if test "x$enable_debug" = "xyes"; then
    changequote({,})
    CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
    CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
    CFLAGS=`echo "$CFLAGS" | $SED -e 's/-g//g'`
    CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-g//g'`
    changequote([,])
    # dnl add -O0 only if GCC or ICC is used
    if test "$GCC" = "yes" || test "$ICC" = "yes"; then
        CFLAGS="$CFLAGS -g -O0 -Wall"
        CXXFLAGS="$CXXFLAGS -g -O0 -Wall"
    fi
fi

AC_CONFIG_FILES([Makefile \
                 g.spec \
                 src/Makefile \
                 Doxyfile \
])
AC_OUTPUT
