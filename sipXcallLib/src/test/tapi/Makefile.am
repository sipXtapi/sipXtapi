INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXMEDIA_LIBS@ \
    @SIPXMEDIA_MP_LIBS@ \
    @SIPXSDP_LIBS@ \
    @SIPXTACK_LIBS@ \
    @SIPXPORT_LIBS@ \
    ../../libsipXcall.la

testsuite_SOURCES = \
  callbacks.cpp \
  EventRecorder.cpp \
  EventValidator.cpp \
  sipXtapiTest.cpp \
  sipXtapiTestCall.cpp \
  sipXtapiTestConference.cpp \
  sipXtapiTestConferenceAdv.cpp \
  sipXtapiTestConfig.cpp \
  sipXtapiTestLine.cpp \
  sipXtapiTestTransfer.cpp \
  TestExternalTransport.cpp \
  TestRegistrar.cpp \
  TestRegistryCollection.cpp \
  TestRegistryData.cpp \
  TestStunServerTask.cpp \
  sipXtapiTestSubscribe.cpp \
  ../jnibutton.cpp

DISTCLEANFILES = Makefile.in
