#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT(sipXmediaLib, 3.3, sipxtapi-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE([foreign subdir-objects])
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

# Configure pkg-config
PKG_PROG_PKG_CONFIG

AC_CONFIG_SRCDIR([src/mp/MpCallFlowGraph.cpp])
AM_CONFIG_HEADER([config.h])

# Codec plugin loading uses dlopen, so we need to check for it here.
AC_LIBTOOL_DLOPEN

AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O

# Check for cross compilation
AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

# Cause configure args to be passed through to subordiant configure script 
# which are invoked automatically via AC_CONFIG_SUBDIRS construct
CONFIGURE_ARGS="$ac_configure_args"
AC_SUBST(CONFIGURE_ARGS)

# Dynamic extenition
CHECH_DYNEXT

# Checks for libraries.
CHECK_COREAUDIO

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

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

# Look for CPPUnit libraries
CHECK_CPPUNIT

# Check for SpanDSP library
CHECK_SPANDSP

# Enable use of code that uses speex dsp library
ENABLE_SPEEX_DSP

# Look for lodable codecs libraries (CHECK_SPEEX changed to ENABLE_CODEC_SPEEX)
CHECK_GSM
ENABLE_CODEC_SPEEX
CHECK_ILBC
CHECK_PCMA_PCMU
CHECK_TONES
CHECK_L16
CHECK_G726
CHECK_G722
CHECK_OPUS
CHECK_NE10
CHECK_AMR_AMRWB
CHECK_G7221
CHECK_G729

# Use external components, if requested.
EXTERNAL_EXTENITIONS

EXTERNAL_EXTENITIONS_MEDIA

# SPEEX detection and configuration code inside general.m4 cannot do
# sub-configure with AC_CONFIG_SUBDIRS, since it would be called more
# than once, so do it here, where we know it only happens once.
#AC_MSG_RESULT(speex contrib: ${contrib_speex_enabled} subdir)
if test "x$contrib_speex_enabled" == "xyes"; then
        #AC_MSG_RESULT(adding speex contrib: ${contrib_speex_enabled} subdir)
        # speex codec has it's own configure, so be sure to call it.
        AC_CONFIG_SUBDIRS([contrib/libspeex])
fi
# Specify a SPEEX_CONTRIB make variable conditional upon using contrib speex.
dnl Do not stick AM_CONDITIONAL inside a shell conditional.
dnl If you do, automake will be confused, thus, it is in configure.ac.
AM_CONDITIONAL(SPEEX_CONTRIB, [test "x$contrib_speex_enabled" == "xyes"])


# declaring other codecs staff
DECLARE_CODECS_STAFF

# Check stream player setting
CHECK_STREAM_PLAYER
if test ${disable_stream_player} = true; then
   AC_MSG_RESULT(Stream player is disabled)
   CXXFLAGS+=" -DDISABLE_STREAM_PLAYER "
fi
# sipX dependencies
SFAC_LIB_PORT
SFAC_LIB_SDP
SFAC_LIB_STACK

SFAC_INIT_FLAGS

SFAC_FEATURE_SIP_TLS

# Enable local audio support (microphone / speaker)
AC_ARG_ENABLE(local-audio,
[  --enable-local-audio    Enable local audio (speaker and microphone), makes sense for CallFlowGraph only],
[ case "${enableval}" in
  yes) enable_local_audio=true ;;
  no) enable_local_audio=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-local-audio) ;;
esac],[enable_local_audio=false])
AM_CONDITIONAL(ENABLE_LOCAL_AUDIO, test x$enable_local_audio = xtrue)

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)
CHECK_GIPSNEQ

AC_CONFIG_FILES([
  Makefile 
  config/sipXcommon.mak
  include/Makefile 
  src/Makefile 
  contrib/Makefile
  contrib/libilbc/Makefile
  src/mp/codecs/Makefile
  src/mp/codecs/plggsm/Makefile
  src/mp/codecs/plgpcmapcmu/Makefile
  src/mp/codecs/plgtones/Makefile
  src/mp/codecs/plgl16/Makefile
  src/mp/codecs/plgspeex/Makefile
  src/mp/codecs/plgilbc/Makefile
  src/mp/codecs/plgg726/Makefile
  src/mp/codecs/plgg722/Makefile
  src/mp/codecs/plgopus/Makefile
  src/mp/codecs/plgamr/Makefile
  src/mp/codecs/plgamrwb/Makefile
  src/mp/codecs/plgg7221/Makefile
  src/mp/codecs/plgg729/Makefile
  src/test/Makefile
  bin/Makefile
  doc/Makefile
])
AC_OUTPUT

