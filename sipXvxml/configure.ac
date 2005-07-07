#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXvxml, 2.9.0, sipx-dev@list.sipfoundry.org)
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
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

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
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])
AM_PATH_GLIB_2_0
if ! test x"$no_glib" = x
then
  AC_MSG_ERROR(GLib2 required)
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_C_VOLATILE
# Check the nature of va_list.
CHECK_VA_LIST

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([vswprintf], [], AC_MSG_ERROR(vswprintf REQUIRED))
AC_CHECK_FUNCS([clock_gettime getcwd gethostbyname gethostname gettimeofday inet_ntoa localtime_r memchr memset mkdir putenv rmdir select setenv socket strchr strdup strerror strrchr strstr strtol strtoul tzset vswprintf])

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
CHECK_GIPS
# GIPS needs to be at the end of the list
LIBS="$LIBS $GIPS_OBJS"

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
AC_OUTPUT
