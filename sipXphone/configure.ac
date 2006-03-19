#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXphone, 2.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/pinger/Pinger.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.

SFAC_LIB_MEDIA
SFAC_LIB_MEDIAADAPTER

## TODO: Should these be passed in via environment vars and/or 
## configure line parameters. From subversion? Cruisecontrol?
AC_SUBST(XPRESSA_BUILD_NUMBER, [28])
AC_SUBST(XPRESSA_BUILD_COMMENT, [opensource_beta])

SFAC_INIT_FLAGS

CHECK_JNI
CHECK_ANT
CHECK_PCRE
CHECK_SSL

CFLAGS="$PCRE_CFLAGS $CFLAGS"
CXXFLAGS="$PCRE_CXXFLAGS $CXXFLAGS"

AC_DEFINE_UNQUOTED([CONFIG_PREFIX_USER_DATA], ["${prefix}/share/sipxphone/"], [Where user data is located])
AC_DEFINE_UNQUOTED([CONFIG_PREFIX_SHARED_DATA], ["${prefix}/share/sipxphone/"], [Where shared data is located])

# Conditionally use the GIPS audio libraries
CHECK_GIPS
# GIPS needs to be at the end of the list
LIBS="$LIBS $GIPS_OBJS"

AC_DEFUN([ADD_SIPX_INCL_LIB],
[
    CFLAGS="$CFLAGS -I$abs_srcdir/$1/include"
    CXXFLAGS="$CXXFLAGS -I$abs_srcdir/$1/include"
])

SFAC_LIB_CALL

AC_CONFIG_FILES([
   config/sipXcommon.mak
   Makefile
   src/Makefile
   include/Makefile
   src/test/Makefile
   javacore/Makefile
   softphone/Makefile
])

AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved until make
# time, so this allows ant to work immediately after ./configure
make config.properties


