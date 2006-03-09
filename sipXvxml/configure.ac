#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXvxml, 3.3.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/client/api/OSBclient.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

# Checks for libraries.
# Check for ZLIB
#
have_zlib='no'
LIB_ZLIB=''
if test "$with_zlib" != 'no'
then
  LIB_ZLIB=''
  AC_MSG_CHECKING(for ZLIB support )
  AC_MSG_RESULT()
  failed=0;
  passed=0;
  AC_CHECK_HEADER(zconf.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_HEADER(zlib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_LIB(z,compress,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,uncompress,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,deflate,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,inflate,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,gzseek,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,gztell,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_MSG_CHECKING(if ZLIB package is complete)
  if test $passed -gt 0
  then
    if test $failed -gt 0
    then
      AC_MSG_RESULT(no -- some components failed test)
      have_zlib='no (failed tests)'
    else
      LIB_ZLIB='-lz'
      LIBS="$LIB_ZLIB $LIBS"
      AC_DEFINE(HasZLIB,1,Define if you have zlib compression library)
      AC_MSG_RESULT(yes)
      have_zlib='yes'
    fi
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(ZLIB required)
  fi
fi
AM_CONDITIONAL(HasZLIB, test "$have_zlib" = 'yes')
AC_SUBST(LIB_ZLIB)

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Check the nature of va_list.
CHECK_VA_LIST

# Checks for library functions.

SFAC_SRCDIR_EXPAND()

FLAGS="-DOPENVXI -D_GNU_SOURCE -D_PTHREADS -D__STL_PTHREADS -DNDEBUG -D_linux_ -I[$abs_srcdir]/include -I[$abs_srcdir]/src -I[$abs_srcdir]/src/client -I[$abs_srcdir]/js/src"
CFLAGS="$CFLAGS $FLAGS"
CXXFLAGS="$CXXFLAGS $FLAGS"

CHECK_LIBWWW
CHECK_XERCES

SFAC_LIB_CALL

SFAC_FEATURE_SIP_TLS

dnl Double the LIBS to deal with recursive dependencies
LIBS="$LIBS $LIBS"

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ

## Common flags for pingtel related source

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([config/sipXcommon.mak])
AC_CONFIG_FILES([js/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/trd/Makefile])
AC_CONFIG_FILES([src/value/Makefile])
AC_CONFIG_FILES([src/log/Makefile])
AC_CONFIG_FILES([src/jsi/Makefile])
AC_CONFIG_FILES([src/inet/Makefile])
AC_CONFIG_FILES([src/prompt/Makefile])
AC_CONFIG_FILES([src/rec/Makefile])
AC_CONFIG_FILES([src/tel/Makefile])
AC_CONFIG_FILES([src/VXI/Makefile])
AC_CONFIG_FILES([src/client/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_OUTPUT
