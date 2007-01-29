
# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_include_HEADERS = \
    sdp/Sdp.h \
    sdp/SdpMediaLine.h \
    sdp/SdpCodec.h \
    sdp/SdpCandidate.h \
    sdp/SdpCandidatePair.h

EXTRA_DIST= \
    sdp/SdpHelperResip.h 

DISTCLEANFILES = Makefile.in
