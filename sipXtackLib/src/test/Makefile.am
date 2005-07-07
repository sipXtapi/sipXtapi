INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXtack.la

testsuite_SOURCES = \
    net/NetAttributeTokenizerTest.cpp \
    net/HttpMessageTest.cpp \
    net/NameValuePairTest.cpp \
    net/NetMd5CodecTest.cpp \
    net/UrlTest.cpp \
	 net/SipMessageTest.cpp \
    net/SdpBodyTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects.
sandbox_LDADD = $(testsuite_LDADD)
sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    net/SdpBodyTest.cpp
