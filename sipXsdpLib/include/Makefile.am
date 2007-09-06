
# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_include_HEADERS = \
    sdp/Sdp.h \
    sdp/SdpCandidate.h \
    sdp/SdpCandidatePair.h \
    sdp/SdpCodec.h \
    sdp/SdpCodecList.h \
    sdp/SdpDefaultCodecFactory.h \
    sdp/SdpMediaLine.h

EXTRA_DIST= \
    sdp/SdpHelperResip.h 

DISTCLEANFILES = Makefile.in
