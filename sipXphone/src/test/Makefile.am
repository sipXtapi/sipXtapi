
INCLUDES = -I$(top_srcdir)/include

## Otherwise unittest won't find java.so or jvm.so
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=@JAVA_LIB_PATH@

## All tests under this GNU variable should run relatively quick
## and of course require no setup
TESTS = sipXphone-check

check_PROGRAMS = sipXphone-check

sipXphone_check_LDADD = \
	../libsipXphone.la \
	@SIPXUNIT_LIBS@ \
  @SIPXPORT_LIBS@ \
  @SIPXTACK_LIBS@ \
  @SIPXMEDIA_LIBS@ \
  @SIPXCALL_LIBS@ \
	-lssl

sipXphone_check_SOURCES = \
    unitmain.cpp
