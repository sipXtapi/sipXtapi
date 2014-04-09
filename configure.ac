#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)
# test0 tarballs are not for any distribution
AC_INIT(sipXtapi, 3.3.0~test17, sipxtapi-dev@list.sipfoundry.org)
AC_CONFIG_SRCDIR([sipXportLib/src/os/OsConfigDb.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wno-extra-portability tar-ustar])
#AC_PREFIX_DEFAULT([/usr/local/sipx])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT


SFAC_AUTOMAKE_VERSION([1.11])
#CHECK_AUTOCONF

# Configure pkg-config
PKG_PROG_PKG_CONFIG

AM_CONFIG_HEADER([config.h])

# Codec plugin loading uses dlopen, so we need to check for it here.
AC_LIBTOOL_DLOPEN

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Check for cross compilation
AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

# Dynamic extenition
CHECH_DYNEXT

# Checks for libraries.
CHECK_COREAUDIO
CHECK_CARBON
#CHECK_LIBRT

# Check for SpanDSP library
CHECK_SPANDSP
# Enable use of code that uses speex dsp library
ENABLE_SPEEX_DSP

# Look for GSM and Speex codecs
CHECK_GSM
ENABLE_CODEC_SPEEX
CHECK_ILBC
CHECK_PCMA_PCMU
CHECK_TONES
CHECK_L16
CHECK_G726
CHECK_G722
CHECK_AMR_AMRWB
CHECK_OPUS

# PCRE is mandatory
PKG_CHECK_MODULES([DEPS_PCRE], [libpcre >= 4.5])

# OpenSSL is mandatory, find it or configure fails:
CHECK_SSL

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Check CPPUnit - begin
# can't be encapsulated as a function due to search path ordering issues
AM_PATH_CPPUNIT(1.9,
      [ SIPX_UNIT_TARGET="sipxunit" ],
      [ SIPX_UNIT_TARGET="sipxportunit"
        CPPUNIT_CFLAGS=' -DNO_CPPUNIT -I$(top_srcdir)/sipXportLib/src/test/sipxportunit '
        AC_MSG_WARN("cppunit not found") ]
)

AC_SUBST(SIPX_UNIT_TARGET)

AM_CONDITIONAL([BUILD_SIPXUNIT], [test "x$SIPX_UNIT_TARGET" == "xsipxunit" ])
AM_CONDITIONAL([BUILD_SIPXPORTUNIT], [test "x$SIPX_UNIT_TARGET" == "xsipxportunit" ])
# Check CPPUnit - end

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

# Check for toplogy flowgraph interface
CHECK_GRAPH_INTERFACE

# SPEEX detection and configuration code inside general.m4 cannot do
# sub-configure with AC_CONFIG_SUBDIRS, since it would be called more
# than once, so do it here, where we know it only happens once.
#if test "x$contrib_speex_enabled" == "xyes"; then
#        # speex codec has it's own configure, so be sure to call it.
#        AC_CONFIG_SUBDIRS([contrib/libspeex])
#fi
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

## Sipviewer is written in java and compiled w/ant so allow someone
## w/o java or ant to effectively ignore anything about this
AC_ARG_ENABLE(sipviewer, 
  [  --enable-sipviewer       Compile java based sipviewer program (needs JVM)],
  , enable_sipviewer=no)
AM_CONDITIONAL(SIPVIEWER, test x$enable_sipviewer = xyes)
if test x$enable_sipviewer = xyes
then
## SIPViewer is only part of project that uses java, so restrict
## test only if installing sipviewer
  CHECK_JDK
  CHECK_ANT
fi
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)


EXTERNAL_EXTENITIONS
EXTERNAL_EXTENITIONS_MEDIA
SPEACKER_SELECTION_CHECK

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE
CHECK_VIDEO

SFAC_FEATURE_SIPX_EZPHONE

# named is used in some sipXtackLib tests
CHECK_NAMED

AC_SEARCH_LIBS([clock_nanosleep],[rt posix4])

AC_CONFIG_FILES([
  Makefile
  sipxtapi.spec
  bin/Makefile
  sipXcallLib/Makefile 
  sipXcallLib/doc/Makefile
  sipXcallLib/examples/Makefile 
  sipXcallLib/examples/sipXezPhone/Makefile
  sipXcallLib/include/Makefile 
  sipXcallLib/src/Makefile 
  sipXcallLib/src/test/Makefile 
  sipXcallLib/src/test/tapi/Makefile
  sipXmediaAdapterLib/Makefile 
  sipXmediaAdapterLib/interface/Makefile
  sipXmediaAdapterLib/sipXmediaMediaProcessing/Makefile
  sipXmediaAdapterLib/sipXmediaMediaProcessing/src/Makefile
  sipXmediaAdapterLib/sipXmediaMediaProcessing/src/test/Makefile
  sipXmediaAdapterLib/sipXmediaMediaProcessing/include/Makefile
  sipXmediaAdapterLib/examples/Makefile
  sipXmediaAdapterLib/doc/Makefile
  sipXmediaLib/Makefile 
  sipXmediaLib/include/Makefile 
  sipXmediaLib/src/Makefile 
  sipXmediaLib/src/mp/codecs/Makefile
  sipXmediaLib/src/mp/codecs/plggsm/Makefile
  sipXmediaLib/src/mp/codecs/plgpcmapcmu/Makefile
  sipXmediaLib/src/mp/codecs/plgtones/Makefile
  sipXmediaLib/src/mp/codecs/plgl16/Makefile
  sipXmediaLib/src/mp/codecs/plgspeex/Makefile
  sipXmediaLib/src/mp/codecs/plgilbc/Makefile
  sipXmediaLib/src/mp/codecs/plgg726/Makefile
  sipXmediaLib/src/mp/codecs/plgg722/Makefile
  sipXmediaLib/src/mp/codecs/plgamr/Makefile
  sipXmediaLib/src/mp/codecs/plgamrwb/Makefile
  sipXmediaLib/src/mp/codecs/plgopus/Makefile
  sipXmediaLib/src/test/Makefile
  sipXmediaLib/doc/Makefile
  sipXportLib/Makefile 
  sipXportLib/include/Makefile 
  sipXportLib/src/Makefile 
  sipXportLib/src/test/Makefile
  sipXportLib/src/test/sipxunit/Makefile
  sipXportLib/src/test/sipxportunit/Makefile
  sipXportLib/src/test/testplugin/Makefile
  sipXportLib/syslogviewer/Makefile
  sipXportLib/doc/Makefile
  sipXsdpLib/Makefile 
  sipXsdpLib/include/Makefile 
  sipXsdpLib/src/Makefile 
  sipXsdpLib/src/test/Makefile
  sipXsdpLib/doc/Makefile
  sipXtackLib/Makefile 
  sipXtackLib/include/Makefile 
  sipXtackLib/src/Makefile 
  sipXtackLib/meta/Makefile 
  sipXtackLib/src/test/Makefile
  sipXtackLib/examples/Makefile
  sipXtackLib/examples/im/Makefile
  sipXtackLib/examples/xmlrpc/Makefile
  sipXtackLib/syslog2siptrace/Makefile 
  sipXtackLib/siplog2siptrace/Makefile
  sipXtackLib/sipviewer/Makefile 
  sipXtackLib/siptest/Makefile 
  sipXtackLib/siptest/src/Makefile 
  sipXtackLib/siptest/include/Makefile 
  sipXtackLib/siptest/src/siptest/Makefile
  sipXtackLib/doc/Makefile
])

AC_OUTPUT
