## Process this file with automake to produce Makefile.in

INCLUDES = -I$(srcdir)/../include 

lib_LTLIBRARIES = libpublish-sipagent.la

libpublish_sipagent_la_LDFLAGS = $(GLIB_LIBS)

libpublish_sipagent_la_CXXFLAGS = $(GLIB_CFLAGS)

libpublish_sipagent_la_SOURCES =  \
    sipendpoint/JNIHelpers.cpp \
    com_pingtel_pds_sds_SipConfigServerAgent.cpp
