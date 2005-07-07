## Process this file with automake to produce Makefile.in

SUBDIRS = . sipdbtest xsdvalid test

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb

lib_LTLIBRARIES = libsipXcommserver.la


## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXcommserver_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXcommserver_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"

libsipXcommserver_la_LIBADD = \
	@SIPXTACK_LIBS@

libsipXcommserver_la_SOURCES =  \
	 ../config/sipxcommserverlib-buildstamp.h \
	 ../config/sipxcommserverlib-buildstamp.cpp \
    sipdb/AliasDB.cpp \
    sipdb/AuthexceptionDB.cpp \
    sipdb/CredentialDB.cpp \
    sipdb/DialByNameDB.cpp \
    sipdb/ExtensionDB.cpp \
    sipdb/HuntgroupDB.cpp \
    sipdb/PermissionDB.cpp \
    sipdb/RegistrationDB.cpp \
    sipdb/ResultSet.cpp \
    sipdb/SIPDBManager.cpp \
    sipdb/SIPXAuthHelper.cpp \
    sipdb/SubscriptionDB.cpp \
    fastdb/cgistub.cpp \
    fastdb/class.cpp \
    fastdb/compiler.cpp \
    fastdb/container.cpp \
    fastdb/cursor.cpp \
    fastdb/database.cpp \
    fastdb/file.cpp \
    fastdb/hashtab.cpp \
    fastdb/localcli.cpp \
    fastdb/query.cpp \
    fastdb/repsock.cpp \
    fastdb/server.cpp \
    fastdb/symtab.cpp \
    fastdb/sync.cpp \
    fastdb/ttree.cpp \
    fastdb/unisock.cpp \
    fastdb/w32sock.cpp \
    fastdb/wwwapi.cpp \
    digitmaps/UrlMapping.cpp
