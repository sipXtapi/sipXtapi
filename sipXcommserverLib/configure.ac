#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXcommserverLib, 3.1.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([src/sipdb/SIPDBManager.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX 	 
AC_PATH_PROG([PERL],[perl]) 	 
AC_PATH_PROG([OPENSSL],[openssl])

# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT

CHECK_XERCES

SFAC_LIB_STACK

SFAC_FEATURE_SIP_TLS

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

AC_CONFIG_FILES([
  Makefile 
  config/localize-sipX.sh
  include/Makefile 
  etc/Makefile 
  bin/Makefile
  bin/ssl-cert/Makefile
  src/Makefile 
  src/test/Makefile 
  src/sipdbtest/Makefile
  src/xsdvalid/Makefile
  doc/Makefile
  meta/Makefile
])
AC_OUTPUT

