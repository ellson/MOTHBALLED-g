#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([g], [0.1], [john.ellson@comcast.net])
AC_CONFIG_SRCDIR([src/pattern.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([tar], [main])
AC_CHECK_LIB([z], [main])

# Checks for header files.
AC_CHECK_HEADERS([ \
    stdio.h \
    stdlib.h \
    string.h \
    time.h \
    sys/utsname.h \
    sys/types.h \
    unistd.h \
    pwd.h \
    errno.h \
    assert.h \
    fcntl.h \
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([ \
    clock_gettime \
    strerror \
    uname \
])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([no-define])
AC_PROG_LIBTOOL
AC_CHECK_PROG(GROFF,groff,groff,false)


AC_CONFIG_FILES([Makefile \
                 src/Makefile \
])
AC_OUTPUT
