#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# benign edit

AC_PREREQ(2.57)
AC_INIT(sipXconfig, 2.9, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])

# for sipagent-jni. could not find a way to disable from make file
# disable to not consume unnec. compile time and unnec install space
# in particular reduce size of rpm
AM_DISABLE_STATIC

AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([neoconf/src/org/sipfoundry/sipxconfig/common/User.java])

AC_ARG_ENABLE(sipagent,
 [ --disable-sipagent    Do not try to compile sipagent jni, requires sipXtackLib],
 , enable_sipagent=yes)
AM_CONDITIONAL(SIPAGENT, test x$enable_sipagent = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

SFAC_INIT_FLAGS

SFAC_FEATURE_SIP_TLS

if test x$enable_sipagent = xyes
then
  AM_CONFIG_HEADER([config.h])

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

  SFAC_LIB_STACK
else
  CHECK_SSL
  SFAC_INIT_FLAGS
fi

# Jar files to all of sipXconfig. 
AC_SUBST(SIPXCONFIG_LIBDIR, [${SIPX_DATADIR}/lib])

AC_ARG_VAR(SIPFOUNDRY_ORG, [If you plan to publish manual to www.sipfoundry.org, set this to where you keep your local copy of http://scm.sipfoundry.org/rep/sipfoundry.org])

# Expand build dir into absolute path
AC_SUBST(BUILDDIR_EXPAND, [`cd $builddir && pwd`])

# turn off the doxygen support shared from other sipX projects
AM_CONDITIONAL(DOC,false)

CHECK_JNI
CHECK_ANT
CHECK_CLOVER

AC_CONFIG_FILES([
    Makefile
    config/sipXcommon.mak
    bin/Makefile
    neoconf/Makefile
    web/Makefile
    sipagent-jni/Makefile
    sipagent-jni/src/Makefile
    sipagent-jni/include/Makefile
    doc/Makefile
])

AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved
# until make time, so here we get a jump start on this mainly for ANT
# purist that don't do make and got right int ant
make config.properties

