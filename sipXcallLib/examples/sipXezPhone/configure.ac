#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXezPhone, 1.0.0)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

## TODO: Should these be passed in via environment vars and/or 
## configure line parameters. From subversion? Cruisecontrol?
AC_SUBST(XPRESSA_BUILD_NUMBER, [28])
AC_SUBST(XPRESSA_BUILD_COMMENT, [opensource_beta])

SFAC_INIT_FLAGS

CHECK_PCRE
CHECK_SSL
CHECK_WXWIDGETS

SFAC_LIB_CALL

CFLAGS="$CFLAGS $PCRE_CFLAGS $WXWIDGET_CFLAGS $CFLAGS -I../../../sipXcallLib/include"
CXXFLAGS="$PCRE_CXXFLAGS $CXXFLAGS -I../../../sipXcallLib/include -I../../../sipXportLib/include"

# Conditionally use the GIPS audio libraries
CHECK_GIPS
# GIPS needs to be at the end of the list
LIBS="$LIBS /usr/lib/libsipXport.la /usr/lib/libsipXtack.la /usr/lib/libsipXmedia.la /usr/lib/libsipXcall.la  $WXWIDGET_LIBS"

AC_CONFIG_FILES([
   Makefile
   config/sipXcommon.mak
])

AC_OUTPUT



