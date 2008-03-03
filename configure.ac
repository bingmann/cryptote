# $Id$ -*- mode: autoconf -*-

AC_PREREQ(2.59)
AC_INIT(cryptote, 0.1)
AC_CONFIG_SRCDIR(src/cryptote/main.cpp)
AC_CONFIG_AUX_DIR(acscripts)
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(foreign)
# AM_MAINTAINER_MODE

# enable full optimization by configure switch
AC_ARG_ENABLE(optimize,
	AS_HELP_STRING([--enable-optimize],
		       [Build with full optimization @<:@default=no@:>@]),
        [ case "${enableval}" in
          yes)
                CFLAGS="$CFLAGS -O3 -DNDEBUG";
                CXXFLAGS="$CXXFLAGS -O3 -DNDEBUG";
          ;;
          no) ;;
          *) AC_MSG_ERROR(bad value ${enableval} for --enable-optimize) ;;
          esac ],
        [ optimize=false ])

# set debug info flag if no optimization flags are set.
if test "$CFLAGS" == ""; then
    CFLAGS="-g"
fi
if test "$CXXFLAGS" == ""; then
    CXXFLAGS="-g"
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_LANG([C++])

# Checks for typedefs, structures, and compiler characteristics.

# Check for zlib

CHECK_ZLIB

# Check for wxWidgets 2.8.0 or later
AM_OPTIONS_WXCONFIG

AM_PATH_WXCONFIG(2.8.0, wxWin=1)

if test "$wxWin" != 1; then
    AC_MSG_ERROR([
	wxWidgets does not seem to be installed on your system.

        If you think wxWidgets >= 2.8.0 is installed,
	please check that wx-config is in path, the directory
	where wxWidgets libraries are installed (returned by
	'wx-config --libs' command) is in LD_LIBRARY_PATH or
	equivalent variable.
    ])
fi

# Check for libgcrypt which is only used in test cases

AM_PATH_LIBGCRYPT([1.2.2])

AM_CONDITIONAL(HAVE_LIBGCRYPT, test "$LIBGCRYPT_LIBS" != "")

if test "$LIBGCRYPT_LIBS" == ""; then
    AC_MSG_WARN([libgcrypt is not installed.
The main programs can still be built, the library is only
used in test cases.])
fi

# Win32 tools to add icons to the programs

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
    AC_CHECK_TOOL(RESCOMP, windres)
    ;;
esac
AM_CONDITIONAL(GOT_RESCOMP, test x$RESCOMP != x)

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		 libstc/Makefile
		 libenctain/Makefile
		 libenctain/testsuite/Makefile
		 libenctain/speedtest/Makefile
		 src/Makefile
		 src/art/Makefile
		 src/pwgen/Makefile
		 src/pwtutor/Makefile
		 src/cryptote/Makefile])
AC_OUTPUT
