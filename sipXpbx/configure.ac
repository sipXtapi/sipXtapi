#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# benign edit

AC_PREREQ(2.57)
AC_INIT(sipXpbx, 2.6.2, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/mod_sipxauth/SIPXAuthHandler.cpp])
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
AM_PATH_GLIB_2_0
if ! test x"$no_glib" = x
then
  AC_MSG_ERROR(GLib2 required)
fi

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

CHECK_APACHE2
CHECK_SSL
CHECK_MODCPLUSPLUS
CHECK_CGICC

SFAC_INIT_FLAGS

SFAC_LIB_COMMSERVER
SFAC_LIB_MEDIA
SFAC_LIB_STACK
SFAC_LIB_PORT

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([include/processcgi/Makefile])
AC_CONFIG_FILES([include/sipxcgi/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/emailnotifier/Makefile])
AC_CONFIG_FILES([src/mailboxmgr/Makefile])
AC_CONFIG_FILES([src/mediaservercgi/Makefile])
AC_CONFIG_FILES([src/mod_sipxauth/Makefile])
AC_CONFIG_FILES([src/processcgi/Makefile])
AC_CONFIG_FILES([src/replicationcgi/Makefile])
AC_CONFIG_FILES([src/sipxcgi/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([src/test/sipxcgi/Makefile])
AC_CONFIG_FILES([src/watchdog/Makefile])
AC_CONFIG_FILES([src/wwwcontent/Makefile])

AM_CONDITIONAL(DO_RPMBUILD, test x$enable_rpmbuild = xyes)

AC_OUTPUT

