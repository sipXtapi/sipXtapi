## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/include -I${top_srcdir}/../sipXcallLib/include

# Make TESTDIR an absolute path to the directory containing
# SipSrvLookupTest.named.*
TESTDIR=$(if $(patsubst /%,,$(top_srcdir)),$(shell pwd)/$(top_srcdir),$(top_srcdir))/src/test/net

# Only -D NAMED_PROGRAM if ./configure has found it and thus set
# @NAMED_PROGRAM@ to non-null.
testsuite_CXXFLAGS = \
    -DTESTDIR=\"$(TESTDIR)\" \
    $(if @NAMED_PROGRAM@,-DNAMED_PROGRAM=\"@NAMED_PROGRAM@\")

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXtack.la

testsuite_SOURCES = \
    net/HttpMessageTest.cpp \
    net/NameValuePairInsensitiveTest.cpp \
    net/NameValuePairTest.cpp \
    net/NetAttributeTokenizerTest.cpp \
    net/NetMd5CodecTest.cpp \
    net/SdpBodyTest.cpp \
    net/SipContactDbTest.cpp \
    net/SipDialogEventTest.cpp \
    net/SipDialogTest.cpp \
    net/SipMessageTest.cpp \
    net/SipPresenceEventTest.cpp \
    net/SipPublishContentMgrTest.cpp \
    net/SipServerShutdownTest.cpp \
    net/SipSrvLookupTest.cpp \
    net/SipSubcriptionMgrTest.cpp \
    net/SipUserAgentTest.cpp \
    net/UrlTest.cpp \
    net/XmlRpcTest.cpp

EXTRA_DIST = \
    net/SipSrvLookupTest.named.zone

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects.
sandbox_LDADD = $(testsuite_LDADD)
sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    net/SipPublishContentMgrTest.cpp
