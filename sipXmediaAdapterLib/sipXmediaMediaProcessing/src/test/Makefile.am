
TESTS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/interface \
           -I$(top_srcdir)/sipXmediaMediaProcessing/include

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    $(CPPUNIT_LIBS) \
    @SIPXTACK_LIBS@ \
    @SIPXSDP_LIBS@ \
    @SIPXPORT_LIBS@ \
    @SIPXMEDIA_LIBS@ \
    ../libsipXmediaProcessing.la \
    $(GIPS_OBJS)

testsuite_CXXFLAGS = -I$(SIPXUNITINC) \
                     @CPPUNIT_CFLAGS@

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@

codecQualitySource/%.WAV: | itu_peak_recordings.rar 
	if [ ! -d codecQualitySource ] ; then mkdir -p codecQualitySource ; fi
	if [ -d tmp777 ] ; then rm -rf tmp777 ; fi
	mkdir tmp777; (cd tmp777; unrar e ../itu_peak_recordings.rar  CONFORMANCE\ TEST\ ITEMS)
	cp tmp777/*REF*.WAV codecQualitySource
	rm -rf tmp777

# ITU has retired the public download URLs for the BS.1387-1 conformance
# test recordings (the old extranet path now redirects to a search page,
# and dms_pubrec returns 403). We try the network first so anyone with a
# working URL gets a fresh copy, and fall back to the committed copy in
# sipXmediaLib/contrib/itu/ if the download fails or yields a non-archive
# (e.g. an HTML redirect page).
itu_peak_recordings.rar:
	@echo "=== Attempting ITU download of $@ ==="
	@if wget --no-check-certificate -q --tries=2 --timeout=30 \
            https://extranet.itu.int/brdocsearch/R-REC/R-REC-BS/R-REC-BS.1387/R-REC-BS.1387-1-200111-I/R-REC-BS.1387-1-200111-I\!\!ZIP-E.zip \
            -O $@.tmp \
            && unrar t $@.tmp >/dev/null 2>&1 ; then \
          mv $@.tmp $@ ; \
          echo "=== Using freshly downloaded archive ===" ; \
        else \
          rm -f $@.tmp ; \
          echo "=== Download failed or returned non-archive; using committed copy ===" ; \
          cp ../../../../sipXmediaLib/contrib/itu/itu_peak_recordings.rar.committed $@ ; \
        fi

#	wget --no-check-certificate https://extranet.itu.int/brdocsearch/R-REC/R-REC-BS/R-REC-BS.1387/R-REC-BS.1387-1-200111-I/R-REC-BS.1387-1-200111-I\!\!ZIP-E.zip -O itu_peak_recordings.rar

# Older URL, kept for reference:
#wget --no-check-certificate https://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1387-1-200111-I\!\!ZIP-E.zip -O itu_peak_recordings.rar

CpPhoneMediaInterfaceTest.cpp: codecQualitySource/GREFCLA.WAV
    
# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
    ../../../../sipXportLib/src/test/os/UnitTestLogHooks.cpp \
    CpCryptoTest.cpp \
    CpPhoneMediaInterfaceTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#

sandbox_CXXFLAGS = -DSANDBOX $(testsuite_CXXFLAGS)

sandbox_CPPFLAGS = $(testsuite_CPPFLAGS)

sandbox_LDFLAGS = $(testsuite_LDFLAGS)

sandbox_LDADD = $(testsuite_LDADD)

sandbox_SOURCES = \
    CpPhoneMediaInterfaceTest.cpp
