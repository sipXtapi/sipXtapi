## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = sdpTest

check_PROGRAMS = sdpTest

## To load source in gdb for libsipXport.la, type the 'share' at the
## gdb console just before stepping into function in sipXportLib
sdpTest_LDADD = \
        @SIPXPORT_LIBS@ \
        @SIPXTACK_LIBS@ \
	../libsipXsdp.la \
	@SSL_LDFLAGS@

sdpTest_CXXFLAGS = \
	-I$(top_srcdir)/include \
	@PCRE_CFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DTEST_DIR=\"@abs_builddir@\" \
	-O0 

sdpTest_SOURCES = \
	sdp/sdpTest.cpp

