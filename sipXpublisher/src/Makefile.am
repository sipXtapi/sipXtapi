## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = sipstatus

sipstatus_LIBS = $LIBS 
sipstatus_LDFLAGS = $(GLIB_LIBS)
sipstatus_CXXFLAGS = $(GLIB_CFLAGS) \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"


sipstatus_SOURCES =  \
    statusserver/main.cpp \
    statusserver/MwiPlugin.cpp \
    statusserver/Notifier.cpp \
    statusserver/PluginXmlParser.cpp \
    statusserver/StatusPluginReference.cpp \
    statusserver/StatusServer.cpp \
    statusserver/SubscribeServerPluginBase.cpp \
    statusserver/SubscribeServerThread.cpp \
    statusserver/WebServer.cpp
