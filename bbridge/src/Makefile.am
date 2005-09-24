## Process this file with automake to produce Makefile.in

bin_PROGRAMS = bbridge

bbridge_LDADD = \
	@SIPXMEDIA_MP_LIBS@ \
	@SIPXTACK_LIBS@ \
	@RESIPROCATE_LIBS@

bbridge_LDFLAGS = \
	@RESIPROCATE_LDFLAGS@

bbridge_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	@RESIPROCATE_CXXFLAGS@

bbridge_SOURCES =  \
    ../config/bbridge-buildstamp.h \
    ../config/bbridge-buildstamp.cpp \
	 bbridge.cpp \
	 Conference.cpp \
	 Conference.h \
	 ConferenceUserAgent.cpp \
	 ConferenceUserAgent.h \
	 Participant.cpp \
	 ParticipantFactory.cpp \
	 ParticipantFactory.h \
	 Participant.h \
         tones.cpp
