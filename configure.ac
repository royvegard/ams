#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(ams, [2.0.1], [alsamodular-devel@lists.sourceforge.net])
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([src/box.h])
AC_CONFIG_HEADER([src/config.h])
AM_INIT_AUTOMAKE([dist-bzip2])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SSIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT

dnl set basic compiler flags
AMS_CORE_CXXFLAGS="-Wall"

dnl set compiler optimization flags
AC_MSG_CHECKING(which processor class to optimize for)
case "$target" in
    i486-*-*)
         AMS_OPT_CXXFLAGS="-march=i486"
         AC_MSG_RESULT(i486)
         ;;
    i586-*-*)
         AMS_OPT_CXXFLAGS="-march=pentium"
         AC_MSG_RESULT(i586)
         ;;
    i686-*-*)
         AMS_OPT_CXXFLAGS="-march=pentiumpro -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(i686)
         ;;
    i786-*-*)
         AMS_OPT_CXXFLAGS="-march=pentium4 -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(i786)
         ;;
    x86_64-*-*)
         AMS_OPT_CXXFLAGS="-m64 -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(x86_64)
         ;;
    *)
         AC_MSG_RESULT(i386)
         AC_MSG_WARN(This is probably not what you want, use --target)
         ;;
esac

if test "x$AMS_OPT_CXXFLAGS" = "x"; then
    AMS_CXXFLAGS=$AMS_CORE_CXXFLAGS
else
    AC_MSG_NOTICE([Compiler optimization... $AMS_OPT_CXXFLAGS])
    AMS_CXXFLAGS="$AMS_CORE_CXXFLAGS $AMS_OPT_CXXFLAGS"
fi
AC_SUBST(AMS_CXXFLAGS)


# Checks for libraries.
AC_CHECK_LIB([QtCore], [main], ,
             [AC_MSG_ERROR(QtCore is required)])
AC_CHECK_LIB([QtGui], [main], ,
             [AC_MSG_ERROR(QtGui is required)])
AC_CHECK_LIB([QtOpenGL], [main], ,
             [AC_MSG_ERROR(QtOpenGL is required)])
AC_CHECK_LIB([asound], [snd_pcm_open], ,
             [AC_MSG_ERROR(asound is required)])
AC_CHECK_LIB([jack], [jack_client_new], ,
             [AC_MSG_ERROR(jack is required)])
AC_CHECK_LIB([pthread], [pthread_create], ,
             [AC_MSG_ERROR(pthread is required)])
AC_CHECK_LIB([m], [roundf], ,
             [AC_MSG_ERROR([required libm missing])])
AC_CHECK_LIB([dl], [dlopen], ,
             [AC_MSG_ERROR(libdl is required)])
dnl for vocoder module
AC_CHECK_LIB([fftw3], [fftw_execute], ,
             [AC_MSG_ERROR(libfftw3 is required)])

dnl first check for libzita-alsa-pcmi, if not found check for libclalsadrv
AC_CHECK_LIB([zita-alsa-pcmi], [main], ,
             [AC_MSG_WARN([libzita-alsa-pcmi not found, trying libclalsadrv...])])
if test "x$ac_cv_lib_zita_alsa_pcmi_main" = "xno"; then
AC_CHECK_LIB([clalsadrv], [main], ,
             [AC_MSG_ERROR(clalsadrv is required)])

dnl check for clalsadrv library version
AC_LANG_PUSH(C++)
saved_libs="$LDFLAGS"
LDFLAGS="$LDFLAGS -lclalsadrv"
AC_MSG_CHECKING([for API 2.0.0 version of libclalsadrv])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <clalsadrv.h>],
    [Alsa_driver ad = Alsa_driver("play", "capt","ctrl", 0, 0, 0)])],
    [HAVE_CLALSADRV_API2=1] [AC_SUBST(HAVE_CLALSADRV_API2)]
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])

LDFLAGS="$saved_libs"
AC_LANG_POP(C++)
fi


PKG_CHECK_MODULES(QT, QtCore QtGui >= 4.2.0, HAVE_QT4="yes", HAVE_QT4="no")
if test "x$HAVE_QT4" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl doesn't contain the subsystem dir.
  QT_INCLUDE_DIR=`$PKG_CONFIG --variable=includedir QtGui`
  QT_CXXFLAGS=`$PKG_CONFIG --cflags QtCore QtGui QtOpenGL`
  EXTRA_QT_INCLUDE_DIR="$QT_INCLUDE_DIR/Qt"
  AC_CHECK_FILE([$QT_INCLUDE_DIR/QWidget],
  AC_MSG_NOTICE([No extra QT_INCLUDE_DIR needed]),
  AC_CHECK_FILE([$EXTRA_QT_INCLUDE_DIR/QWidget],
             QT_CPPFLAGS="$QT_CPPFLAGS -I$EXTRA_QT_INCLUDE_DIR",
             AC_MSG_WARN([QWidget not found])))

  AC_MSG_NOTICE([Set QT_CXXFLAGS... $QT_CXXFLAGS])
  AC_SUBST(QT_CXXFLAGS)
else
  AC_MSG_ERROR([cannot find Qt4 library >= 4.2])
fi

dnl check for moc
AC_PATH_PROGS(MOC, moc-qt4 moc, no, [$PATH:$QTDIR/bin])
if test "x$MOC" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'moc', please install the Qt4 development tools package.])
fi

dnl check for lupdate
AC_PATH_PROGS(LUPDATE, lupdate-qt4 lupdate, no, [$PATH:$QTDIR/bin])
if test "x$LUPDATE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lupdate', please install the Qt4 development tools package.])
fi

dnl check for lrelease
AC_PATH_PROGS(LRELEASE, lrelease-qt4 lrelease, no, [$PATH:$QTDIR/bin])
if test "x$LRELEASE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lrelease', please install the Qt4 development tools package.])
fi

# Checks for special library functions.
AC_CHECK_FUNCS([floor ftruncate getcwd memset mkdir pow rint select sqrt])

# Check for ladspa
AC_CHECK_HEADER([ladspa.h], ,
                [AC_MSG_ERROR(essential header file is missing)])

# Check for ladspa plugins path
AC_ARG_WITH(ladspa-path,
  [AC_HELP_STRING([--with-ladspa-path=DIR],
    [directory for ladspa plugins])],
  [ if test "x$withval" != "x" ; then
     LADSPA_PATH="$withval"
    fi ],
  [LADSPA_PATH="/usr/lib/ladspa:/usr/local/lib/ladspa"])
AC_SUBST(LADSPA_PATH)

dnl JACK session
AC_ARG_ENABLE(jack-session,
    [AS_HELP_STRING(--disable-jack-session, [Disable JACK session support])],
    [jack_session=$enableval],
    [jack_session=yes])

if test "$jack_session" != "no"; then
    AC_CHECK_HEADER(jack/session.h, jack_session_found="yes",
                    jack_session_found="no")
    if test "$jack_session_found" = "yes"; then
        AC_DEFINE(JACK_SESSION, 1,
                  [Define to enable JACK session support])
        fi
    fi


AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([demos/Makefile])
AC_CONFIG_FILES([instruments/Makefile])
AC_CONFIG_FILES([tutorial/Makefile])
AC_CONFIG_FILES([colors/Makefile])
AC_CONFIG_FILES([pixmaps/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
