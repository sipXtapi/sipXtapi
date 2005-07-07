

SUBDIRS = 

cgidir=@wwwdir@/cgi-bin

mediaservercgidir = $(cgidir)/voicemail
mediaservercgi_PROGRAMS = mediaserver.cgi

mediaserver_cgi_LDADD = \
    @CGICC_DIR@/lib/libcgicc.la \
    ../mailboxmgr/libmailboxmgr.la \
    ../sipxcgi/libsipxcgi.la

mediaserver_cgi_CXXFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir)/include/ -I$(top_srcdir)/src \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\"

mediaserver_cgi_LDFLAGS = $(GLIB_LIBS)
mediaserver_cgi_SOURCES = main.cpp
