#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXcallLib, 2.9, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
CHECK_AUTOCONF

SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/cp/CallManager.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
CHECK_COREAUDIO
CHECK_LIBRT
CHECK_PCRE
CHECK_SSL

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_C_VOLATILE

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
AC_CHECK_FUNCS([clock_gettime getcwd gethostbyname gethostname gettimeofday inet_ntoa localtime_r memchr memset mkdir putenv rmdir select setenv socket strchr strdup strerror strrchr strstr strtol strtoul tzset])

CHECK_CPPUNIT

SFAC_LIB_MEDIA

SFAC_FEATURE_SIP_TLS

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE
CHECK_VIDEO

SFAC_FEATURE_SIPX_EZPHONE

AC_CONFIG_FILES([
  Makefile 
  bin/Makefile
  config/sipXcommon.mak
  doc/Makefile
  examples/CallScript/src/Makefile
  examples/Makefile 
  examples/PlaceCall/Makefile
  examples/ReceiveCall/Makefile
  examples/musicServer/Makefile
  examples/sipXezPhone/Makefile
  include/Makefile 
  src/Makefile 
  src/test/Makefile 
  src/test/tapi/Makefile 
])
AC_OUTPUT
