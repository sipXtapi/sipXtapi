## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/include

# Make TESTDIR an absolute path to the directory containing
# SipSrvLookupTest.named.*
TESTDIR=$(if $(patsubst /%,,$(top_srcdir)),$(shell pwd)/$(top_srcdir),$(top_srcdir))/src/test/sdp

# Only -D NAMED_PROGRAM if ./configure has found it and thus set
# @NAMED_PROGRAM@ to non-null.
testsuite_CXXFLAGS = \
    -DTESTDIR=\"$(TESTDIR)\" \
    -I$(SIPXUNITINC) \
	@SSL_CXXFLAGS@ \
    @CPPUNIT_CFLAGS@ \
    -O0 
#    $(if @NAMED_PROGRAM@,-DNAMED_PROGRAM=\"@NAMED_PROGRAM@\")

testsuite_CPPFLAGS = \
    @CPPUNIT_CFLAGS@ \
	@PCRE_CFLAGS@

## To load source in gdb for libsipXport.la, type the 'share' at the
## gdb console just before stepping into function in sipXportLib
testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    $(CPPUNIT_LIBS) \
    @SIPXPORT_LIBS@ \
	../libsipXsdp.la \
	@SSL_LDFLAGS@

testsuite_SOURCES = \
	sdp/SdpSetGetTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects.
sandbox_LDADD = $(testsuite_LDADD)
sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    sdp/SdpSetGetTest.cpp
