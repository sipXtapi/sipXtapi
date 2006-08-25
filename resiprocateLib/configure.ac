#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(resiprocateLib, 3.1.0, resiprocate-devel@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)

SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

# AM_PATH_CPPUNIT(1.10)
# This is how to check using AM_PATH_CPPUNIT

AC_CONFIG_SRCDIR([resiprocate/resip/stack/SipStack.hxx])
AC_PROG_LIBTOOL

# Checks for libraries.
CHECK_LIBRT
CHECK_CPPUNIT

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_PCRE
CHECK_SSL

SFAC_LIB_PORT

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([config/sipXcommon.mak])

AC_OUTPUT
