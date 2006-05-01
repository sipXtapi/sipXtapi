#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXproxy, 3.4.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/sipauthproxy/sipauthproxymain.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

SFAC_INIT_FLAGS
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)

# Checks for libraries.

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

SFAC_LIB_COMMSERVER

SFAC_FEATURE_SIP_TLS

SFAC_FEATURE_DBTEST
AM_CONDITIONAL(BUILD_DBTESTS, test x$enable_dbtests = xyes)
if test x$enable_dbtests = xyes
then
  CHECK_RUBY
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_FILES([config/sipXcommon.mak])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/sipforkingproxy/Makefile])
AC_CONFIG_FILES([src/sipauthproxy/Makefile])
AC_CONFIG_FILES([src/cse/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([etc/database/Makefile])
AC_CONFIG_FILES([src/cdr/Makefile])

if test x$enable_dbtests = xyes
then
  AC_CONFIG_FILES([src/cse/test/Makefile])
fi

AC_OUTPUT

