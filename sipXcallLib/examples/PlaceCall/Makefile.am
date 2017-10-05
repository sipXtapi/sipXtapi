include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = PlaceCall

INCLUDES = -Iinclude -I$(top_srcdir)/examples/PlaceCall/include

PlaceCall_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
    @SIPXMEDIA_MP_LIBS@ \
    @SIPXMEDIA_LIBS@ \
    @SIPXTACK_LIBS@ \
    @SIPXSDP_LIBS@ \
    @SIPXPORT_LIBS@ \
    @CA_LIBS@
#    @SIPXMEDIA_VE_LIBS@  \
#    @CA_LIBS@

PlaceCall_SOURCES = \
    src/PlaceCall.cpp \
    src/ExternalTransport.cpp

noinst_HEADERS = \
    include/PlaceCall.h \
    include/PlaceCallWntApp.h \
    include/ExternalTransport.h
