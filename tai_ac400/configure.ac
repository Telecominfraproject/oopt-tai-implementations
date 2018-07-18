
dnl Process this file with autoconf to produce a configure script.
AC_INIT([ac400_tai], [1.0.0000])
AC_CONFIG_SRCDIR([])

dnl use local config dir for extras
AC_CONFIG_AUX_DIR(config)

dnl we use auto cheader
AM_CONFIG_HEADER(config.h)

dnl Defines the Language - we need gcc linker ...
AC_LANG_C

dnl Auto make
AM_INIT_AUTOMAKE

dnl Provides control over re-making of all auto files
AM_MAINTAINER_MODE

dnl Required for cases make defines a MAKE=make ??? Why
AC_PROG_MAKE_SET

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC

dnl We will use libtool for making ...
#AC_PROG_LIBTOOL
LT_INIT([shared disable-static])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([sys/types.h stdio.h stdlib.h string.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

dnl Checks for library functions.
#AC_CHECK_FUNCS([strchr strstr strtol strtoull regcomp regexec])

dnl Define an input config option to control debug compile
AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
	yes) debug=true ;;
	no)  debug=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

CFLAGS_TAI_INTERFACE_COMMON="-Wall -Wswitch -Wunused -Werror -fPIC -fno-strict-aliasing -Wextra -Wunused -Wno-unused-parameter"
AC_SUBST(CFLAGS_TAI_INTERFACE_COMMON)

dnl Create the following Makefiles
AC_OUTPUT([Makefile src/Makefile])
