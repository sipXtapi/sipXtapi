#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXtackLib, 2.9, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/net/SipMessage.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

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
CHECK_SSL

# sipX dependancies

SFAC_LIB_PORT

SFAC_FEATURE_SIP_TLS

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

## Sipviewer is written in java and compiled w/ant so allow someone
## w/o java or ant to effectively ignore anything about this
AC_ARG_ENABLE(sipviewer, 
  [  --disable-sipviewer       Do not try to compile java based sipviewer program],
  , enable_sipviewer=yes)
AM_CONDITIONAL(SIPVIEWER, test x$enable_sipviewer = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

if test x$enable_sipviewer = xyes
then
## SIPViewer is only part of project that uses java, so restrict
## test only if installing sipviewer
  CHECK_JDK
  CHECK_ANT
fi

# named is used in some sipXtackLib tests
CHECK_NAMED

AC_CONFIG_FILES([
  Makefile 
  config/sipXcommon.mak
  include/Makefile 
  src/Makefile 
  bin/Makefile 
  meta/Makefile 
  src/test/Makefile
  examples/Makefile
  examples/im/Makefile
  syslog2siptrace/Makefile 
  siplog2siptrace/Makefile
  sipviewer/Makefile 
  siptest/Makefile 
  siptest/src/Makefile 
  siptest/include/Makefile 
  siptest/src/siptest/Makefile
  doc/Makefile  
])

AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved until make
# time, so this allows ant to work immediately after ./configure
make config.properties
