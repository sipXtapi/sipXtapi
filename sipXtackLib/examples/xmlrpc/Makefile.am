## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = xmlrpcClient

xmlrpcClient_LDADD = $(top_builddir)/src/libsipXtack.la @SIPXPORT_LIBS@

xmlrpcClient_SOURCES =  xmlrpcClient.cpp
