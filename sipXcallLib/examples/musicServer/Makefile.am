## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = musicserver

musicserver_CXXFLAGS = $(GLIB_CFLAGS)
musicserver_LDFLAGS = $(top_builddir)/src/libsipXcall.la $(GLIB_LIBS)

musicserver_SOURCES =  \
    main.cpp \
    listener.cpp

noinst_HEADERS = \
	listener.h

EXTRA_DIST = \
	default.wav

