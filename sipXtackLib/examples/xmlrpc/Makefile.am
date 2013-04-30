## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXtackLib/include -I$(top_srcdir)/sipXportLib/include

noinst_PROGRAMS = xmlrpcClient

xmlrpcClient_LDADD = $(top_builddir)/sipXtackLib/src/libsipXtack.la $(top_builddir)/sipXportLib/src/libsipXport.la

xmlrpcClient_SOURCES =  xmlrpcClient.cpp
