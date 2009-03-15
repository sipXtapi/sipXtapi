include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = PlaceCall

INCLUDES = -Iinclude

PlaceCall_LDADD = \
    $(top_builddir)/src/libsipXtapi.la \
    -ldl \
    @CA_LIBS@

PlaceCall_SOURCES = \
    src/PlaceCall.cpp \
    src/ExternalTransport.cpp

noinst_HEADERS = \
    include/PlaceCall.h \
    include/PlaceCallWntApp.h \
    include/ExternalTransport.h
