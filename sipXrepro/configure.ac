#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipxrepro, 3.1.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/sipxrepro.cpp])
AM_CONFIG_HEADER([config.h]) # interacts badly with resiprocate sources
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

SFAC_FEATURE_SIP_TLS

# Checks for libraries.
CHECK_REPRO
SFAC_LIB_COMMSERVER

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Outputs

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([config/sipXcommon.mak])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/Makefile])
#AC_CONFIG_FILES([doc/Makefile])
#AC_CONFIG_FILES([meta/Makefile])
#AC_CONFIG_FILES([etc/Makefile])
#AC_CONFIG_FILES([lib/Makefile])

AC_OUTPUT
