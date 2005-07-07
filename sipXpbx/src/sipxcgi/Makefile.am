
INCLUDES = \
        -I$(top_srcdir)/include \
        -I@CGICC_DIR@/include

noinst_LTLIBRARIES = libsipxcgi.la

libsipxcgi_la_LIBADD = @CGICC_DIR@/lib/libcgicc.la
libsipxcgi_la_CXXFLAGS = $(GLIB_CFLAGS)
libsipxcgi_la_LDFLAGS = $(GLIB_CFLAGS)
libsipxcgi_la_SOURCES = \
    CgiValues.cpp


