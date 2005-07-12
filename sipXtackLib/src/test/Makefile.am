INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../sipXcallLib/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

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
    net/SipDialogEventTest.cpp \
    net/SipDialogTest.cpp \
    net/SipMessageTest.cpp \
    net/SipPresenceEventTest.cpp \
    net/SipPublishContentMgrTest.cpp \
    net/SipSubcriptionMgrTest.cpp \
    net/SipUserAgentTest.cpp \
    net/UrlTest.cpp \
    net/XmlRpcTest.cpp


# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects.
sandbox_LDADD = $(testsuite_LDADD)
sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    net/SipPublishContentMgrTest.cpp
