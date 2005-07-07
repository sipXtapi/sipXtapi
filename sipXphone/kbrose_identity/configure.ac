#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXphone, 2.5, sipx-dev@list.sipfoundry.org)
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
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])
## Check to see that we are using the minimum required version of glib-2
## and set up the CFLAGS and LDFLAGS for it
AM_PATH_GLIB_2_0( [2.2.0],
  AC_MSG_RESULT(found glib2),
  AC_MSG_ERROR(glib2 is required),
  gthread
)

## TODO: Should these be passed in via environment vars and/or 
## configure line parameters. From subversion? Cruisecontrol?
AC_SUBST(XPRESSA_BUILD_NUMBER, [0])
AC_SUBST(XPRESSA_BUILD_COMMENT, [opensource_beta])

SFAC_INIT_FLAGS

CHECK_JNI
CHECK_ANT
CHECK_PCRE
CHECK_SSL

AC_DEFINE_UNQUOTED([CONFIG_PREFIX_USER_DATA], ["${prefix}/share/sipxphone/"], [Where user data is located])
AC_DEFINE_UNQUOTED([CONFIG_PREFIX_SHARED_DATA], ["${prefix}/share/sipxphone/"], [Where shared data is located])

# Conditionally use the GIPS audio libraries
CHECK_GIPS
# GIPS needs to be at the end of the list
LIBS="$LIBS $GIPS_OBJS"

SFAC_SRCDIR_EXPAND

AC_DEFUN([ADD_SIPX_INCL_LIB],
[
    CFLAGS="$CFLAGS -I$abs_srcdir/$1/include"
    CXXFLAGS="$CXXFLAGS -I$abs_srcdir/$1/include"
])

ADD_SIPX_INCL_LIB([sipXportLib])
ADD_SIPX_INCL_LIB([sipXtackLib])
ADD_SIPX_INCL_LIB([sipXmediaLib])
ADD_SIPX_INCL_LIB([sipXcallLib])


AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile include/Makefile src/test/Makefile])
AC_CONFIG_FILES([javacore/Makefile])
AC_CONFIG_FILES([softphone/Makefile])
AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved until make
# time, so this allows ant to work immediately after ./configure
make config.properties


