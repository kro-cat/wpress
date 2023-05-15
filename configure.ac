AC_PREREQ([2.69])
AC_INIT([wpress], [0.1.0], [bugs@kara-online.net], [skel], [https://github.com/kro-cat/wpress.git])

m4_include([m4/ax_valgrind_check.m4])

AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug], [Enable debug messages (default is no)])],
  [enable_debug=$enableval],
  [enable_debug=no]
)

if test "x$enable_debug" = "xyes"; then
	AC_DEFINE([DEBUG], [], [enable debug messages])
	AC_MSG_NOTICE([debug messages will be enabled])
  WARN_CFLAGS="${WARN_CFLAGS:-} -ggdb"
fi

AC_ARG_ENABLE(static-libgcc,
  [AS_HELP_STRING([--enable-static-libgcc], [Statically compile with libgcc])],
  [CFLAGS="${CFLAGS:-} -static -static-libgcc"],
  []
)

#host switch
AC_CANONICAL_HOST

AS_CASE(["$host_os"],
  [*linux*], [
    OS_VERSION=linux
    AC_DEFINE([OS_LINUX], [], [host_os is linux-based])
  ],
  [*mingw*|*cygwin*], [
    OS_VERSION=windows
    AC_DEFINE([OS_LINUX], [], [host_os is windows-based])
    AC_DEFINE([WINVER], [_WIN32_WINNT_WIN10],
      [Define windows version as WIN10])
    AC_DEFINE([_WIN32_WINNT], [_WIN32_WINNT_WIN10],
      [Define windows version as WIN10])
  ],
  [AC_MSG_WARN([unsupported system: $host_os])]
)
AM_CONDITIONAL([HOST_WINDOWS], [test "x${OS_VERSION}" = "xwindows"])

#autoconf stuff
AM_INIT_AUTOMAKE([subdir-objects])
WARN_CFLAGS="${WARN_CFLAGS} -W -Wall -Wunused -Werror"
AC_SUBST(WARN_CFLAGS)
AC_CONFIG_MACRO_DIRS([m4])
#AC_CONFIG_SUBDIRS([include/lib-buffer])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])
AX_VALGRIND_DFLT([memcheck], [on])
AX_VALGRIND_CHECK

#program config
AC_PROG_CC

#headers
AC_CHECK_HEADERS([unistd.h])

#types
AC_TYPE_SIZE_T

#functions
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strcspn])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strtoull])
AC_FUNC_MALLOC

#makefiles
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
#tests/Makefile
#include/Makefile
AC_OUTPUT
