# $Id$ -*- mode: autoconf -*-

AC_PREREQ(2.59)
AC_INIT(cryptote, 0.6.0)
AC_CONFIG_SRCDIR(src/cryptote/main.cpp)
AC_CONFIG_AUX_DIR(acscripts)

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

# Check for Win32 system and set compilation flag.

AC_MSG_CHECKING(building for Win32)
case "$target_os" in
    *cygwin* | *mingw32*)
        ON_WIN32=true
        AC_MSG_RESULT(yes)
        ;;
    *)
        ON_WIN32=false
        AC_MSG_RESULT(no)
        ;;
esac
AM_CONDITIONAL(ON_WIN32, $ON_WIN32)

AC_MSG_CHECKING(building for MacOSX)
case "$target_os" in
    *darwin*)
        ON_MACOSX=true
        AC_MSG_RESULT(yes)
        ;;
    *)
        ON_MACOSX=false
        AC_MSG_RESULT(no)
        ;;
esac
AM_CONDITIONAL(ON_MACOSX, $ON_MACOSX)

# enable hardware specific code in Botan

AC_MSG_CHECKING(target CPU type)
case "$target_cpu" in
    "i686" | "x86_64")
	AC_DEFINE(BOTAN_TARGET_ARCH_IS_IA32, [1])
	AC_MSG_RESULT(ia32)
	;;
    "powerpc")
	AC_DEFINE(BOTAN_TARGET_ARCH_IS_PPC, [1])
	AC_MSG_RESULT(ppc)
	;;
    *)
	AC_MSG_RESULT(unknown)
        ;;
esac

# enable full optimization by configure switch

AC_ARG_ENABLE(optimize,
	AS_HELP_STRING([--enable-optimize],
		       [Build with full optimization @<:@default=no@:>@]),
        [ case "${enableval}" in
          yes)
                CFLAGS="$CFLAGS -O3 -fomit-frame-pointer -DNDEBUG";
                CXXFLAGS="$CXXFLAGS -O3 -fomit-frame-pointer -DNDEBUG";
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

AM_CONDITIONAL(CROSS_COMPILATION, test "$cross_compiling" == "yes")

# Check for gettext programs

AC_CHECK_PROG(MSGFMT, msgfmt, msgfmt)
AC_CHECK_PROG(MSGMERGE, msgmerge, msgmerge)
AC_CHECK_PROG(XGETTEXT, xgettext, xgettext)

AM_CONDITIONAL(GOT_GETTEXT, test "$MSGFMT" != "" -a "$MSGMERGE" != "" -a "$XGETTEXT" != "")

# Check whether to enable gcov coverage flags and macros

AC_ARG_ENABLE(gcov,
    AS_HELP_STRING([--enable-gcov],
                   [enable test coverage with gcov @<:@default=no@:>@]),
	[case "${enableval}" in
         yes) gcov=true ;;
         no)  gcov=false ;;
         *)   AC_MSG_ERROR([bad value ${enableval} for --enable-gcov]) ;;
         esac],
        [gcov=false])

AM_CONDITIONAL(GCOV, test "x$gcov" = "xtrue")

if test "x$gcov" = "xtrue"; then
    CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
    CXXFLAGS="$CXXFLAGS -fprofile-arcs -ftest-coverage"
fi

# configure switch to disable WebUpdateCheck by default

AC_MSG_CHECKING(whether to disable WebUpdateCheck by default)

AC_ARG_ENABLE(default-webupdatecheck,
	AS_HELP_STRING([--disable-default-webupdatecheck],
		       [Disable WebUpdateCheck by default. @<:@default=no@:>@]),
        [ case "${enableval}" in
          yes) disable_webupdatecheck=0 ;;
          no) disable_webupdatecheck=1 ;;
          *) AC_MSG_ERROR(bad value ${enableval} for --disable-default-webupdatecheck) ;;
          esac ],
        [ disable_webupdatecheck=0 ])

if test "$disable_webupdatecheck" != 0; then
    AC_DEFINE(DISABLE_WEBUPDATECHECK, 1)
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

# Checks for typedefs, structures, and compiler characteristics.

# Check for wxWidgets 2.8.0 or later

AC_LANG([C++])

AM_OPTIONS_WXCONFIG

AM_PATH_WXCONFIG(2.9.2, [wxWin3=1], [wxWin3=0], [aui,stc,html,adv,net,core])

if test "$wxWin3" == 0; then
        AC_MSG_WARN([Could not find wxWidgets >= 2.9.2, testing for 2.8.0.])

	AM_PATH_WXCONFIG(2.8.0, [wxWin=1], [wxWin=0], [aui,html,adv,net,core])

	if test "$wxWin" == 0; then
	    AC_MSG_ERROR([
		wxWidgets does not seem to be installed on your system.

	        If you think wxWidgets >= 2.8.0 is installed,
		please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' command) is in LD_LIBRARY_PATH or
		equivalent variable.
	    ])
	fi
fi

AM_CONDITIONAL(HAVE_WXWIDGETS3, test "$wxWin3" != 0)

# Check for tex2rtf utility from wxWidgets to regenerate help.

AC_CHECK_TOOL(TEX2RTF, tex2rtf)

AM_CONDITIONAL(GOT_TEX2RTF, test "$TEX2RTF" != "")

# Check for zlib - based on CHECK_ZLIB()

AC_ARG_WITH(zlib,
	AS_HELP_STRING([--with-zlib=DIR],
		       [root directory path of zlib installation
		        @<:@defaults to /usr/local or /usr if not found in /usr/local@:>@]),
[if test -d "$withval"
  then
    ZLIB_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
fi])

if test ! -f "${ZLIB_HOME}/include/zlib.h"; then
  ZLIB_HOME=/usr/local
  if test ! -f "${ZLIB_HOME}/include/zlib.h"; then
    ZLIB_HOME=/usr
  fi
fi

AC_MSG_CHECKING(zlib in ${ZLIB_HOME})

ZLIB_OLD_LDFLAGS=$LDFLAGS
ZLIB_OLD_CPPFLAGS=$CPPFLAGS
LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
AC_LANG_SAVE
AC_LANG_C
AC_CHECK_LIB(z, inflateEnd, [zlib_cv_libz=yes], [zlib_cv_libz=no])
AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cv_zlib_h=no])
AC_LANG_RESTORE
LDFLAGS="$ZLIB_OLD_LDFLAGS"
CPPFLAGS="$ZLIB_OLD_CPPFLAGS"

if test "$zlib_cv_libz" = "yes" -a "$zlib_cv_zlib_h" = "yes"
then
  #
  # If both library and header were found, use them
  #
  ZLIB_CFLAGS="-I${ZLIB_HOME}/include"
  ZLIB_LIBS="-L${ZLIB_HOME}/lib -lz"
  AC_SUBST(ZLIB_CFLAGS)
  AC_SUBST(ZLIB_LIBS)
else
  #
  # If either header or library was not found, revert and bomb
  #
  AC_MSG_ERROR(specify a valid zlib installation with --with-zlib=DIR)
  AC_MSG_ERROR([*** zlib library not found!
Install the zlib / libz development package using your distribution's package manager.])
fi

# Check if bzip2 is available - also based on CHECK_ZLIB()

AC_ARG_WITH(bzlib,
	AS_HELP_STRING([--with-bzlib=DIR],
		       [root directory path of bzip2 installation
		        @<:@defaults to /usr/local or /usr if not found in /usr/local@:>@]),
[if test -d "$withval"
  then
    BZLIB_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
fi])

if test ! -f "${BZLIB_HOME}/include/bzlib.h"; then
  BZLIB_HOME=/usr/local
  if test ! -f "${BZLIB_HOME}/include/bzlib.h"; then
    BZLIB_HOME=/usr
  fi
fi

AC_MSG_CHECKING(bzlib in ${BZLIB_HOME})
AC_MSG_RESULT()

BZLIB_OLD_LDFLAGS=$LDFLAGS
BZLIB_OLD_CPPFLAGS=$CPPFLAGS
LDFLAGS="$LDFLAGS -L${BZLIB_HOME}/lib"
CPPFLAGS="$CPPFLAGS -I${BZLIB_HOME}/include"
AC_LANG_SAVE
AC_LANG_C
AC_CHECK_LIB(bz2, BZ2_bzCompressInit, [bzlib_cv_libbz2=yes], [bzlib_cv_libbz2=no])
AC_CHECK_HEADER(bzlib.h, [bzlib_cv_bzlib_h=yes], [bzlib_cv_bzlib_h=no])
AC_LANG_RESTORE
LDFLAGS="$BZLIB_OLD_LDFLAGS"
CPPFLAGS="$BZLIB_OLD_CPPFLAGS"

if test "$bzlib_cv_libbz2" = "yes" -a "$bzlib_cv_bzlib_h" = "yes"
then
  #
  # If both library and header were found, use them
  #
  BZLIB_CFLAGS="-I${BZLIB_HOME}/include"
  BZLIB_LIBS="-L${BZLIB_HOME}/lib -lbz2"
  AC_SUBST(BZLIB_CFLAGS)
  AC_SUBST(BZLIB_LIBS)
else
  #
  # If either header or library was not found, revert and bomb
  #
  AC_MSG_ERROR(specify a valid bzip2 (bzlib) installation with --with-bzlib=DIR)
  AC_MSG_ERROR([*** bzip2 library not found!
Install the bzip2 development package using your distribution's package manager.])
fi

# Win32 tools to add icons and resources to the programs

if $ON_WIN32; then
   AM_CONDITIONAL(GOT_RESCOMP, test "$WX_RESCOMP" != "")
else
   AM_CONDITIONAL(GOT_RESCOMP, false)
fi

# Check for functions not available on some systems

EXTRA_LIBS=""

AC_CHECK_DECLS([clock_gettime],[],[],[#include <time.h>]) # Not on MacOXS

AC_CHECK_FUNCS(clock_gettime, [], [
    AC_CHECK_LIB(rt, clock_gettime, [
        AC_DEFINE(HAVE_CLOCK_GETTIME, 1)
        EXTRA_LIBS="$EXTRA_LIBS -lrt"
    ])
])

AC_SUBST(EXTRA_LIBS)

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		 libstc/Makefile
		 libenctain/Makefile
		 libenctain/testsuite/Makefile
		 src/Makefile
		 src/art/Makefile
		 src/art/crystal/Makefile
		 src/art/slick/Makefile
		 src/art/gnome/Makefile
		 src/locale/Makefile
		 src/locale/wxstd/Makefile
                 src/help/Makefile
                 src/help/en/Makefile
                 src/help/en/html/Makefile
                 src/help/de/Makefile
                 src/help/de/html/Makefile
		 src/pwgen/Makefile
		 src/cryptote/Makefile
		 desktop/Makefile])

if $ON_WIN32; then
    AC_CONFIG_FILES([win32/cryptote-win32.iss win32/cryptote-pwgen-win32.iss])
fi
if $ON_MACOSX; then
    AC_CONFIG_FILES([src/cryptote/Info.plist src/pwgen/Info.plist])
fi

AC_OUTPUT
