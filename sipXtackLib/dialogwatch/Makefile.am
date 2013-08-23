## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    dialogdisplay

INCLUDES = \
	-I$(top_srcdir)/sipXtackLib/include \
	-I$(top_builddir)/sipXtackLib/include

bin_PROGRAMS = dialogwatch

bin_SCRIPTS = dialogdisplay

# Program that monitors dialog events.
dialogwatch_CXXFLAGS = 
dialogwatch_LDADD = $(top_builddir)/src/libsipXtack.la
dialogwatch_SOURCES = \
    dialogwatch.cpp
