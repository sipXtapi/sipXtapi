#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT(sipXmediaPluginMgr, 2.9, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
CHECK_AUTOCONF

SFAC_AUTOMAKE_VERSION([1.6])

# Configure pkg-config
PKG_PROG_PKG_CONFIG

AC_CONFIG_SRCDIR([src/mediaPlugin/MediaPluginFactory.cpp])
AM_CONFIG_HEADER([config.h])

# Codec plugin loading uses dlopen, so we need to check for it here.
AC_LIBTOOL_DLOPEN

AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Check for cross compilation
AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

# Checks for libraries.
CHECK_COREAUDIO

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
# Look for CPPUnit libraries
CHECK_CPPUNIT

# sipX dependencies
SFAC_LIB_STACK

SFAC_FEATURE_SIP_TLS

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

AC_CONFIG_FILES([
  config/sipXcommon.mak
  Makefile 
  src/Makefile
  include/Makefile
  doc/Makefile
])
AC_OUTPUT

