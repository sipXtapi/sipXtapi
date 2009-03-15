include $(top_builddir)/config/sipXcommon.mak

bin_PROGRAMS = ReceiveCall

INCLUDES = -Iinclude -I../PlaceCall/include

ReceiveCall_LDADD = \
    $(top_builddir)/src/libsipXtapi.la \
    -ldl \
    @CA_LIBS@ 

ReceiveCall_SOURCES = \
    src/ReceiveCall.cpp \
    ../PlaceCall/src/ExternalTransport.cpp

noinst_HEADERS = \
    include/ReceiveCall.h \
    include/ReceiveCallWntApp.h
