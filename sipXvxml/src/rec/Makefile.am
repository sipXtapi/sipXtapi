noinst_LTLIBRARIES = libOSBrec.la 

# do not do versioning for noinst libraries

INCLUDES = -I$(top_srcdir)/../sipXcallLib/include

libOSBrec_la_CXXFLAGS = -DCOMPANY_DOMAIN=L\"pingtel.com\"

libOSBrec_la_SOURCES =  \
	osbrec.cpp \
	osbrec_utils.cpp

noinst_HEADERS = \
	osbrec_utils.h
