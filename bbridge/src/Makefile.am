## Process this file with automake to produce Makefile.in

bin_PROGRAMS = bbridge

bbridge_LDADD = \
   @SIPXMEDIAADAPTER_LIBS@ \
	@SIPXTACK_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
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
    $(top_srcdir)/config/bbridge-buildstamp.h \
    $(top_srcdir)/config/bbridge-buildstamp.cpp \
	 bbridge.h \
	 bbridge.cpp \
	 Conference.cpp \
	 Conference.h \
	 ConferenceSubscriptionApp.cpp \
	 ConferenceSubscriptionApp.h \
	 ConferenceUserAgent.cpp \
	 ConferenceUserAgent.h \
	 DtmfEvent.h \
	 DtmfEvent.cpp \
	 Participant.cpp \
	 Participant.h \
	 ParticipantFactory.cpp \
	 ParticipantFactory.h \
	 Subsystem.cpp \
	 Subsystem.h \
	 tones.cpp
