#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(bbridge, 0.1.0, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/ConferenceUserAgent.h])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

# Checks for libraries.

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.


SFAC_LIB_MEDIA
SFAC_LIB_MEDIAADAPTER
SFAC_LIB_COMMSERVER

CHECK_GIPSNEQ
CHECK_RESIPROCATE

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([doc/Makefile])

AC_OUTPUT
